#include <bitset>
#include <string>
#include <sstream>

namespace {

  template<typename INT>
  std::string to_hex(INT i) {
    std::stringstream s;
    s << "0x" << std::hex << i;
    return s.str();
  }

  template<typename INT>
  std::string to_binary(INT i, int n) {
    std::stringstream s;
    if (sizeof(i) <= 4) {
      std::bitset<32> b(i);
      s << "0b" << b.to_string().substr(32-n,32);
    } else if (sizeof(i) <= 8) {
      std::bitset<64> b(i);
      s << "0b" << b.to_string().substr(64-n,64);
    }
    return s.str();
  }

}  // namespace
