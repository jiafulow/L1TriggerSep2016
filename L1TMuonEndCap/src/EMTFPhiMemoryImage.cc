#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPhiMemoryImage.hh"

#include <stdexcept>

#define UINT64_BITS 64


EMTFPhiMemoryImage::EMTFPhiMemoryImage() {
  reset();
}

EMTFPhiMemoryImage::~EMTFPhiMemoryImage() {

}

EMTFPhiMemoryImage::EMTFPhiMemoryImage(const EMTFPhiMemoryImage& other) {
  for (unsigned int i = 0; i < _layers; ++i) {
    for (unsigned int j = 0; j < _units; ++j) {
      _buffer[i][j] = other._buffer[i][j];
    }
  }
}

EMTFPhiMemoryImage& EMTFPhiMemoryImage::operator=(const EMTFPhiMemoryImage& other) {
  if (this == &other)
    return *this;

  for (unsigned int i = 0; i < _layers; ++i) {
    for (unsigned int j = 0; j < _units; ++j) {
      _buffer[i][j] = other._buffer[i][j];
    }
  }
  return *this;
}

void EMTFPhiMemoryImage::reset() {
  for (unsigned int i = 0; i < _layers; ++i) {
    for (unsigned int j = 0; j < _units; ++j) {
      _buffer[i][j] = 0;
    }
  }
}

void EMTFPhiMemoryImage::set_bit(unsigned int layer, unsigned int bit) {
  check_input(layer, bit);

  unsigned int unit = bit / UINT64_BITS;
  unsigned int mask = (1u << (bit % UINT64_BITS));
  _buffer[layer][unit] |= mask;
}

void EMTFPhiMemoryImage::clear_bit(unsigned int layer, unsigned int bit) {
  check_input(layer, bit);

  unsigned int unit = bit / UINT64_BITS;
  unsigned int mask = (1u << (bit % UINT64_BITS));
  _buffer[layer][unit] &= ~mask;
}

bool EMTFPhiMemoryImage::test_bit(unsigned int layer, unsigned int bit) const {
  check_input(layer, bit);

  unsigned int unit = bit / UINT64_BITS;
  unsigned int mask = (1u << (bit % UINT64_BITS));
  return _buffer[layer][unit] & mask;
}

void EMTFPhiMemoryImage::set_word(unsigned int layer, unsigned int unit, value_type value) {
  check_input(layer, unit*UINT64_BITS);

  _buffer[layer][unit] = value;
}

EMTFPhiMemoryImage::value_type EMTFPhiMemoryImage::get_word(unsigned int layer, unsigned int unit) const {
  check_input(layer, unit*UINT64_BITS);

  return _buffer[layer][unit];
}

void EMTFPhiMemoryImage::check_input(unsigned int layer, unsigned int bit) const {
  if (layer >= _layers) {
    char what[128];
    snprintf(what, sizeof(what), "layer (which is %u) >= _layers (which is %u)", layer, _layers);
    throw std::out_of_range(what);
  }

  unsigned int unit = bit / UINT64_BITS;
  if (unit >= _units) {
    char what[128];
    snprintf(what, sizeof(what), "unit (which is %u) >= _units (which is %u)", unit, _units);
    throw std::out_of_range(what);
  }
}

// See https://en.wikipedia.org/wiki/Circular_shift#Implementing_circular_shifts
// return (val << len) | ((unsigned) val >> (-len & (sizeof(INT) * CHAR_BIT - 1)));
void EMTFPhiMemoryImage::rotl(const value_type n) {
  if (n >= _units*UINT64_BITS)
    return;

  value_type tmp[_layers][_units];
  std::copy(&(_buffer[0][0]), &(_buffer[0][0]) + (_layers*_units), &(tmp[0][0]));

  const value_type mask = UINT64_BITS - 1;
  const value_type n1 = n % UINT64_BITS;
  const value_type n2 = _units - (n / UINT64_BITS);
  const value_type n3 = (n1 == 0) ? n2+1 : n2;

  unsigned int i = 0, j = 0, j_curr = 0, j_next = 0;
  for (i = 0; i < _layers; ++i) {
    for (j = 0; j < _units; ++j) {
      // if n2 == 0:
      //   j_curr = 0, 1, 2
      //   j_next = 2, 0, 1
      // if n2 == 1:
      //   j_curr = 2, 0, 1
      //   j_next = 1, 2, 0
      j_curr = (n2+j) % _units;
      j_next = (n3+j+_units-1) % _units;
      _buffer[i][j] = (tmp[i][j_curr] << n1) | (tmp[i][j_next] >> (-n1 & mask));
    }
  }
}

void EMTFPhiMemoryImage::rotr(const value_type n) {
  if (n >= _units*UINT64_BITS)
    return;

  value_type tmp[_layers][_units];
  std::copy(&(_buffer[0][0]), &(_buffer[0][0]) + (_layers*_units), &(tmp[0][0]));

  const value_type mask = UINT64_BITS - 1;
  const value_type n1 = n % UINT64_BITS;
  const value_type n2 = n / UINT64_BITS;
  const value_type n3 = (n1 == 0) ? n2+_units-1 : n2;

  unsigned int i = 0, j = 0, j_curr = 0, j_next = 0;
  for (i = 0; i < _layers; ++i) {
    for (j = 0; j < _units; ++j) {
      // if n2 == 0:
      //   j_curr = 0, 1, 2
      //   j_next = 1, 2, 0
      // if n2 == 1:
      //   j_curr = 2, 0, 1
      //   j_next = 0, 1, 2
      j_curr = (n2+j)% _units;
      j_next = (n3+j+1) % _units;
      _buffer[i][j] = (tmp[i][j_curr] >> n1) | (tmp[i][j_next] << (-n1 & mask));
    }
  }
}

unsigned int EMTFPhiMemoryImage::op_and(const EMTFPhiMemoryImage& other) const {
  // Unroll
  bool b_st1 = (_buffer[0][0] & other._buffer[0][0]) ||
               (_buffer[0][1] & other._buffer[0][1]) ||
               (_buffer[0][2] & other._buffer[0][2]);
  bool b_st2 = (_buffer[1][0] & other._buffer[1][0]) ||
               (_buffer[1][1] & other._buffer[1][1]) ||
               (_buffer[1][2] & other._buffer[1][2]);
  bool b_st3 = (_buffer[2][0] & other._buffer[2][0]) ||
               (_buffer[2][1] & other._buffer[2][1]) ||
               (_buffer[2][2] & other._buffer[2][2]);
  bool b_st4 = (_buffer[3][0] & other._buffer[3][0]) ||
               (_buffer[3][1] & other._buffer[3][1]) ||
               (_buffer[3][2] & other._buffer[3][2]);

  //   bit 0: st3 or st4 hit
  //   bit 1: st2 hit
  //   bit 2: st1 hit
  unsigned int layer = (b_st1 << 2) | (b_st2 << 1) | (b_st3 << 0) | (b_st4 << 0);
  return layer;
}
