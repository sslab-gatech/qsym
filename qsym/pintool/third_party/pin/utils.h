#include <string>
#include <sstream>

#include "pin.H"
extern "C" {
#include "xed-interface.h"
}

namespace qsym {
inline std::string disassemble(ADDRINT pc) {
#if defined(TARGET_IA32E)
  static const xed_state_t dstate = {XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b};
#else
  static const xed_state_t dstate = { XED_MACHINE_MODE_LEGACY_32, XED_ADDRESS_WIDTH_32b};
#endif
  xed_decoded_inst_t xedd;
  xed_decoded_inst_zero_set_mode(&xedd,&dstate);

  //Pass in the proper length: 15 is the max. But if you do not want to
  //cross pages, you can pass less than 15 bytes, of course, the
  //instruction might not decode if not enough bytes are provided.
  const unsigned int max_inst_len = 15;

  xed_error_enum_t xed_code = xed_decode(&xedd, reinterpret_cast<UINT8*>(pc), max_inst_len);
  BOOL xed_ok = (xed_code == XED_ERROR_NONE);
  if (xed_ok) {
    std::stringstream ss;
    ss << hex << std::setw(8) << pc << " ";
    char buf[2048];

    // set the runtime adddress for disassembly
    xed_uint64_t runtime_address = static_cast<xed_uint64_t>(pc);

    xed_format_context(XED_SYNTAX_INTEL, &xedd,
        buf, 2048, runtime_address, 0, 0);
    ss << buf;
    return ss.str();
  }
  else {
    return "";
  }
}

} // namespace qsym
