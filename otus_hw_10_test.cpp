// otus_hw_10_test.cpp in Otus homework#10 project

#define BOOST_TEST_MODULE OTUS_HW_10_TEST

#include <boost/test/unit_test.hpp>
#include "homework_10.h"
#include "./command_processor/command_processor_mt.h"

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <vector>

enum class DebugOutput
{
  debug_on,
  debug_off
};

/* Helper functions */
std::array<std::vector<std::string>, 3>
getProcessorOutput
(
  const std::string& inputString,
  char openDelimiter,
  char closeDelimiter,
  size_t bulkSize,
  DebugOutput debugOutput
)
{
  std::stringstream inputStream{inputString};
  std::stringstream outputStream{};
  std::stringstream errorStream{};
  std::stringstream metricsStream{};

  {
  CommandProcessor<2> testProcessor {
    inputStream, outputStream, errorStream, metricsStream,
    bulkSize, openDelimiter, closeDelimiter
  };

  testProcessor.run();

  //std::this_thread::sleep_for(std::chrono::milliseconds{200});

  }

  std::array<std::vector<std::string>, 3> result {};

  std::string tmpString{};
  while(std::getline(outputStream, tmpString))
  {
    if (DebugOutput::debug_on == debugOutput)
    {
      std::cout << tmpString << '\n';
    }
    result[0].push_back(tmpString);
  }

  while(std::getline(errorStream, tmpString))
  {
    result[1].push_back(tmpString);
  }

  while(std::getline(metricsStream, tmpString))
  {
    result[2].push_back(tmpString);
  }

  return result;
}


auto parseMetrics(std::stringstream& metricsStream)
{
  std::string threadMetrics{};
  size_t searchStartPosition{};
  size_t digitPosition{};

  std::vector<std::vector<int>> result;
  while (std::getline(metricsStream, threadMetrics))
  {
    result.push_back(std::vector<int>{});
    int nextNumber{};
    std::string tempString{};
    std::stringstream threadMetricsStream{threadMetrics};
    while (!threadMetricsStream.eof())
    {
      threadMetricsStream >> tempString;
      if (std::stringstream{tempString} >> nextNumber)
      {
        result.back().push_back(nextNumber);
      }
      tempString = " ";
    }
  }
  return result;
}

void checkMetrics(std::stringstream& metricsStream,
                  int stringsExpected, int commandsExpected, int bulksExpected)
{
  auto numericMetrics {parseMetrics(metricsStream)};

  BOOST_CHECK(numericMetrics.size() == 4);
  BOOST_CHECK(numericMetrics[0].size() == 3);
  BOOST_CHECK(numericMetrics[1].size() == 2);
  BOOST_CHECK(numericMetrics[2].size() == 2);
  BOOST_CHECK(numericMetrics[3].size() == 2);

  int mainStringsCount{numericMetrics[0][0]};
  int mainCommandsCount{numericMetrics[0][1]};
  int mainBulksCount{numericMetrics[0][2]};

  int logBulksCount{numericMetrics[1][0]};
  int logCommandsCount{numericMetrics[1][1]};

  int fileOneBulksCount{numericMetrics[2][0]};
  int fileOneCommandsCount{numericMetrics[2][1]};

  int fileTwoBulksCount{numericMetrics[3][0]};
  int fileTwoCommandsCount{numericMetrics[3][1]};

  BOOST_CHECK(mainStringsCount == stringsExpected);
  BOOST_CHECK(mainCommandsCount == commandsExpected);
  BOOST_CHECK(mainBulksCount == bulksExpected);

  BOOST_CHECK(logCommandsCount == commandsExpected);
  BOOST_CHECK(logBulksCount == bulksExpected);

  BOOST_CHECK(fileOneCommandsCount + fileTwoCommandsCount == commandsExpected);
  BOOST_CHECK(fileOneBulksCount + fileTwoBulksCount == bulksExpected);
}

BOOST_AUTO_TEST_SUITE(homework_10_test)

