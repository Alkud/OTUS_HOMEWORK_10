// input_reader.cpp in Otus homework#7 project

#include "input_reader.h"
#include <string>
#include <stdexcept>
#include <mutex>

InputReader::InputReader(std::istream& newInput, std::mutex& newInputLock,
                         const std::shared_ptr<SmartBuffer<std::string> >& newBuffer) :
  input{newInput},
  inputLock{newInputLock},
  buffer{newBuffer},
  shouldExit{false}
{
  if (nullptr == buffer)
  {
    throw(std::invalid_argument{"Input reader destination buffer not defined!"});
  }
}

InputReader::~InputReader()
{
  std::cout << "IR destructor\n";
}



void InputReader::read()
{
  std::string nextString{};
  try
  {
    std::lock_guard<std::mutex> lockInput{inputLock};
    while(shouldExit!= true
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
  }
  catch(std::exception& ex)
  {
    //std::cout << "\n                     reader ABORT\n";

    sendMessage(Message::Abort);
    std::cerr << ex.what();
  }
  sendMessage(Message::NoMoreData);
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
