#ifndef L1TMuonEndCap_EMTFPhiMemoryImage_hh
#define L1TMuonEndCap_EMTFPhiMemoryImage_hh

#include <cstdint>
#include <iosfwd>

// Originally written by Ivan Furic and Matt Carver (Univ of Florida)


class EMTFPhiMemoryImage {
public:
  typedef uint64_t value_type;

  EMTFPhiMemoryImage();
  ~EMTFPhiMemoryImage();

  // Copy constructor and copy assignment needed to manage resources
  EMTFPhiMemoryImage(const EMTFPhiMemoryImage& other);
  EMTFPhiMemoryImage& operator=(const EMTFPhiMemoryImage& other);

  void reset();

  void set_bit(unsigned int layer, unsigned int bit);

  void clear_bit(unsigned int layer, unsigned int bit);

  bool test_bit(unsigned int layer, unsigned int bit) const;

  void set_word(unsigned int layer, unsigned int unit, value_type value);

  value_type get_word(unsigned int layer, unsigned int unit) const;

  // Left rotation by n bits
  void rotl(const value_type n);

  // Right rotation by n bits
  void rotr(const value_type n);

  // Kind of like AND operator
  // It returns a layer code which encodes
  //   bit 0: st3 or st4 hit
  //   bit 1: st2 hit
  //   bit 2: st1 hit
  unsigned int op_and(const EMTFPhiMemoryImage& other) const;

  void print(std::ostream& out) const;

private:
  void check_input(unsigned int layer, unsigned int bit) const;

  // Num of layers
  //   [0,1,2,3] --> [st1,st2,st3,st4]
  static const unsigned int _layers = 4;

  // Num of value_type allocated per layer
  //   3 * 64 bits = 192 bits
  static const unsigned int _units = 3;

  // Hits in non-key stations
  value_type _buffer[_layers][_units];
};

// _____________________________________________________________________________
// Output streams
std::ostream& operator<<(std::ostream& o, const EMTFPhiMemoryImage& p);

#endif