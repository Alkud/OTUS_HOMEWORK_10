// logger.h in Otus homework#7 project

#pragma once

#include "listeners.h"
#include "smart_buffer_mt.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <atomic>
#include <vector>
#include <thread>
#include <fstream>

template <size_t threadsCount = 2u>
class Logger : public NotificationListener,
               public MessageListener,
               public MessageBroadcaster,
               public std::enable_shared_from_this<NotificationListener>
{
public:

  using DataType = std::pair<size_t, std::string>;

  Logger(const std::shared_ptr<SmartBuffer<DataType>>& newBuffer,
         const std::string& newDestinationDirectory = "",
         std::ostream& newErrorOut = std::cerr, std::ostream& newMetricsOut = std::cout) :
    buffer{newBuffer}, destinationDirectory{newDestinationDirectory}, errorOut{newErrorOut},
    shouldExit{false}, notificationsCount{0},
    previousTimeStamp{}, additionalNameSection{},
    workingThreads{}, threadMetrics{}, metricsOut{newMetricsOut}
  {
    if (nullptr == buffer)
    {
      throw(std::invalid_argument{"Logger source buffer not defined!"});
    }
  }

  ~Logger()
  {
    shouldExit = true;
    threadNotifier.notify_all();
    for (auto& thread : workingThreads)
    {
      if (thread.joinable() == true)
      {
        thread.join();
      }
    }

    /* Output metrics */
    for (size_t threadNumber{}; threadNumber < threadsCount; ++threadNumber)
    {
      metricsOut << "file thread #" << threadNumber
                 << " - " << threadMetrics[threadNumber].totalBulksCount
                 << " bulk(s), " << threadMetrics[threadNumber].totalCommandsCount
                 << " command(s)" << std::endl;
    }
  }

  void reactNotification(NotificationBroadcaster* sender) override
  {
    if (buffer.get() == sender)
    {
      {
        std::lock_guard<std::mutex> lockNotifier{notifierLock};
        ++notificationsCount;
      }
      threadNotifier.notify_one();
    }
  }

  void reactMessage(class MessageBroadcaster* sender, Message message) override
  {
    if (Message::NoMoreData == message)
    {
      shouldExit = true;
      threadNotifier.notify_all();
    }
  }

  void start()
  {
    for (size_t threadNumber{0}; threadNumber < threadsCount; ++threadNumber)
    {      
      threadMetrics.push_back(MetricsRecord{});
      additionalNameSection.push_back(0u);
      workingThreads.push_back(std::thread{&Logger::run, this, threadNumber});
    }
  }


private:

  void run(const size_t threadNumber)
  {
    try
    {
      while(shouldExit != true)
      {
        std::unique_lock<std::mutex> lockNotifier{notifierLock};

        threadNotifier.wait(lockNotifier, [this](){return this->notificationsCount.load() > 0 || shouldExit;});

        if (notificationsCount.load() > 0)
        {
          log(threadNumber);
          --notificationsCount;
        }

        lockNotifier.unlock();
      }
    }
    catch(const std::out_of_range& ex)
    {
      if (ex.what() == "Buffer is empty!"
          && shouldExit != true)
      {
        throw;
      }
    }
    catch (const std::exception& ex)
    {
      shouldExit = true;
      sendMessage(Message::AllDataReceived);
      errorOut << "Logger stopped. Thread #" << threadNumber << ". Reason: " << ex.what() << std::endl;
      std::cout << "Logger stopped. Thread #" << threadNumber << ". Reason: " << ex.what() << std::endl;
    }
  }

  struct MetricsRecord
  {
    size_t totalBulksCount{};
    size_t totalCommandsCount{};
  };

  bool log(const size_t threadNumber)
  {
    if (nullptr == buffer)
    {
      return false;
    }

    std::unique_lock<std::mutex> lockBuffer{buffer->dataLock};

    if (buffer->dataSize() == 0)
    {
      lockBuffer.unlock();
      return false;
    }

    auto bufferReply{buffer->getItem(shared_from_this())};    

    lockBuffer.unlock();

    if (false == bufferReply.first)
    {      
      return false;
    }

    auto nextBulkInfo{bufferReply.second};    

    if (nextBulkInfo.first != previousTimeStamp)
    {
      additionalNameSection[threadNumber] = 1u;
    }

    std::string bulkFileName{
      destinationDirectory + std::to_string(nextBulkInfo.first)
    };

    auto fileNameSuffix {std::to_string(additionalNameSection[threadNumber])};
    auto logFileName {bulkFileName + "_" + fileNameSuffix +  ".log"};

    while (std::ifstream {logFileName})
    {
      ++additionalNameSection[threadNumber];
      fileNameSuffix = std::to_string(additionalNameSection[threadNumber]);
      logFileName = bulkFileName+ "_" + fileNameSuffix + ".log";
    }    

    std::ofstream logFile{logFileName};

    if(!logFile)
    {
      errorOut << "Cannot create log file " <<
                  logFileName << " !" << std::endl;
      return false;
    }

    logFile << nextBulkInfo.second << '\n';
    logFile.close();

    ++additionalNameSection[threadNumber];

    /* Refresh metrics */
    ++threadMetrics[threadNumber].totalBulksCount;
    threadMetrics[threadNumber].totalCommandsCount
        += std::count(nextBulkInfo.second.begin(),
                      nextBulkInfo.second.end(), ',') + 1;

    return true;
  }


  std::shared_ptr<SmartBuffer<DataType>> buffer;
  std::string destinationDirectory;
  std::ostream& errorOut;

  bool shouldExit;
  std::atomic<size_t> notificationsCount;
  std::condition_variable threadNotifier{};
  std::mutex notifierLock;

  size_t previousTimeStamp;
  std::vector<size_t> additionalNameSection;

  std::vector<std::thread> workingThreads;
  std::vector<MetricsRecord> threadMetrics;
  std::ostream& metricsOut;
};


