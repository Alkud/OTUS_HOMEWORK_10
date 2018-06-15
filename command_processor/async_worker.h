// async_worker.h in Otus homework#11 project

#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <future>
#include <cassert>

template<size_t workingThreadsCount = 1u>
class AsyncWorker
{
public:
  AsyncWorker() = delete;

  AsyncWorker(const std::string& newWorkerName) :
    shouldExit{false}, noMoreData{false}, isStopped{true}, notificationCount{},
    threadFinished{}, workerName{newWorkerName}
  {
    futureResults.reserve(workingThreadsCount);
    threadFinished.resize(workingThreadsCount, false);
  }

  virtual ~AsyncWorker()
  {
    //std::cout << "\n                    " << workerName << " destructor\n";
    assert(isStopped == true);
  }

  virtual void start()
  {
    startWorkingThreads();
  }

  virtual bool startAndWait()
  {
    startWorkingThreads();
    if (futureResults.empty() != true)
    {
      return waitForThreadTermination();
    }
    else
    {
      return false;
    }
  }

  void stop()
  {
    if (true == isStopped)
    {
      return;
    }

    shouldExit = true;
    threadNotifier.notify_all();

    for (auto& result : futureResults)
    {
      if (result.wait_for(std::chrono::milliseconds(0))
          != std::future_status::ready)
      {
        result.wait_for(std::chrono::milliseconds(150));
      }
    }
    isStopped = true;
  }

protected:

  void startWorkingThreads()
  {
    if (futureResults.empty() != true)
    {
      return;
    }

    /* start working threads */
    for (size_t threadIndex{0}; threadIndex < workingThreadsCount; ++threadIndex)
    {
      futureResults.push_back(
        std::async(
          std::launch::async,
          &AsyncWorker<workingThreadsCount>::run,
          this, threadIndex
        )
      );
    }
    isStopped = false;
  }

  bool waitForThreadTermination()
  {
    /* wait for working threads results */
    bool workSuccess{true};
    for (auto& result : futureResults)
    {
      workSuccess = workSuccess && result.get();
    }

    return workSuccess;
  }

  virtual bool run(const size_t threadIndex)
  {
    try
    {
      while(this->shouldExit != true
            && (this->noMoreData != true || this->notificationCount > 0))
      {
        std::unique_lock<std::mutex> lockNotifier{this->notifierLock};

        if (notificationCount.load() > 0)
        {
          --notificationCount;
          lockNotifier.unlock();
          threadProcess(threadIndex);
        }
        else
        {
          std::unique_lock<std::mutex> lockControl{controlLock};
          if (shouldExit != true || noMoreData != true)
          {
            lockControl.unlock();
            std::cout << "\n                     " << this->workerName<< " waiting. shouldExit="<< shouldExit << ", noMoreData=" << noMoreData << "\n";
            threadNotifier.wait_for(lockNotifier, std::chrono::seconds(1), [this]()
            {
              return this->noMoreData || this->notificationCount.load() > 0 || this->shouldExit;
            });
          }
          else
          {
            lockControl.unlock();
          }
          lockNotifier.unlock();
        }
      }

      /*check if this thread is the only active one */
      std::unique_lock<std::mutex> lockTermination{terminationLock};

      size_t activeThreadCount{};
      for (size_t idx{0}; idx < workingThreadsCount; ++idx)
      {
        if (idx != threadIndex
            && threadFinished[idx] != true)
        {
          ++activeThreadCount;
        }
      }

      std::cout << "\n                     " << this->workerName<< " activeThreadCount=" << activeThreadCount << "\n";

      if (0 == activeThreadCount)
      {
        std::cout << "\n                     " << this->workerName<< " finishing\n";
        onTermination(threadIndex);
      }

      threadFinished[threadIndex] = true;

      lockTermination.unlock();

      std::cout << "\n                     " << this->workerName<< " finished\n";

      return true;
    }
    catch (const std::exception& ex)
    {
      onThreadException(ex, threadIndex);
      return false;
    }
  }

  virtual bool threadProcess(const size_t threadIndex) = 0;

  virtual void onThreadException(const std::exception& ex, const size_t threadIndex) = 0;

  virtual void onTermination(const size_t threadIndex) = 0;

  std::vector<std::future<bool>> futureResults{};
  std::mutex controlLock;
  bool shouldExit;
  bool noMoreData;
  bool isStopped;

  std::atomic<size_t> notificationCount;
  std::condition_variable threadNotifier{};
  std::mutex notifierLock;

  std::vector<bool> threadFinished;
  std::mutex terminationLock;

  const std::string workerName;
};
