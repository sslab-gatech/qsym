#ifndef __AFL_TRACE_MAP_H__
#define __AFL_TRACE_MAP_H__

#define FFL(_b) (0xffULL << ((_b) << 3))
#define FF(_b)  (0xff << ((_b) << 3))

#include <cstring>
#include <fstream>
#include <iostream>
#include <set>

#include "common.h"

namespace qsym {

class AflTraceMap {

private:
  std::string path_;
  ADDRINT prev_loc_;
  UINT8 *trace_map_;
  UINT8 *virgin_map_;
  UINT8 *context_map_;
  std::set<ADDRINT> visited_;

  void allocMap();
  void setDefault();
  void import(const std::string path);
  void commit();
  ADDRINT getIndex(ADDRINT h);
  bool isInterestingContext(ADDRINT h, ADDRINT bits);

public:
  AflTraceMap(const std::string path);
  bool isInterestingBranch(ADDRINT pc, bool taken);
};
} // namespace qsym
#endif // __AFL_TRACE_MAP_H__
