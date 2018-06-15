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
    ++notificationCount;
    threadNotifier.notify_one();
  }
}

void Publisher::reactMessage(MessageBroadcaster* sender, Message message)
{
  switch(message)
  {
  case Message::NoMoreData :
    if (buffer.get() == sender)
    {
      //std::cout << "\n                    publisher NoMoreData received\n";
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

const SharedMetrics Publisher::getMetrics()
{
  return threadMetrics;
}

void Publisher::onTermination(const size_t threadIndex)
{

}

void Publisher::onThreadException(const std::exception& ex, const size_t threadIndex)
{

}

bool Publisher::threadProcess(const size_t threadIndex)
{

}

bool Publisher::run(const size_t)
{
  try
  {
    while(noMoreData != true)
    {
      if (true == shouldExit)
      {
        break;
      }

      if (notificationCount.load() > 0)
      {
        if (publish() == true)
        {
          --notificationCount;
        }
      }
      else if (noMoreData != true)
      {
        /* wait for new notifications or NoMoreData message */
        std::unique_lock<std::mutex> lockNotifier{notifierLock};
        threadNotifier.wait(lockNotifier, [this]()
        {
          return shouldExit || noMoreData || notificationCount.load() > 0;
        });
        lockNotifier.unlock();
      }
    }

    if (shouldExit != true)
    {
      while (notificationCount.load() > 0)
      {
        if (publish() == true)
        {
          --notificationCount;
        }
        else
        {
          //std::cout << "\n                    publisher has wrong notification count \n";
          break;
        }
      }
    }

    //sendMessage(Message::AllDataPublsihed);
    terminationFlag = true;
    if (true == shouldExit)
    {
      abortFlag = true;
    }
    terminationNotifier.notify_all();

    return true;
  }
  catch (const std::exception& ex)
  {
    errorOut << workerName << " stopped. Reason: " << ex.what() << std::endl;

    sendMessage(Message::Abort);

    abortFlag = true;
    terminationNotifier.notify_all();

    return false;
  }
}

bool Publisher::publish()
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
