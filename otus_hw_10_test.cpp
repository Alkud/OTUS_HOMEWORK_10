// otus_hw_10_test.cpp in Otus homework#10 project

#define BOOST_TEST_MODULE OTUS_HW_10_TEST

#include <boost/test/unit_test.hpp>
#include "homework_10.h"
#include "command_processor_mt.h"


#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include <thread>

enum class DebugOutput
{
  debug_on,
  debug_off
};

/* Helper function */
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
  const CommandProcessor<2> testProcessor {
    inputStream, outputStream, errorStream, metricsStream,
    bulkSize, openDelimiter, closeDelimiter
  };

  testProcessor.run();

  std::this_thread::sleep_for(std::chrono::milliseconds{200});

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

BOOST_AUTO_TEST_SUITE(homework_10_test)

BOOST_AUTO_TEST_CASE(objects_creation_failure)
{
  std::mutex dummyMutex{};
  /* can't create input reader with null buffer pointer */
  BOOST_CHECK_THROW((InputReader{std::cin, dummyMutex, nullptr}), std::invalid_argument);
  /* can't create publisher with null buffer pointer */
  BOOST_CHECK_THROW((Publisher{nullptr, std::cout, dummyMutex}), std::invalid_argument);
  /* can't create logger with null buffer pointer */
  BOOST_CHECK_THROW((Logger<2>{nullptr, ""}), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(log_file_creation_failure)
{
  const auto bulkBuffer{std::make_shared<SmartBuffer<std::pair<size_t, std::string>>>()};
  std::stringstream outputStream{};
  std::stringstream metricsStream{};
  std::string badDirectoryName{"/non_existing_directory/"};

  {
  /* use bad directory name as constructor parameter */
  const auto badLogger{
    std::make_shared<Logger<2>>(
          bulkBuffer, badDirectoryName,
          outputStream, metricsStream
          )
  };
  /* connect buffer to logger */
  bulkBuffer->addNotificationListener(badLogger);
  /* putting some data to the buffer results in error message */
  bulkBuffer->putItem(std::make_pair<size_t, std::string>(1234, "bulk"));

  std::this_thread::sleep_for(std::chrono::milliseconds{100});

  /* imitating enexpected buffer exhaustion */
  bulkBuffer->clear();
  /* now buffer nitification should NOT throw an exception */
  BOOST_CHECK_NO_THROW((bulkBuffer->notify()));

  std::this_thread::sleep_for(std::chrono::milliseconds{100});
  }

  /* get error message string */
  std::string errorMessage{outputStream.str()};

  /* error message sholud contain expected file name */
  BOOST_CHECK(errorMessage.find("1234_1.log") != std::string::npos);
  /* error message sholud contain expected thread number */
  BOOST_CHECK(errorMessage.find("Logger stopped. Thread #1") != std::string::npos);

  /* get metrics string */
  std::string metrics{metricsStream.str()};
  /* metrics sholud contain expected text */
  BOOST_CHECK(std::string("file thread #0 - 0 bulk(s), 0 command(s)\n"
                          "file thread #1 - 0 bulk(s), 0 command(s)\n") == metrics);
}

BOOST_AUTO_TEST_CASE(trying_get_from_empty_buffer)
{
  SmartBuffer<std::pair<size_t, std::string>> emptyBuffer;
  /* emptyBuffer.getItem() should throw an exception */
  BOOST_CHECK_THROW((emptyBuffer.getItem()), std::out_of_range);
}



BOOST_AUTO_TEST_CASE(no_command_line_parameters)
{
  try
  {
    /* user input imitation: entering bulk size */
    std::stringstream inputStream{"-1\n"
                                  "2\n"};

    std::stringstream outputStream{};
    std::stringstream errorStream{};
    std::stringstream metricsStream{};
    /* comand line arguments */
    char* arg[]{"/home/user/bulk"};

    {
    homework(1, arg, inputStream, outputStream, errorStream, metricsStream);
    }

    /* application outpur should contain expected text*/
    BOOST_CHECK(outputStream.str() ==
                "\nPlease enter bulk size (must be greater than 0): "
                "\nPlease enter bulk size (must be greater than 0): ");

    /* metrics sholud contain expected text */
    BOOST_CHECK(metricsStream.str() ==
                "log thread - 0 bulk(s), 0 command(s)\n"
                "file thread #0 - 0 bulk(s), 0 command(s)\n"
                "file thread #1 - 0 bulk(s), 0 command(s)\n");

  }
  catch (const std::exception& ex)
  {
    BOOST_CHECK(false);
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(empty_input_test)
{
  try
  {
    auto processorOutput{
      getProcessorOutput(std::string{}, '{', '}', 3, DebugOutput::debug_off)
    };

    /* metrics sholud contain expected text */
    BOOST_CHECK(processorOutput[2][0] ==
                "log thread - 0 bulk(s), 0 command(s)");
    BOOST_CHECK(processorOutput[2][1] ==
                "file thread #0 - 0 bulk(s), 0 command(s)");
    BOOST_CHECK(processorOutput[2][2] ==
                "file thread #1 - 0 bulk(s), 0 command(s)");

  }
  catch (const std::exception& ex)
  {
    BOOST_CHECK(false);
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(empty_command_test)
{
  const std::string testString{"cmd1\n"
                               "\n"
                               "cmd2"};
  try
  {
    auto processorOutput{
      getProcessorOutput(testString, '{', '}', 3, DebugOutput::debug_off)
    };

    /* main application output */
    BOOST_CHECK(processorOutput[0][0] == "bulk: cmd1, , cmd2");

    /* application error output */
    BOOST_CHECK(processorOutput[1].size() == 0);

    /* application metrics output */
    BOOST_CHECK(processorOutput[2][0] ==
                "log thread - 1 bulk(s), 3 command(s)");
    BOOST_CHECK(processorOutput[2][1] ==
                "file thread #0 - 1 bulk(s), 3 command(s)");
    BOOST_CHECK(processorOutput[2][2] ==
                "file thread #1 - 0 bulk(s), 0 command(s)");
  }
  catch (const std::exception& ex)
  {
    BOOST_CHECK(false);
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(bulk_segmentation_test1)
{
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
    BOOST_CHECK(processorOutput[0][0] == "bulk: cmd1, cmd2, cmd3");
    BOOST_CHECK(processorOutput[0][1] == "bulk: cmd4");

    /* application error output */
    BOOST_CHECK(processorOutput[1].size() == 0);

    /* application metrics output */
    BOOST_CHECK(processorOutput[2][0] ==
                "log thread - 2 bulk(s), 4 command(s)");
    BOOST_CHECK(processorOutput[2][1] ==
                "file thread #0 - 1 bulk(s), 3 command(s)");
    BOOST_CHECK(processorOutput[2][2] ==
                "file thread #1 - 1 bulk(s), 1 command(s)");

  }
  catch (const std::exception& ex)
  {
    BOOST_CHECK(false);
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(bulk_segmentation_test2)
{
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
    BOOST_CHECK(processorOutput[0][0] == "bulk: cmd1");
    BOOST_CHECK(processorOutput[0][1] == "bulk: cmd2, cmd3");
    BOOST_CHECK(processorOutput[0][2] == "bulk: cmd4");

    /* application error output */
    BOOST_CHECK(processorOutput[1].size() == 0);

    /* application metrics output */
    BOOST_CHECK(processorOutput[2][0] ==
                "log thread - 3 bulk(s), 4 command(s)");
    BOOST_CHECK(processorOutput[2][1] ==
                "file thread #0 - 2 bulk(s), 2 command(s)");
    BOOST_CHECK(processorOutput[2][2] ==
                "file thread #1 - 1 bulk(s), 2 command(s)");
  }
  catch (const std::exception& ex)
  {
    BOOST_CHECK(false);
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(nested_bulks_test)
{
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
    BOOST_CHECK(processorOutput[0][0] ==
                "bulk: cmd1, cmd2, cmd3");
    BOOST_CHECK(processorOutput[0][1] ==
                "bulk: cmd4, cmd5, cmd6, cmd7, cmd8, cmd9");
    BOOST_CHECK(processorOutput[0][2] ==
                "bulk: cmd10, cmd11, cmd12, cmd13");

    /* application error output */
    BOOST_CHECK(processorOutput[1].size() == 0);

    /* application metrics output */
    BOOST_CHECK(processorOutput[2][0] ==
                "log thread - 3 bulk(s), 13 command(s)");
    BOOST_CHECK(processorOutput[2][1] ==
                "file thread #0 - 2 bulk(s), 7 command(s)");
    BOOST_CHECK(processorOutput[2][2] ==
                "file thread #1 - 1 bulk(s), 6 command(s)");
  }
  catch (const std::exception& ex)
  {
    BOOST_CHECK(false);
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(unexpected_bulk_end_test)
{
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
    BOOST_CHECK(processorOutput[2][0] ==
                "log thread - 1 bulk(s), 3 command(s)");
    BOOST_CHECK(processorOutput[2][1] ==
                "file thread #0 - 1 bulk(s), 3 command(s)");
    BOOST_CHECK(processorOutput[2][2] ==
                "file thread #1 - 0 bulk(s), 0 command(s)");
  }
  catch (const std::exception& ex)
  {
    BOOST_CHECK(false);
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(incorrect_closing_test)
{
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
    BOOST_CHECK(processorOutput[2][0] ==
                "log thread - 3 bulk(s), 8 command(s)");
    BOOST_CHECK(processorOutput[2][1] ==
                "file thread #0 - 2 bulk(s), 6 command(s)");
    BOOST_CHECK(processorOutput[2][2] ==
                "file thread #1 - 1 bulk(s), 2 command(s)");
  }
  catch (const std::exception& ex)
  {
    BOOST_CHECK(false);
    std::cerr<< ex.what();
  }
}


BOOST_AUTO_TEST_CASE(commands_containing_delimiter_test)
{
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
    BOOST_CHECK(processorOutput[2][0] ==
                "log thread - 3 bulk(s), 5 command(s)");
    BOOST_CHECK(processorOutput[2][1] ==
                "file thread #0 - 2 bulk(s), 3 command(s)");
    BOOST_CHECK(processorOutput[2][2] ==
                "file thread #1 - 1 bulk(s), 2 command(s)");
  }
  catch (const std::exception& ex)
  {
    BOOST_CHECK(false);
    std::cerr << ex.what();
  }
}


BOOST_AUTO_TEST_CASE(logging_test)
{
  try
  {
    /* wait a second to get separate log files for this test */
    std::this_thread::sleep_for(std::chrono::seconds{1});

    const std::string testString{
      "cmd1\n"
      "cmd2\n"
      "cmd3\n"
      "cmd4\n"
      "cmd5\n"
      "cmd6\n"
      "cmd7\n"
      "cmd8\n"
    };
    auto processorOutput{
      getProcessorOutput(testString, '(', ')', 8, DebugOutput::debug_off)
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
    while (!std::ifstream{std::to_string(ticksCount).append("_1.log")})
    {
      ++ticksCount;
    }

    std::string logFileName{
      std::to_string(ticksCount).append("_1.log")
    };

    std::ifstream logFile(logFileName);

    std::string logString{};
    std::getline(logFile, logString);

    /* main application output */
    BOOST_CHECK(processorOutput[0][0] ==
                "bulk: cmd1, cmd2, cmd3, cmd4, cmd5, cmd6, cmd7, cmd8");

    /* application error output */
    BOOST_CHECK(processorOutput[1].size() == 0);

    /* application metrics output */
    BOOST_CHECK(processorOutput[2][0] ==
                "log thread - 1 bulk(s), 8 command(s)");
    BOOST_CHECK(processorOutput[2][1] ==
                "file thread #0 - 1 bulk(s), 8 command(s)");
    BOOST_CHECK(processorOutput[2][2] ==
                "file thread #1 - 0 bulk(s), 0 command(s)");

    /* check log file state */
    BOOST_CHECK(logFile);

    /* check log file content */
    BOOST_CHECK(logString ==
                "bulk: cmd1, cmd2, cmd3, cmd4, cmd5, cmd6, cmd7, cmd8");
  }
  catch (const std::exception& ex)
  {
    BOOST_CHECK(false);
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_CASE(log_file_name_uniqueness_test)
{
  try
  {
    /* wait 2 seconds to get separate log file for this test */
    std::this_thread::sleep_for(std::chrono::seconds{2});

    const std::string testString{
      "cmd1\n"
      "cmd2\n"
      "cmd3\n"
      "cmd4\n"
      "cmd5\n"
      "cmd6\n"
      "cmd7\n"
      "cmd8\n"
    };
    auto processorOutput{
      getProcessorOutput(testString, '(', ')', 1, DebugOutput::debug_off)
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
    while (!std::ifstream{std::to_string(ticksCount).append("_1.log")})
    {
      ++ticksCount;
    }

    std::string logFileName{};
    std::ifstream logFile{};
    std::string logString{};

    for (size_t i{1}; i <= 8; ++i)
    {
      logFileName = std::to_string(ticksCount) +
                    + "_" + std::to_string(i) + ".log";

      logFile.open(logFileName);

      /* check log file state */
      BOOST_CHECK(logFile);

      std::getline(logFile, logString);

      /* check log file content */
      BOOST_CHECK(logString ==
                  "bulk: cmd" + std::to_string(i));

      logFile.close();
    }

    /* main application output */
    BOOST_CHECK(processorOutput[0].size() == 8);
    for (size_t i{1}; i <= 8; ++i)
    {
      BOOST_CHECK(processorOutput[0][i - 1] ==
                  "bulk: cmd" + std::to_string(i));
    }

    /* application error output */
    BOOST_CHECK(processorOutput[1].size() == 0);

    /* application metrics output */
    BOOST_CHECK(processorOutput[2][0] ==
                "log thread - 8 bulk(s), 8 command(s)");
    BOOST_CHECK(processorOutput[2][1] ==
                "file thread #0 - 4 bulk(s), 4 command(s)");
    BOOST_CHECK(processorOutput[2][2] ==
                "file thread #1 - 4 bulk(s), 4 command(s)");

  }
  catch (const std::exception& ex)
  {
    BOOST_CHECK(false);
    std::cerr << ex.what();
  }
}

BOOST_AUTO_TEST_SUITE_END()

