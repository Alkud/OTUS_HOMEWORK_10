#CmakeList.txt for command_processor library in OTUS Homework #10

# set library name and content
add_library(command_processor
  broadcasters.h listeners.h
  async_worker.h
  thread_worker.h
  command_processor_mt.h
  input_processor.h input_processor.cpp
  input_reader.h input_reader.cpp
  logger_mt.h
  smart_buffer_mt.h
  publisher_mt.h publisher_mt.cpp
  weak_ptr_less.h
  thread_metrics.h
)
