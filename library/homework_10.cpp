// homework_10.cpp in Otus homework#10 project

#include "homework_10.h"
#include "command_processor_mt.h"

void homework(int argc, char* argv[], std::istream& inputStream, std::ostream& outputStream,
              std::ostream& errorStream, std::ostream& metricsStream)
{
  int commandLineParam{};
  try
  {
    commandLineParam = argc < 2 ? -1 : std::stoi(argv[1]);
  }
  catch(const std::exception& ex)
  {
    outputStream << "\nOnly integer numbers are allowed";
    commandLineParam = -1;
  }

  std::string userInput{};
  if (commandLineParam < 1)
  {
    while (commandLineParam < 1)
    {
      outputStream << "\nPlease enter bulk size (must be greater than 0): ";
      if (!std::getline(inputStream, userInput))
      {
        outputStream << std::endl;
        return;
      }
      try
      {
        commandLineParam = std::stoi(userInput);
      }
      catch(const std::exception& ex)
      {
        outputStream << "\nOnly integer numbers are allowed";
        commandLineParam = -1;
      }
    }
  }

  const size_t bulkSize{commandLineParam};

  const CommandProcessor<2> processor{inputStream, outputStream, errorStream, metricsStream, bulkSize, '{', '}'};

  processor.run();
}
