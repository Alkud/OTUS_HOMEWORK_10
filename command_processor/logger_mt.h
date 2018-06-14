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
         bool& newTerminationFlag, bool& newAbortFlag,
         std::condition_variable& newTerminationNotifier,
         const std::string& newDestinationDirectory = "",
         std::ostream& newErrorOut = std::cerr) :
    AsyncWorker<threadsCount>{newWorkerName},
    buffer{newBuffer}, destinationDirectory{newDestinationDirectory}, errorOut{newErrorOut},
    previousTimeStamp{}, additionalNameSection{},
    threadMetrics{},
    terminationFlag{newTerminationFlag}, abortFlag{newAbortFlag},
    terminationNotifier{newTerminationNotifier}
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
    threadFinished.resize(threadsCount, false);
  }

  ~Logger()
  {
    this->stop();
  }

  void reactNotification(NotificationBroadcaster* sender) override
  {
    if (buffer.get() == sender)
    {
      ++this->notificationCount;
      this->threadNotifier.notify_one();
    }
  }

  void reactMessage(class MessageBroadcaster* sender, Message message) override
  {
    switch(message)
    {
    case Message::NoMoreData :
      if (buffer.get() == sender)
      {
        this->noMoreData = true;
        this->threadNotifier.notify_all();
      }
      break;

    case Message::Abort :
      this->noMoreData = true;
      this->shouldExit = true;
      this->threadNotifier.notify_all();
      sendMessage(Message::Abort);
    }
  }

  const SharedMultyMetrics getMetrics()
  {
    return threadMetrics;
  }

private:

  bool run(const size_t threadIndex) override
  {
    try
    {
      while(this->shouldExit != true
            && (this->noMoreData != true || this->notificationCount > 0))
      {
        std::unique_lock<std::mutex> lockNotifier{this->notifierLock};

        this->threadNotifier.wait(lockNotifier, [this]()
        {
          return this->noMoreData || this->notificationCount.load() > 0 || this->shouldExit;
        });        

        if (this->shouldExit == true)
        {
          lockNotifier.unlock();
          break;
        }

        if (this->notificationCount.load() > 0)
        {
          --this->notificationCount;
          lockNotifier.unlock();
          if (log(threadIndex))
          {            
          }
        }
        else
        {
          lockNotifier.unlock();
        }
      }

      /*check if this thread is the only active one */
      std::unique_lock<std::mutex> lockNotifier{this->notifierLock};

      size_t activeThreadCount{};
      for (size_t idx{0}; idx < threadsCount; ++idx)
      {
        if (idx != threadIndex
            && threadFinished[idx] != true)
        {
          ++activeThreadCount;
        }
      }

      if (0 == activeThreadCount)
      {
        //std::cout << "\n                     " << this->workerName<< " AllDataLogged\n";
        //sendMessage(Message::AllDataLogged);
        terminationFlag = true;

        if (true == this->shouldExit)
        {
          abortFlag = true;
        }

        terminationNotifier.notify_all();
      }

      threadFinished[threadIndex] = true;

      lockNotifier.unlock();

      //std::cout << "\n                     " << this->workerName<< " finished\n";

      return true;
    }
    catch (const std::exception& ex)
    {
      errorOut << this->workerName << " thread #" << threadIndex << " stopped. Reason: " << ex.what() << std::endl;

      threadFinished[threadIndex] = true;
      this->shouldExit = true;
      this->threadNotifier.notify_all();

      sendMessage(Message::Abort);

      abortFlag = true;
      terminationNotifier.notify_all();

      return false;
    }
  }

  bool log(const size_t threadIndex)
  {
    if (nullptr == buffer)
    {
      throw(std::invalid_argument{"Logger source buffer not defined!"});
    }

    std::unique_lock<std::mutex> lockBuffer{buffer->dataLock};

    if (buffer->dataSize() == 0)
    {
      lockBuffer.unlock();
      throw std::out_of_range{"Buffer is empty!"};
    }

    auto bufferReply{buffer->getItem(shared_from_this())};    

    lockBuffer.unlock();

    if (false == bufferReply.first)
    {
     // std::cout << "\n                     " << this->workerName<< " FALSE received\n";
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

//    while (std::ifstream {logFileName})
//    {
//      ++additionalNameSection[threadIndex];
//      fileNameSuffix = std::to_string(additionalNameSection[threadIndex])
//                       + "_" + std::to_string(threadIndex + 1);
//      logFileName = bulkFileName+ "_" + fileNameSuffix + ".log";
//    }

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


  std::shared_ptr<SmartBuffer<DataType>> buffer;
  std::string destinationDirectory;
  std::ostream& errorOut;

  size_t previousTimeStamp;
  std::vector<size_t> additionalNameSection;

  SharedMultyMetrics threadMetrics;

  std::vector<bool> threadFinished;

  bool& terminationFlag;
  bool& abortFlag;
  std::condition_variable& terminationNotifier;
};


