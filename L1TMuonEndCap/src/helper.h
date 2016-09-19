#include <bitset>
#include <string>
#include <sstream>

namespace {

  template<typename INT>
  std::string to_hex(INT i) {
    std::stringstream s;
    s << std::hex << i;
    return s.str();
  }

  template<typename INT>
  std::string to_binary(INT i, int n) {
    if (sizeof(i) <= 4) {
      std::bitset<32> b(i);
      return b.to_string().substr(32-n,32);
    } else if (sizeof(i) <= 8) {
      std::bitset<64> b(i);
      return b.to_string().substr(64-n,64);
    }
    return std::string();
  }

}  // namespace
