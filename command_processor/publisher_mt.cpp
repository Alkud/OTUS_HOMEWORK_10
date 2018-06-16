// publisher.cpp in Otus homework#11 project

#include "publisher_mt.h"


Publisher::Publisher(const std::string& newWorkerName,
                     const std::shared_ptr<SmartBuffer<std::pair<size_t, std::string> > >& newBuffer, bool& newTerminationFlag, bool& newAbortFlag, std::condition_variable& newTerminationNotifier,
                     std::ostream& newOutput, std::mutex& newOutpuLock,
                     std::ostream& newErrorOut) :
  AsyncWorker<1>{newWorkerName},
  buffer{newBuffer}, output{newOutput}, outputLock{newOutpuLock},
  threadMetrics{std::make_shared<ThreadMetrics>("publisher")},
  errorOut{newErrorOut},
  terminationFlag{newTerminationFlag}, abortFlag{newAbortFlag},
  terminationNotifier{newTerminationNotifier}
{
  if (nullptr == buffer)
  {
    throw(std::invalid_argument{"Publisher source buffer not defined!"});
  }
}

Publisher::~Publisher()
{
  stop();
}

void Publisher::reactNotification(NotificationBroadcaster* sender)
{
  if (buffer.get() == sender)
  {
    #ifdef _DEBUG
      std::cout << this->workerName << " reactNotification\n";
    #endif

    ++notificationCount;    
    threadNotifier.notify_one();
  }
}

void Publisher::reactMessage(MessageBroadcaster* sender, Message message)
{
  switch(message)
  {
  case Message::NoMoreData :
    if (noMoreData != true && buffer.get() == sender)
    {
      #ifdef _DEBUG
        std::cout << "\n                    publisher NoMoreData received\n";
      #endif

      std::lock_guard<std::mutex> lockControl{this->controlLock};
      noMoreData = true;
      threadNotifier.notify_all();
    }
    break;

  case Message::Abort :
    if (shouldExit != true)
    {
      {
        std::lock_guard<std::mutex> lockControl{this->controlLock};
        shouldExit = true;
        threadNotifier.notify_all();
      }
        sendMessage(Message::Abort);
    }
    break;
  }
}

const SharedMetrics Publisher::getMetrics()
{
  return threadMetrics;
}

bool Publisher::threadProcess(const size_t threadIndex)
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
    return false;
  }

  auto nextBulkInfo{bufferReply.second};

  std::lock_guard<std::mutex> lockOutput{outputLock};
  output << nextBulkInfo.second << '\n';

  /* Refresh metrics */
  ++threadMetrics->totalBulkCount;
    threadMetrics->totalCommandCount
      += std::count(nextBulkInfo.second.begin(),
                    nextBulkInfo.second.end(), ',') + 1;

  return true;
}

void Publisher::onThreadException(const std::exception& ex, const size_t threadIndex)
{
  errorOut << this->workerName << " thread #" << threadIndex << " stopped. Reason: " << ex.what() << std::endl;

  threadFinished[threadIndex] = true;
  shouldExit = true;
  threadNotifier.notify_all();

  sendMessage(Message::Abort);

  abortFlag = true;
  terminationNotifier.notify_all();
}

void Publisher::onTermination(const size_t threadIndex)
{
  #ifdef _DEBUG
    std::cout << "\n                     " << this->workerName<< " AllDataLogged\n";
  #endif

  if (true == noMoreData && notificationCount.load() == 0)
  {
    terminationFlag = true;
  }

  if (true == shouldExit)
  {
    abortFlag = true;
  }

  terminationNotifier.notify_all();
}
