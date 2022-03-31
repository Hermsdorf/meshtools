cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0057 NEW)
set(CMAKE_CXX_STANDARD 14)

project(fparser)

file(GLOB fparser_source_files_lib
         "*.hh"
         "extrasrc/*.hh"
         "extrasrc/*.cc"
         "*.cc"
    )

include_directories(extrasrc)
include_directories(mpfr)

add_library(fparser ${fparser_source_files_lib})


