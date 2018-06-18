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
  #ifdef _DEBUG
    std::cout << "IR destructor\n";
  #endif
}



void InputReader::read()
{
  std::string nextString{};
  state = WorkerState::Started;
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
      std::unique_lock<std::mutex> lockBuffer{buffer->dataLock};
      buffer->putItem(nextString);
      lockBuffer.unlock();
    }
    sendMessage(Message::NoMoreData);
    state = WorkerState::Finished;
  }
  catch(std::exception& ex)
  {
    #ifdef _DEBUG
      std::cout << "\n                     reader ABORT\n";
    #endif

    sendMessage(Message::Abort);
    std::lock_guard<std::mutex> lockErrorOut{errorOutLock};
    errorOut << ex.what();
    state = WorkerState::Finished;
  }

}

void InputReader::reactMessage(MessageBroadcaster* sender, Message message)
{
  switch(message)
  {
  case Message::Abort :
    if (shouldExit != true)
    {
      shouldExit = true;
      sendMessage(Message::Abort);
    }
    break;
  }
}

WorkerState InputReader::getWorkerState()
{
  return state;
}
