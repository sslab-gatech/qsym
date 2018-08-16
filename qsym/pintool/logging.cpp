#include <cstdio>
#include <iostream>

#include "pin.H"
#include "logging.h"
#include "solver.h"

namespace qsym {

KNOB<bool> g_opt_debug(KNOB_MODE_WRITEONCE, "pintool",
    "d", "0", "turn on debug mode");

void log(const char* tag, const std::string &msg) {
  std::string tagged_msg = std::string("[") + tag + "] " + msg;
  std::cerr << tagged_msg;

  LOG(tagged_msg);
}

bool isDebugMode() {
  return g_opt_debug.Value();
}

void LOG_FATAL(const std::string &msg) {
  std::string fatal_msg;
  if (g_solver)
    fatal_msg = "[" + hexstr(g_solver->last_pc()) + "]: ";
  fatal_msg += msg;
  log("FATAL", fatal_msg);

  // since abort() sometimes not working
  // trigger crash to terminate pin
  // CRASH();
  exit(-1);
}

void LOG_INFO(const std::string &msg) {
  log("INFO", msg);
}

void LOG_STAT(const std::string &msg) {
  log("STAT", msg);
}
void LOG_WARN(const std::string &msg) {
  log("WARN", msg);
}

} // namespace qsym
