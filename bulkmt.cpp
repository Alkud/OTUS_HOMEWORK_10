// bulk.cpp in Otus homework#10 project

#include "./homework_10.h"
#include <stdexcept>
#include <iostream>

int main(int argc, char* argv[])
{
  try
  {
    return(homework(argc, argv, std::cin, std::cout));
  }
  catch(const std::exception& ex)
  {
    std::cerr << ex.what();
  }  
}
