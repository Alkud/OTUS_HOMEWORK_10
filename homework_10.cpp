// homework_10.cpp in Otus homework#10 project

#include <array>
#include <utility>
#include "homework_10.h"
#include "./command_processor/command_processor_mt.h"

constexpr size_t maxThreadCount {20};

template<size_t... I>
constexpr auto
buildProcessorTemplatesHelper(std::index_sequence<I...>)
-> std::array<std::shared_ptr<AbstractProcessor>, sizeof... (I)>
{
  return {std::make_shared<CommandProcessor<I + 1>>(
          std::ref(std::cin), std::ref(std::cout), std::ref(std::cerr), std::ref(std::cout), 1, '{', '}')...};
}

template<size_t N>
constexpr auto
buildProcessorTemplates()
{
  return buildProcessorTemplatesHelper(std::make_index_sequence<N>{});
}

auto processorTemplates {buildProcessorTemplates<maxThreadCount>()};



int homework(int argc, char* argv[], std::istream& inputStream, std::ostream& outputStream,
              std::ostream& errorStream, std::ostream& metricsStream)
{
  if (argc < 2 || std::stoull(std::string{argv[1]}) < 1)
  {
    errorStream << "usage: bulkmt [bulk size] {[number of logging threads]}" << std::endl;
    return 1;
  }

  size_t bulkSize{std::stoull(std::string{argv[1]})};

  size_t loggingThreadCount{2};

  if (3 == argc)
  {
    loggingThreadCount = std::stoull(std::string{argv[2]});
    processorTemplates[loggingThreadCount - 1]->setBulkSize(bulkSize);
    processorTemplates[loggingThreadCount - 1]->run();
  }
  else
  {
    CommandProcessor<2> processor{inputStream, outputStream, errorStream, metricsStream, bulkSize, '{', '}'};
    processor.run();
  }
  return 0;
}
