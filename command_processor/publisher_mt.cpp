// publisher.cpp in Otus homework#7 project

#include "publisher_mt.h"


Publisher::Publisher(const std::shared_ptr<SmartBuffer<std::pair<size_t, std::string> > >& newBuffer,
                     std::ostream& newOutput, std::mutex& newOutpuLock,
                     std::ostream& newErrorOut, std::ostream& newMetricksOut) :
  buffer{newBuffer}, output{newOutput}, outputLock{newOutpuLock},
  shouldExit{false}, notificationsCount{0},
  workingThread{}, threadMetrics{},
  errorOut{newErrorOut}, metricsOut{newMetricksOut}
{
  if (nullptr == buffer)
  {
    throw(std::invalid_argument{"Publisher source buffer not defined!"});
  }
}

Publisher::~Publisher()
{
  shouldExit = true;
  threadNotifier.notify_all();
  if (workingThread.joinable())
  {
    workingThread.join();
  }

  /* Output metrics */
  metricsOut << "log thread"
             << " - " << threadMetrics.totalBulksCount << " bulk(s), "
             << threadMetrics.totalCommandsCount << " command(s)" << std::endl;
}

void Publisher::reactNotification(NotificationBroadcaster* sender)
{
  if (buffer.get() == sender)
  {
    std::lock_guard<std::mutex> lockNotifier{notifierLock};
    ++notificationsCount;
    threadNotifier.notify_one();
  }
}

void Publisher::reactMessage(MessageBroadcaster* sender, Message message)
{
  if (Message::NoMoreData == message)
  {
    shouldExit = true;
    threadNotifier.notify_one();
  }
}

void Publisher::start()
{
  if (workingThread.joinable() != true)
  {
    workingThread = std::thread{&Publisher::run, this};
  }
}

void Publisher::run()
{
  try
  {
    while(shouldExit != true)
    {

      std::unique_lock<std::mutex> lockNotifier{notifierLock};

      threadNotifier.wait(lockNotifier, [this](){return this->notificationsCount.load() > 0 || shouldExit;});

      lockNotifier.unlock();

      if (notificationsCount.load() > 0)
      {
        if (publish() == true)
        {
          --notificationsCount;
        }
      }
    }
  }
  catch (const std::exception& ex)
  {
    sendMessage(Message::AllDataReceived);
    shouldExit = true;
    errorOut << "Publisher stopped. Reason: " << ex.what() << std::endl;
  }
}

bool Publisher::publish()
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

  std::lock_guard<std::mutex> lockOutput{outputLock};
  output << nextBulkInfo.second << '\n';

  /* Refresh metrics */
  ++threadMetrics.totalBulksCount;
  threadMetrics.totalCommandsCount
      += std::count(nextBulkInfo.second.begin(),
                    nextBulkInfo.second.end(), ',') + 1;

  return true;
}