BOOST_AUTO_TEST_CASE(objects_creation_failure)
{
  #ifdef NDEBUG
  #else
   // std::cout << "objects_creation_failure test\n";
  #endif

  std::mutex dummyMutex{};

  /* can't create input reader with null buffer pointer */
  BOOST_CHECK_THROW((InputReader{std::cin, dummyMutex, nullptr, std::cerr, dummyMutex}), std::invalid_argument);

  /* can't create publisher with null buffer pointer */
  BOOST_CHECK_THROW((Publisher{"publisher", nullptr, std::cout, dummyMutex, std::cerr, dummyMutex}), std::invalid_argument);

  /* can't create logger with null buffer pointer */
  BOOST_CHECK_THROW((Logger<2>{"logger", nullptr, std::cerr, dummyMutex, ""}), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(log_file_creation_failure)
{
  #ifdef NDEBUG
  #else
    //std::cout << "log_file_creation_failure test\n";
  #endif

  std::stringstream errorStream{};
  std::mutex errorStreamLock;
  std::stringstream metricsStream{};
  std::string badDirectoryName{"/non_existing_directory/"};
  SharedMultyMetrics metrics{};

  {
    const auto bulkBuffer{std::make_shared<SmartBuffer<std::pair<size_t, std::string>>>("bulk buffer", errorStream, errorStreamLock)};
    const auto dummyBroadcaster {std::make_shared<MessageBroadcaster>()};

    /* use bad directory name as constructor parameter */
    const auto badLogger{
      std::make_shared<Logger<2>>(
            "bad logger", bulkBuffer,
            errorStream, errorStreamLock,
            badDirectoryName
            )
    };

    /* connect buffer to logger */
    dummyBroadcaster->addMessageListener(bulkBuffer);
    bulkBuffer->addNotificationListener(badLogger);
    bulkBuffer->addMessageListener(badLogger);

    bulkBuffer->start();

    badLogger->start();

    /* putting some data to the buffer results in error message */
    bulkBuffer->putItem(std::make_pair<size_t, std::string>(1234, "bulk"));

    dummyBroadcaster->sendMessage(Message::NoMoreData);

    std::this_thread::sleep_for(std::chrono::milliseconds{200});

    /* get metrics */
    metrics = badLogger->getMetrics();
  }

  /* get error message string */
  std::string errorMessage{errorStream.str()};

  /* error message sholud contain expected text */
  BOOST_CHECK(errorMessage.find("Cannot create log file") != std::string::npos);


  /* metrics sholud contain expected values */
  BOOST_CHECK(metrics[0]->totalBulkCount == 0
              && metrics[0]->totalCommandCount == 0
              && metrics[1]->totalBulkCount == 0
              && metrics[1]->totalCommandCount == 0);
}

BOOST_AUTO_TEST_CASE(trying_get_from_empty_buffer)
{
  #ifdef NDEBUG
  #else
    //std::cout << "trying_get_from_empty_buffer test\n";
  #endif

  std::mutex dummyMutex;

  const auto emptyBuffer{
    std::make_shared<SmartBuffer<std::pair<size_t, std::string>>>(
          "empty buffer", std::cerr, dummyMutex
          )
  };

  /* create a dummy message broadcaster */
  MessageBroadcaster dummyBroadcaster{};
  dummyBroadcaster.addMessageListener(emptyBuffer);

  emptyBuffer->start();

  /* emptyBuffer.getItem() should throw an exception */
  BOOST_CHECK_THROW((emptyBuffer->getItem()), std::out_of_range);
  dummyBroadcaster.sendMessage(Message::NoMoreData);
}



BOOST_AUTO_TEST_CASE(no_command_line_parameters)
{
  #ifdef NDEBUG
  #else
    //std::cout << "no_command_line_parameters test\n";
  #endif

  try
  {
    std::stringstream inputStream{};
    std::stringstream outputStream{};
    std::stringstream errorStream{};
    std::stringstream metricsStream{};
    /* comand line arguments */
    char* arg[]{"/home/user/bulk"};

    {
      BOOST_CHECK(homework(1, arg, inputStream, outputStream, errorStream, metricsStream) == 1);
    }

    /* error output should contain expected text*/
    BOOST_CHECK(errorStream.str() ==
                "usage: bulkmt [bulk size]\n");

    /* application metrics and output sholud be empty */
    BOOST_CHECK(outputStream.str() == ""
                && metricsStream.str() == "");

  }
  catch (const std::exception& ex)
  {
    BOOST_FAIL("");
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(empty_input_test)
{
  #ifdef NDEBUG
  #else
    //std::cout << "empty_input_test test\n";
  #endif

  try
  {
    auto processorOutput{
      getProcessorOutput(std::string{}, '{', '}', 3, DebugOutput::debug_off)
    };

    /* metrics sholud contain expected text */

    BOOST_CHECK(processorOutput[2].size() == 4);

    BOOST_CHECK(processorOutput[2][0] ==
                "main thread - 0 string(s), 0 command(s), 0 bulk(s)");

    BOOST_CHECK(processorOutput[2][1] ==
                "log thread - 0 bulk(s), 0 command(s)");
    BOOST_CHECK(processorOutput[2][2] ==
                "file thread #0 - 0 bulk(s), 0 command(s)");
    BOOST_CHECK(processorOutput[2][3] ==
                "file thread #1 - 0 bulk(s), 0 command(s)");

  }
  catch (const std::exception& ex)
  {
    BOOST_FAIL("");
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(empty_command_test)
{
  #ifdef NDEBUG
  #else
    //std::cout << "empty_command_test test\n";
  #endif

  const std::string testString{"cmd1\n"
                               "\n"
                               "cmd2"};
  try
  {
    auto processorOutput{
      getProcessorOutput(testString, '{', '}', 3, DebugOutput::debug_off)
    };

    /* check main application output */
    BOOST_CHECK(processorOutput[0][0] == "bulk: cmd1, , cmd2");

    /* check application error output */
    BOOST_CHECK(processorOutput[1].size() == 0);

    /* check application metrics output */
    BOOST_CHECK(processorOutput[2].size() == 4);

    std::stringstream metricsStream{};
    for (const auto& tmpString : processorOutput[2])
    {
      metricsStream << tmpString << '\n';
    }

    checkMetrics(metricsStream, 3, 3, 1);
  }
  catch (const std::exception& ex)
  {
    BOOST_FAIL("");
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(bulk_segmentation_test1)
{
  #ifdef NDEBUG
  #else
    //std::cout << "bulk_segmentation_test1 test\n";
  #endif

  try
  {
    const std::string testString{"cmd1\n"
                           "cmd2\n"
                           "cmd3\n"
                           "cmd4"};
    auto processorOutput{
      getProcessorOutput(testString, '{', '}', 3, DebugOutput::debug_off)
    };

    /* main application output */
    BOOST_CHECK(processorOutput[0].size() == 2);

    BOOST_CHECK(processorOutput[0][0] == "bulk: cmd1, cmd2, cmd3");
    BOOST_CHECK(processorOutput[0][1] == "bulk: cmd4");

    /* application error output */
    BOOST_CHECK(processorOutput[1].size() == 0);

    if (processorOutput[1].size() != 0)
    {
      for (const auto& string : processorOutput[1])
      {
        std::cout << string << '\n';
      }
    }

    /* application metrics output */
    BOOST_CHECK(processorOutput[2].size() == 4);

    std::stringstream metricsStream{};
    for (const auto& tmpString : processorOutput[2])
    {
      metricsStream << tmpString << '\n';
    }

    checkMetrics(metricsStream, 4, 4, 2);
  }
  catch (const std::exception& ex)
  {
    BOOST_FAIL("");
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(bulk_segmentation_test2)
{
  #ifdef NDEBUG
  #else
    //std::cout << "bulk_segmentation_test2 test\n";
  #endif

  try
  {
    const std::string testString
    {
      "cmd1\n"
      "<\n"
      "cmd2\n"
      "cmd3\n"
      ">\n"
      "cmd4"
    };
    auto processorOutput{
      getProcessorOutput(testString, '<', '>', 3, DebugOutput::debug_off)
    };

    /* main application output */
    BOOST_CHECK(processorOutput[0].size() == 3);

    if (processorOutput[0].size() != 3)
    {
      for (const auto& string : processorOutput[0])
      {
        std::cout << string << '\n';
      }
    }

    BOOST_CHECK(processorOutput[0][0] == "bulk: cmd1");
    BOOST_CHECK(processorOutput[0][1] == "bulk: cmd2, cmd3");
    BOOST_CHECK(processorOutput[0][2] == "bulk: cmd4");

    /* application error output */
    BOOST_CHECK(processorOutput[1].size() == 0);

    /* application metrics output */
    BOOST_CHECK(processorOutput[2].size() == 4);

    std::stringstream metricsStream{};
    for (const auto& tmpString : processorOutput[2])
    {
      metricsStream << tmpString << '\n';
    }

    checkMetrics(metricsStream, 6, 4, 3);
  }
  catch (const std::exception& ex)
  {
    BOOST_FAIL("");
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(nested_bulks_test)
{
  #ifdef NDEBUG
  #else
    //std::cout << "nested_bulks_test test\n";
  #endif

  try
  {
    const std::string testString{
      "cmd1\n"
      "cmd2\n"
      "cmd3\n"
      "(\n"
        "cmd4\n"
        "cmd5\n"
        "(\n"
          "cmd6\n"
          "(\n"
              "cmd7\n"
          ")\n"
          "cmd8\n"
        ")\n"
        "cmd9\n"
      ")\n"
      "cmd10\n"
      "cmd11\n"
      "cmd12\n"
      "cmd13\n"
    };
    auto processorOutput{
      getProcessorOutput(testString, '(', ')', 4, DebugOutput::debug_off)
    };

    /* main application output */
    BOOST_CHECK(processorOutput[0].size() == 3);
    BOOST_CHECK(processorOutput[0][0] ==
                "bulk: cmd1, cmd2, cmd3");
    BOOST_CHECK(processorOutput[0][1] ==
                "bulk: cmd4, cmd5, cmd6, cmd7, cmd8, cmd9");
    BOOST_CHECK(processorOutput[0][2] ==
                "bulk: cmd10, cmd11, cmd12, cmd13");

    /* application error output */
    BOOST_CHECK(processorOutput[1].size() == 0);

    /* application metrics output */
    BOOST_CHECK(processorOutput[2].size() == 4);

    std::stringstream metricsStream{};
    for (const auto& tmpString : processorOutput[2])
    {
      metricsStream << tmpString << '\n';
    }

    checkMetrics(metricsStream, 19, 13, 3);
  }
  catch (const std::exception& ex)
  {
    BOOST_FAIL("");
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(unexpected_bulk_end_test)
{
  #ifdef NDEBUG
  #else
    //std::cout << "unexpected_bulk_end_test test\n";
  #endif

  try
  {
    const std::string testString{
      "cmd1\n"
      "cmd2\n"
      "cmd3\n"
      "(\n"
        "cmd4\n"
        "cmd5\n"
        "(\n"
          "cmd6\n"
          "(\n"
            "cmd7\n"
            "cmd8\n"
          ")\n"
          "cmd9\n"
        ")\n"
        "cmd10\n"
        "cmd11\n"
        "cmd12\n"
        "cmd13\n"
    };
    auto processorOutput{
      getProcessorOutput(testString, '(', ')', 4, DebugOutput::debug_off)
    };

    /* main application output */
    BOOST_CHECK(processorOutput[0][0] ==
                "bulk: cmd1, cmd2, cmd3");

    /* application error output */
    BOOST_CHECK(processorOutput[1].size() == 0);

    /* application metrics output */
    BOOST_CHECK(processorOutput[2].size() == 4);

    std::stringstream metricsStream{};
    for (const auto& tmpString : processorOutput[2])
    {
      metricsStream << tmpString << '\n';
    }

    checkMetrics(metricsStream, 18, 3, 1);
  }
  catch (const std::exception& ex)
  {
    BOOST_FAIL("");
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(incorrect_closing_test)
{
  #ifdef NDEBUG
  #else
    //std::cout << "incorrect_closing_test test\n";
  #endif

  try
  {
    const std::string testString{
      "cmd1\n"
      "cmd2\n"
      "cmd3\n"
      "(\n"
        "cmd4\n"
        "cmd5\n"
      ")\n"
      "cmd6\n"
      ")\n"
      "cmd7\n"
      "cmd8\n"
    };
    auto processorOutput{
      getProcessorOutput(testString, '(', ')', 4, DebugOutput::debug_off)
    };

    /* main application output */
    BOOST_CHECK(processorOutput[0][0] ==
                "bulk: cmd1, cmd2, cmd3");
    BOOST_CHECK(processorOutput[0][1] ==
                "bulk: cmd4, cmd5");
    BOOST_CHECK(processorOutput[0][2] ==
                "bulk: cmd6, cmd7, cmd8");

    /* application error output */
    BOOST_CHECK(processorOutput[1].size() == 0);

    /* application metrics output */
    BOOST_CHECK(processorOutput[2].size() == 4);

    std::stringstream metricsStream{};
    for (const auto& tmpString : processorOutput[2])
    {
      metricsStream << tmpString << '\n';
    }

    checkMetrics(metricsStream, 11, 8, 3);
  }
  catch (const std::exception& ex)
  {
    BOOST_FAIL("");
    std::cerr<< ex.what();
  }
}


BOOST_AUTO_TEST_CASE(commands_containing_delimiter_test)
{
  #ifdef NDEBUG
  #else
    //std::cout << "commands_containing_delimiter_test test\n";
  #endif

  try
  {
    const std::string testString
    {
      "cmd1\n"
      "{cmd2\n"
      "cmd3\n"
      "cmd4}\n"
      "cmd5"
    };
    auto processorOutput{
      getProcessorOutput(testString, '{', '}', 2, DebugOutput::debug_off)
    };

    /* main application output */
    BOOST_CHECK(processorOutput[0][0] == "bulk: cmd1, {cmd2");
    BOOST_CHECK(processorOutput[0][1] == "bulk: cmd3, cmd4}");
    BOOST_CHECK(processorOutput[0][2] == "bulk: cmd5");

    /* application error output */
    BOOST_CHECK(processorOutput[1].size() == 0);

    /* application metrics output */
    BOOST_CHECK(processorOutput[2].size() == 4);

    std::stringstream metricsStream{};
    for (const auto& tmpString : processorOutput[2])
    {
      metricsStream << tmpString << '\n';
    }

    checkMetrics(metricsStream, 5, 5, 3);
  }
  catch (const std::exception& ex)
  {
    BOOST_FAIL("");
    std::cerr << ex.what();
  }
}


BOOST_AUTO_TEST_CASE(logging)
{
  #ifdef NDEBUG
  #else
    //std::cout << "logging test\n";
  #endif

  try
  {
    /* wait 2 seconds to get separate log files for this test */
    std::this_thread::sleep_for(std::chrono::seconds{2});

    const std::string testString{
      "cmd1\n"
      "cmd2\n"
      "cmd3\n"
      "cmd4\n"
    };
    auto processorOutput{
      getProcessorOutput(testString, '(', ')', 4, DebugOutput::debug_off)
    };

    /* get current time */
    std::chrono::time_point<std::chrono::system_clock>
    bulkStartTime{std::chrono::system_clock::now()};
    /* convert bulk start time to integer ticks count */
    auto ticksCount{
      std::chrono::duration_cast<std::chrono::seconds>
      (
        bulkStartTime.time_since_epoch()
      ).count()
    };

    /* build log file name */
    --ticksCount;
    while (!std::ifstream{std::to_string(ticksCount).append("_11.log")}
           &&!std::ifstream{std::to_string(ticksCount).append("_12.log")})
    {
      ++ticksCount;
    }

    std::string logFileName;
    if (std::ifstream{std::to_string(ticksCount).append("_11.log")})
    {
      logFileName = std::to_string(ticksCount) + "_11.log";
    }
    else
    {
      logFileName = std::to_string(ticksCount) + "_12.log";
    };

    std::ifstream logFile(logFileName);

    std::string logString{};
    std::getline(logFile, logString);

    /* main application output */
    BOOST_CHECK(processorOutput[0][0] ==
                "bulk: cmd1, cmd2, cmd3, cmd4");

    /* application error output */
    BOOST_CHECK(processorOutput[1].size() == 0);

    /* application metrics output */
    BOOST_CHECK(processorOutput[2].size() == 4);

    std::stringstream metricsStream{};
    for (const auto& tmpString : processorOutput[2])
    {
      metricsStream << tmpString << '\n';
    }

    checkMetrics(metricsStream, 4, 4, 1);

    /* check log file state */
    BOOST_CHECK(logFile);

    /* check log file content */
    BOOST_CHECK(logString ==
                "bulk: cmd1, cmd2, cmd3, cmd4");
  }
  catch (const std::exception& ex)
  {
    BOOST_FAIL("");
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(unexpected_buffer_exhaustion)
{
  #ifdef NDEBUG
  #else
    //std::cout << "unexpected_buffer_exhaustion test\n";
  #endif

  try
  {
    std::string inputString{
      "a\nb\nc\nd\n"
    };

    std::stringstream inputStream{inputString};
    std::stringstream outputStream{};
    std::stringstream errorStream{};
    std::stringstream metricsStream{};

    {
      CommandProcessor<2> testProcessor {
        inputStream, outputStream, errorStream, metricsStream,
        3, '<', '>'
      };
      testProcessor.getBulkBuffer()->notify();
      testProcessor.run();
    }


    auto errorMessage{errorStream.str()};

    //std::cout << "Error message:" << errorMessage << std::endl;

    BOOST_CHECK(errorMessage.find("Abnormal termination")
                != std::string::npos);

    BOOST_CHECK(errorMessage.find("Buffer is empty!")
                != std::string::npos);
  }
  catch (const std::exception& ex)
  {
    BOOST_CHECK(false);
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_SUITE_END()




