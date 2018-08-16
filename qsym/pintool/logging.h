#ifndef QSYM_LOGGING_H_
#define QSYM_LOGGING_H_

#include <stdio.h>
#include <stdlib.h>
#include <cstdarg>
#include <string>
#include "pin.H"
#include "compiler.h"

namespace qsym {

#define UNREACHABLE() \
  LOG_FATAL(std::string(__FILE__) + ":" \
            + std::to_string(__LINE__) + ": Unreachable");

#define QSYM_ASSERT(x) \
  if (!(x)) \
    LOG_FATAL( \
        std::string(__FILE__) + ":" \
        + std::to_string(__LINE__) + ": " #x);

void log(const char* tag, const std::string &msg);

#define LOG_DEBUG(msg) \
  do { \
    if (isDebugMode()) \
      log("DEBUG", msg); \
  } while(0);

bool isDebugMode();
void LOG_FATAL(const std::string &msg);
void LOG_INFO(const std::string &msg);
void LOG_STAT(const std::string &msg);
void LOG_WARN(const std::string &msg);

} // namespace qsym

#endif // QSYM_LOGGING_H_
