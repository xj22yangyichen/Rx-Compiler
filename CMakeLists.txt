cmake_minimum_required(VERSION 3.20)
project(rx_compiler LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Java REQUIRED COMPONENTS Runtime)
find_package(Threads REQUIRED)

set(ANTLR_JAR "$ENV{HOME}/.local/lib/antlr-4.13.2-complete.jar" CACHE FILEPATH "ANTLR 4.13.2 complete jar")
if(NOT EXISTS "${ANTLR_JAR}")
  message(FATAL_ERROR "Set ANTLR_JAR to the ANTLR 4.13.2 complete jar")
endif()
find_path(ANTLR_INCLUDE_DIR antlr4-runtime.h
  HINTS "$ENV{HOME}/.local/include/antlr4-runtime" PATH_SUFFIXES antlr4-runtime REQUIRED)
find_library(ANTLR_LIBRARY NAMES antlr4-runtime HINTS "$ENV{HOME}/.local/lib" REQUIRED)

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${generated_dir}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS grammar/Lexer.g4 grammar/Parser.g4)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/grammar/Lexer.g4" lexer_text)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/grammar/Parser.g4" parser_text)
string(REPLACE "lexer grammar Lexer;" "lexer grammar RxLexer;" lexer_text "${lexer_text}")
string(REPLACE "parser grammar Parser;" "parser grammar RxParser;" parser_text "${parser_text}")
string(REPLACE "tokenVocab=Lexer" "tokenVocab=RxLexer" parser_text "${parser_text}")
file(CONFIGURE OUTPUT "${generated_dir}/RxLexer.g4" CONTENT "${lexer_text}" @ONLY)
file(CONFIGURE OUTPUT "${generated_dir}/RxParser.g4" CONTENT "${parser_text}" @ONLY)
add_custom_command(
  OUTPUT "${generated_dir}/RxLexer.cpp" "${generated_dir}/RxLexer.h"
    "${generated_dir}/RxParser.cpp" "${generated_dir}/RxParser.h"
  COMMAND "${Java_JAVA_EXECUTABLE}" -jar "${ANTLR_JAR}"
    -Dlanguage=Cpp -no-listener RxLexer.g4 RxParser.g4
  WORKING_DIRECTORY "${generated_dir}"
  DEPENDS "${generated_dir}/RxLexer.g4" "${generated_dir}/RxParser.g4" "${ANTLR_JAR}"
  VERBATIM)

add_library(rx_parser STATIC "${generated_dir}/RxLexer.cpp" "${generated_dir}/RxParser.cpp")
target_include_directories(rx_parser SYSTEM PUBLIC "${ANTLR_INCLUDE_DIR}" "${generated_dir}")
target_link_libraries(rx_parser PUBLIC "${ANTLR_LIBRARY}" Threads::Threads)
add_library(rx_frontend STATIC
  src/ast/ast.cpp src/ast/ast-printer.cpp
  src/frontend/error-listener.cpp src/frontend/frontend.cpp
  src/frontend/ast-builder.cpp
  src/semantic/analyzer.cpp src/semantic/type.cpp
  src/semantic/check-stmt.cpp src/semantic/check-expr.cpp)
target_link_libraries(rx_frontend PUBLIC rx_parser)
target_compile_options(rx_frontend PRIVATE -Wall -Wextra -Wpedantic)
add_executable(rxc src/main.cpp)
target_link_libraries(rxc PRIVATE rx_frontend)
target_compile_options(rxc PRIVATE -Wall -Wextra -Wpedantic)
