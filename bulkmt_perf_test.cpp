// bulkmt_perf_test.cpp in Otus homework#10 project

#include <stdexcept>
#include <iostream>
#include <array>
#include <sstream>
#include <utility>
#include "./command_processor/command_processor_mt.h"

constexpr size_t maxThreadCount {300};

std::stringstream inputStream;

template<size_t... I>
constexpr auto
buildProcessorTemplatesHelper(std::index_sequence<I...>)
-> std::array<std::shared_ptr<AbstractProcessor>, sizeof... (I)>
{
  return {std::make_shared<CommandProcessor<I + 1>>(
          std::ref(inputStream), std::ref(std::cout), std::ref(std::cerr), std::ref(std::cout), 1, '{', '}')...};
}

template<size_t N>
constexpr auto
buildProcessorTemplates()
{
  return buildProcessorTemplatesHelper(std::make_index_sequence<N>{});
}

auto processorTemplates {buildProcessorTemplates<maxThreadCount>()};



int perf_test(int argc, char* argv[])
{
  if (argc < 2 || std::stoull(std::string{argv[1]}) < 1)
  {
    std::cerr << "usage: bulkmt_perf_test [bulk size] {[number of logging threads]} {[number of commands]}" << std::endl;
    return 1;
  }

  size_t bulkSize{std::stoull(std::string{argv[1]})};

  size_t loggingThreadCount{2};

  size_t testCommandCount{10000};

  if (argc > 2)
  {
    loggingThreadCount = std::stoull(std::string{argv[2]});
  }

  if (argc > 3)
  {
    testCommandCount = std::stoull(std::string{argv[3]});
  }

  processorTemplates[loggingThreadCount - 1]->setBulkSize(bulkSize);

  for (size_t i{0}; i < testCommandCount; ++i)
  {
    inputStream << i << '\n' ;
  }

  processorTemplates[loggingThreadCount - 1]->run();

  return 0;
}

int main(int argc, char* argv[])
{
  try
  {
    return(perf_test(argc, argv));
  }
  catch(const std::exception& ex)
  {
    std::cerr << ex.what();
    return 0;
  }
}
