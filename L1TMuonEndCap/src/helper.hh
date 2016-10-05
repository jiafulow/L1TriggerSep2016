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

  template<typename T, size_t N>
  size_t array_size(T(&)[N]) { return N; }

  template<typename T, size_t N>
  std::string array_as_string(const T(&arr)[N]) {
    std::stringstream s;
    const char* sep = "";
    for (size_t i=0; i<N; ++i) {
      s << sep << arr[i];
      sep = " ";
    }
    return s.str();
  }

  // See http://stackoverflow.com/a/21510185
  namespace details {
    template <class T> struct _reversed {
      T& t; _reversed(T& _t): t(_t) {}
      decltype(t.rbegin()) begin() { return t.rbegin(); }
      decltype(t.rend()) end() { return t.rend(); }
    };
  }
  template <class T> details::_reversed<T> reversed(T& t) { return details::_reversed<T>(t); }

}  // namespace
