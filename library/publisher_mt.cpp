// publisher.cpp in Otus homework#7 project

#include "publisher_mt.h"


Publisher::Publisher(const std::shared_ptr<SmartBuffer<std::pair<size_t, std::string> > >& newBuffer,
                     std::ostream& newOutput, std::mutex& newOutpuLock,
                     std::ostream& newErrorOut, std::ostream& newMetricksOut) :
  buffer{newBuffer}, output{newOutput}, outputLock{newOutpuLock},
  shouldExit{false}, notificationsCount{},
  workingThread{}, threadMetrics{},
  errorOut{newErrorOut}, metricsOut{newMetricksOut}
{
  if (nullptr == buffer)
  {
    throw(std::invalid_argument{"Publisher source buffer not defined!"});
  }
  workingThread = std::thread{&Publisher::run, this};
}

Publisher::~Publisher()
{
  shouldExit = true;
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
    ++notificationsCount;
  }
}

void Publisher::run()
{
  try
  {
    while(shouldExit != true)
    {
      if (notificationsCount.load() > 0)
      {
        publish();
        --notificationsCount;
      }
    }
  }
  catch (const std::exception& ex)
  {
    errorOut << "Publisher stopped. Reason: " << ex.what() << std::endl;
    shouldExit = true;
  }
}

void Publisher::publish()
{
  auto nextBulkInfo{buffer->getItem(shared_from_this())};
  std::lock_guard<std::mutex> lockOutput{outputLock};
  output << nextBulkInfo.second << '\n';

  /* Refresh metrics */
  ++threadMetrics.totalBulksCount;
  threadMetrics.totalCommandsCount
      += std::count(nextBulkInfo.second.begin(),
                    nextBulkInfo.second.end(), ',') + 1;
}
