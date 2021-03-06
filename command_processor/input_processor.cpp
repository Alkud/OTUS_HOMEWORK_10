// input_processor.cpp in Otus homework#7 project

#include "input_processor.h"

InputProcessor::InputProcessor(const size_t& newBulkSize, const char& newBulkOpenDelimiter, const char& newBulkCloseDelimiter,
                               const std::shared_ptr<SmartBuffer<std::string> >& newInputBuffer,
                               const std::shared_ptr<SmartBuffer<std::pair<size_t, std::string> > >& newOutputBuffer,
                               std::ostream& newErrorOut, std::mutex& newErrorOutLock) :
  bulkSize{newBulkSize},
  bulkOpenDelimiter{newBulkOpenDelimiter},
  bulkCloseDelimiter{newBulkCloseDelimiter},
  inputBuffer{newInputBuffer},
  outputBuffer{newOutputBuffer},
  customBulkStarted{false},
  nestingDepth{0},
  shouldExit{false},
  errorOut{newErrorOut}, errorOutLock{newErrorOutLock},
  threadMetrics{std::make_shared<ThreadMetrics>("input processor")},
  state{WorkerState::Started}
{
  if (nullptr == inputBuffer)
  {
    throw(std::invalid_argument{"Input processor source buffer not defined!"});
  }

  if (nullptr == outputBuffer)
  {
    throw(std::invalid_argument{"Input processor destination buffer not defined!"});
  }
}

InputProcessor::~InputProcessor()
{
  #ifdef NDEBUG
  #else
    //std::cout << "IP destructor\n";
  #endif
}

void InputProcessor::reactNotification(NotificationBroadcaster* sender)
{
  if (inputBuffer.get() == sender)
  {
    try
    {
      auto bufferReply{inputBuffer->getItem(shared_from_this())};

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
      #ifdef NDEBUG
      #else
        //std::cout << "\n                     processor ABORT\n";
      #endif

      {
        std::lock_guard<std::mutex> lockErrorOut{errorOutLock};
        std::cerr << ex.what();
      }
      shouldExit = true;
      sendMessage(Message::SystemError);
    }
  }
}

void InputProcessor::reactMessage(MessageBroadcaster* sender, Message message)
{
  if (messageCode(message) < 1000) // non error message
  {
    switch(message)
    {
    case Message::NoMoreData :
      if (inputBuffer.get() == sender)
      {
        #ifdef NDEBUG
        #else
          //std::cout << "\n                     processor NoMoreData received\n";
        #endif

        if (customBulkStarted != true)
        {
          #ifdef NDEBUG
          #else
            //std::cout << "\n                     processor trying close last bulk\n";
          #endif

          closeCurrentBulk();
        }
        sendMessage(Message::NoMoreData);

        #ifdef NDEBUG
        #else
          //std::cout << "\n                     processor NoMoreData sent\n";
        #endif

        state.store(WorkerState::Finished);
       }
      break;

    default:
      break;
    }
  }
  else                             // error message
  {
    if (shouldExit != true)
    {
      shouldExit = true;
      sendMessage(message);
      state.store(WorkerState::Finished);
    }
  }
}

const SharedMetrics InputProcessor::getMetrics()
{
  return threadMetrics;
}

void InputProcessor::setBulkSize(const size_t newBulkSize)
{
  bulkSize = newBulkSize;
}

WorkerState InputProcessor::getWorkerState()
{
  return state.load();
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
  outputBuffer->putItem(std::make_pair(ticksCount, newBulk));

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
