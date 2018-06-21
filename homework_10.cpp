// homework_10.cpp in Otus homework#10 project

#include <array>
#include <utility>
#include "homework_10.h"
#include "./command_processor/command_processor_mt.h"

int homework(int argc, char* argv[], std::istream& inputStream, std::ostream& outputStream,
              std::ostream& errorStream, std::ostream& metricsStream)
{
  if (argc < 2 || std::stoull(std::string{argv[1]}) < 1)
  {
    errorStream << "usage: bulkmt [bulk size]" << std::endl;
    return 1;
  }

  size_t bulkSize{std::stoull(std::string{argv[1]})};

  CommandProcessor<2> processor{inputStream, outputStream, errorStream, metricsStream, bulkSize, '{', '}'};
  processor.run();

  return 0;
}
