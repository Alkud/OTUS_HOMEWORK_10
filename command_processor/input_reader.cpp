// input_reader.cpp in Otus homework#7 project

#include "input_reader.h"
#include <string>
#include <stdexcept>
#include <mutex>

InputReader::InputReader(std::istream& newInput, std::mutex& newInputLock,
                         const SharedStringBuffer& newBuffer, std::ostream& newErrorOut, std::mutex& newErrorOutLock) :
  input{newInput},
  inputLock{newInputLock},
  buffer{newBuffer},
  shouldExit{false},
  errorOut{newErrorOut}, errorOutLock{newErrorOutLock},
  state{WorkerState::NotStarted}
{
  if (nullptr == buffer)
  {
    throw(std::invalid_argument{"Input reader destination buffer not defined!"});
  }
}

InputReader::~InputReader()
{
  #ifdef NDEBUG
  #else
    //std::cout << "IR destructor\n";
  #endif
}



void InputReader::read()
{
  std::string nextString{};
  state.store(WorkerState::Started);
  try
  {
    std::lock_guard<std::mutex> lockInput{inputLock};
    while(shouldExit != true
          && std::getline(input, nextString))
    {
      if (nextString.size() > (size_t)InputReaderSettings::MaxInputStringSize)
      {
        std::cerr << "Maximum command length exceeded! String truncated";
        nextString = nextString.substr((size_t)InputReaderSettings::MaxInputStringSize);
      }      
      buffer->putItem(nextString);
    }
    sendMessage(Message::AllDataReceived);
    sendMessage(Message::NoMoreData);
    state.store(WorkerState::Finished);
  }
  catch(std::exception& ex)
  {
    #ifdef NDEBUG
    #else
      //std::cout << "\n                     reader ABORT\n";
    #endif

    {
      std::lock_guard<std::mutex> lockErrorOut{errorOutLock};
      errorOut << ex.what();
    }

    sendMessage(Message::SystemError);
    state.store(WorkerState::Finished);
  }

}

void InputReader::reactMessage(MessageBroadcaster* sender, Message message)
{
  if (messageCode(message) > 1000           // error message
      && shouldExit != true)
  {
    shouldExit = true;
    sendMessage(message);
  }
}

WorkerState InputReader::getWorkerState()
{
  return state.load();
}
