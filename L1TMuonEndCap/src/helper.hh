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
  constexpr size_t array_size(T(&)[N]) { return N; }

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

  // See http://stackoverflow.com/a/53878
  template <class STR=std::string>
  std::vector<STR> split_string(const std::string& s, char c = ' ', char d = ' ') {
    std::vector<STR> result;
    const char* str = s.c_str();
    do {
      const char* begin = str;
      while(*str != c && *str != d && *str)
        str++;
      result.emplace_back(begin, str);
    } while (0 != *str++);
    return result;
  }

  template <class T1, class T2>
  void flatten_container(const T1& input, T2& output) {
    typename T1::const_iterator it;
    for (it = input.begin(); it != input.end(); ++it) {
      output.insert(output.end(), it->begin(), it->end());
    }
  }

  template <class ForwardIt, class BinaryPredicate, class BinaryOp>
  ForwardIt adjacent_cluster(ForwardIt first, ForwardIt last, BinaryPredicate adjacent, BinaryOp cluster) {
    if (first == last) return last;

    ForwardIt result = first;
    while (++first != last) {
      if (!adjacent(*result, *first)) {
        *++result = std::move(*first);
      } else {
        cluster(*result, *first);
      }
    }
    return ++result;
  }

}  // namespace
