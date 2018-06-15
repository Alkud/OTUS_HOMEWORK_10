// input_processor.cpp in Otus homework#7 project

#include "input_processor.h"

InputProcessor::InputProcessor(const size_t& newBulkSize, const char& newBulkOpenDelimiter, const char& newBulkCloseDelimiter,
                               const std::shared_ptr<SmartBuffer<std::string> >& newInputBuffer,
                               const std::shared_ptr<SmartBuffer<std::pair<size_t, std::string> > >& newOutputBuffer,
                               std::ostream& newErrorOut) :
  bulkSize{newBulkSize > 1 ? newBulkSize : 1},
  bulkOpenDelimiter{newBulkOpenDelimiter},
  bulkCloseDelimiter{newBulkCloseDelimiter},
  inputBuffer{newInputBuffer},
  outputBuffer{newOutputBuffer},
  customBulkStarted{false},
  nestingDepth{0},
  errorOut{newErrorOut},
  threadMetrics{std::make_shared<ThreadMetrics>("input processor")}
{}

InputProcessor::~InputProcessor()
{
}

void InputProcessor::reactNotification(NotificationBroadcaster* sender)
{
  if (inputBuffer.get() == sender)
  {
    try
    {
      std::unique_lock<std::mutex> lockInputBuffer{inputBuffer->dataLock};
      auto bufferReply{inputBuffer->getItem(shared_from_this())};
      lockInputBuffer.unlock();

      if (false == bufferReply.first)
       {
         return;
       }

      auto nextCommand{bufferReply.second};
      ++threadMetrics->totalStringCount;

      if (bulkOpenDelimiter == nextCommand)          // bulk open command received
      {
        /* if a custom bulk isn't started,
         * send accumulated commands to the output buffer,
         * then start a new custom bulk */
        if (customBulkStarted == false)
        {
          startNewBulk();
        }

        ++nestingDepth;
      }
      else if (bulkCloseDelimiter == nextCommand)    // bulk close command received
      {
        if (nestingDepth >= 1)
        {
           --nestingDepth;
        }

        /* if a custom bulk is started,
        * send accumulated commands to the output buffer,
        * then label custom bulk as closed */
        if (true == customBulkStarted &&
           0 == nestingDepth)
        {
         closeCurrentBulk();
        }
      }
      else                                           // any other command received
      {
       /* if no custom bulk started and temporary buffer is empty,
        * reset bulk start time */
       if (false == customBulkStarted &&
           true == tempBuffer.empty())
       {
         bulkStartTime = std::chrono::system_clock::now();
       }
       /* put new command to the temporary buffer */
       addCommandToBulk(std::move(nextCommand));
       /* if custom bulk isn't started,
        * and current bulk is complete,
        * send it to the output buffer */
       if (tempBuffer.size() == bulkSize &&
           customBulkStarted == false)
       {
         sendCurrentBulk();
       }
     }
    }
    catch(std::exception& ex)
    {
      //std::cout << "\n                     processor ABORT\n";

      sendMessage(Message::Abort);
      std::cerr << ex.what();
    }
  }
}

void InputProcessor::reactMessage(MessageBroadcaster* sender, Message message)
{
  switch (message)
  {
  case Message::NoMoreData:
    if (inputBuffer.get() == sender)
    {
      if (customBulkStarted != true)
      {
        closeCurrentBulk();
      }
      sendMessage(Message::NoMoreData);
     }
     break;

  case Message::Abort :
    if (customBulkStarted != true)
    {
      closeCurrentBulk();
    }
    sendMessage(Message::Abort);
    break;

   default:
     break;
  }
}

const SharedMetrics InputProcessor::getMetrics()
{
  return threadMetrics;
}

void InputProcessor::sendCurrentBulk()
{
  if (tempBuffer.empty() == true)
  {
    return;
  }

  /* concatenate commands to a bulk */
  std::string newBulk{"bulk: "};
  auto iter{tempBuffer.begin()};
  newBulk += *iter;
  ++iter;
  for (; iter != tempBuffer.end(); ++iter)
  {
    newBulk += (", " + *iter);
  }

  /* convert bulk start time to integer ticks count */
  auto ticksCount{
    std::chrono::duration_cast<std::chrono::seconds>
    (
      bulkStartTime.time_since_epoch()
    ).count()
  };

  /* send the bulk to the output buffer */
  {
    std::lock_guard<std::mutex> lockOutputBuffer{outputBuffer->dataLock};
    outputBuffer->putItem(std::make_pair(ticksCount, newBulk));
  }

  /* Refresh metrics */
  threadMetrics->totalCommandCount += tempBuffer.size();
  ++threadMetrics->totalBulkCount;

  /*clear temporary buffer */
  tempBuffer.clear();
}

void InputProcessor::startNewBulk()
{
  sendCurrentBulk();
  bulkStartTime = std::chrono::system_clock::now();
  customBulkStarted = true;
}


void InputProcessor::closeCurrentBulk()
{
  sendCurrentBulk();
  customBulkStarted = false;
}

void InputProcessor::addCommandToBulk(std::string&& newCommand)
{
  tempBuffer.push_back(std::move(newCommand));
}


