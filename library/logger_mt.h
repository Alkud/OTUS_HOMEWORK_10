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
               public std::enable_shared_from_this<Logger<threadsCount>>
{
public:

  using DataType = std::pair<size_t, std::string>;

  Logger(const std::shared_ptr<SmartBuffer<DataType>>& newBuffer,
         const std::string& newDestinationDirectory = "",
         std::ostream& newErrorOut = std::cerr, std::ostream& newMetricsOut = std::cout) :
    buffer{newBuffer}, destinationDirectory{newDestinationDirectory}, errorOut{newErrorOut},
    shouldExit{false}, notificationsCount{0}, activeThreadNumber{0},
    previousTimeStamp{}, additinalNameSection{1u},
    workingThreads{}, threadMetrics{}, metricsOut{newMetricsOut}
  {
    if (nullptr == buffer)
    {
      throw(std::invalid_argument{"Logger source buffer not defined!"});
    }

    for (size_t threadNumber{0}; threadNumber < threadsCount; ++threadNumber)
    {
      workingThreads.push_back(std::thread{&Logger::run, this, threadNumber});
      threadMetrics.push_back(MetricsRecord{});
    }
  }

  ~Logger()
  {
    shouldExit = true;
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
      ++notificationsCount;
    }
  }


private:

  void run(const size_t threadNumber)
  {
    try
    {
      while(shouldExit != true)
      {
        if (threadNumber == activeThreadNumber
            && notificationsCount.load() > 0)
        {
          log(threadNumber);
          --notificationsCount;
          activeThreadNumber = (threadNumber + 1) % threadsCount;
        }
      }
    }
    catch (const std::exception& ex)
    {
      shouldExit = true;
      errorOut << "Logger stopped. Thread #" << threadNumber << ". Reason: " << ex.what() << std::endl;
    }
  }

  struct MetricsRecord
  {
    size_t totalBulksCount{};
    size_t totalCommandsCount{};
  };

  void log(const size_t threadNumber)
  {
    auto nextBulkInfo{buffer->getItem(this->shared_from_this())};

    if (nextBulkInfo.first != previousTimeStamp)
    {
      additinalNameSection = 1u;
    }

    std::string bulkFileName{
      destinationDirectory + std::to_string(nextBulkInfo.first)
    };

    auto fileNameSuffix {std::to_string(additinalNameSection)};
    auto logFileName {bulkFileName + "_" + fileNameSuffix +  ".log"};

    while (std::ifstream {logFileName})
    {
      ++additinalNameSection;
      fileNameSuffix = std::to_string(additinalNameSection);
      logFileName = bulkFileName+ "_" + fileNameSuffix + ".log";
    }

    std::ofstream logFile{logFileName};

    if(!logFile)
    {
        errorOut << "Cannot create log file " <<
                     logFileName << " !" << std::endl;
        return;
    }

    logFile << nextBulkInfo.second << '\n';
    logFile.close();

    ++additinalNameSection;

    /* Refresh metrics */
    ++threadMetrics[threadNumber].totalBulksCount;
    threadMetrics[threadNumber].totalCommandsCount
        += std::count(nextBulkInfo.second.begin(),
                      nextBulkInfo.second.end(), ',') + 1;
  }


  std::shared_ptr<SmartBuffer<DataType>> buffer;
  std::string destinationDirectory;
  std::ostream& errorOut;

  bool shouldExit;
  std::atomic<size_t> notificationsCount;
  size_t activeThreadNumber;
  size_t previousTimeStamp;
  size_t additinalNameSection;

  std::vector<std::thread> workingThreads;
  std::vector<MetricsRecord> threadMetrics;
  std::ostream& metricsOut;
};


