// logger.h in Otus homework#11 project

#pragma once

#include <chrono>
#include <iostream>
#include <memory>
#include <atomic>
#include <vector>
#include <thread>
#include <fstream>
#include "listeners.h"
#include "smart_buffer_mt.h"
#include "thread_metrics.h"
#include "async_worker.h"


template <size_t threadsCount = 2u>
class Logger : public NotificationListener,
               public MessageListener,
               public MessageBroadcaster,
               public std::enable_shared_from_this<NotificationListener>,
               public AsyncWorker<threadsCount>
{
public:

  using DataType = std::pair<size_t, std::string>;

  Logger(const std::string& newWorkerName,
         const std::shared_ptr<SmartBuffer<DataType>>& newBuffer,
         const std::string& newDestinationDirectory = "",
         std::ostream& newErrorOut, std::mutex& newErrorOutLock) :
    AsyncWorker<threadsCount>{newWorkerName},
    buffer{newBuffer}, destinationDirectory{newDestinationDirectory},
    previousTimeStamp{}, additionalNameSection{},
    errorOut{newErrorOut}, errorOutLock{newErrorOutLock},
    threadMetrics{}
  {
    if (nullptr == buffer)
    {
      throw(std::invalid_argument{"Logger source buffer not defined!"});
    }

    for (size_t threadIndex{0}; threadIndex < threadsCount; ++threadIndex)
    {
      threadMetrics.push_back(std::make_shared<ThreadMetrics>(
          std::string{"logger thread#"} + std::to_string(threadIndex)
      ));
      additionalNameSection.push_back(1);
    }
  }

  ~Logger()
  {
    this->stop();
  }

  void reactNotification(NotificationBroadcaster* sender) override
  {
    if (buffer.get() == sender)
    {
      #ifdef _DEBUG
        std::cout << this->workerName << " reactNotification\n";
      #endif

      ++this->notificationCount;
      this->threadNotifier.notify_one();
    }
  }

  void reactMessage(class MessageBroadcaster* sender, Message message) override
  {
    if (messageCode(message) < 1000) // non error message
    {
      switch(message)
      {
      case Message::NoMoreData :
        if (this->noMoreData.load() != true && buffer.get() == sender)
        {
          #ifdef _DEBUG
            std::cout << "\n                     " << this->workerName<< " NoMoreData received\n";
          #endif

          this->noMoreData.store(true);
          this->threadNotifier.notify_all();
        }
        break;

      default:
        break;
      }
    }
    else                             // error message
    {
      if (this->shouldExit.load() != true)
      {
        this->shouldExit.store(true);
        this->threadNotifier.notify_all();
        sendMessage(message);
      }
    }
  }

  const SharedMultyMetrics getMetrics()
  {
    return threadMetrics;
  }

private:

  bool threadProcess(const size_t threadIndex) override
  {
    if (nullptr == buffer)
    {
      throw(std::invalid_argument{"Logger source buffer not defined!"});
    }

    decltype(buffer->getItem()) bufferReply{};
    {
      std::lock_guard<std::mutex> lockBuffer{buffer->dataLock};
      bufferReply = buffer->getItem(shared_from_this());
    }

    if (false == bufferReply.first)
    {
      #ifdef _DEBUG
        std::cout << "\n                     " << this->workerName<< " FALSE received\n";
      #endif

      return false;
    }

    auto nextBulkInfo{bufferReply.second};    

    if (nextBulkInfo.first != previousTimeStamp)
    {
      additionalNameSection[threadIndex] = 1u;
      previousTimeStamp = nextBulkInfo.first;
    }

    std::string bulkFileName{
      destinationDirectory + std::to_string(nextBulkInfo.first)
    };

    auto fileNameSuffix {std::to_string(additionalNameSection[threadIndex])
          + std::to_string(threadIndex + 1)};
    auto logFileName {bulkFileName + "_" + fileNameSuffix +  ".log"};

    std::ofstream logFile{logFileName};

    if(!logFile)
    {
      errorOut << "Cannot create log file " <<
                  logFileName << " !" << std::endl;
      throw(std::ios_base::failure{"Log file creation error!"});
    }

    logFile << nextBulkInfo.second << '\n';
    logFile.close();

    ++additionalNameSection[threadIndex];

    /* Refresh metrics */
    ++threadMetrics[threadIndex]->totalBulkCount;
    threadMetrics[threadIndex]->totalCommandCount
        += std::count(nextBulkInfo.second.begin(),
                      nextBulkInfo.second.end(), ',') + 1;

    return true;
  }

  void onThreadException(const std::exception& ex, const size_t threadIndex) override
  {
    errorOut << this->workerName << " thread #" << threadIndex << " stopped. Reason: " << ex.what() << std::endl;

    this->threadFinished[threadIndex] = true;
    this->shouldExit = true;
    this->threadNotifier.notify_all();

    sendMessage(Message::Abort);

    abortFlag = true;
    terminationNotifier.notify_all();
  }

  void onTermination(const size_t threadIndex) override
  {
    #ifdef _DEBUG
      std::cout << "\n                     " << this->workerName<< " AllDataLogged\n";
    #endif

    if (true == this->noMoreData && this->notificationCount.load() == 0)
    {
      terminationFlag = true;
    }

    if (true == this->shouldExit)
    {
      abortFlag = true;
    }

    terminationNotifier.notify_all();
  }


  std::shared_ptr<SmartBuffer<DataType>> buffer;
  std::string destinationDirectory;

  size_t previousTimeStamp;
  std::vector<size_t> additionalNameSection;

  std::ostream& errorOut;
  std::mutex& errorOutLock;

  SharedMultyMetrics threadMetrics;
};


