#ifndef L1TMuonEndCap_EMTFGEMDetId_h
#define L1TMuonEndCap_EMTFGEMDetId_h

#include "DataFormats/DetId/interface/DetId.h"

#include <stdint.h>
#include <iosfwd>


class GEMDetId;
class ME0DetId;

class EMTFGEMDetId : public DetId {
public:
  explicit EMTFGEMDetId(const GEMDetId& id);
  explicit EMTFGEMDetId(const ME0DetId& id);

  /// Sort Operator based on the raw detector id
  bool operator < (const EMTFGEMDetId& r) const;

  /// The identifiers
  int region() const;
  int ring() const;  // NOTE: use ME0 --> ring 4 convention
  int station() const;  // NOTE: use ME0 --> station 1 convention
  int layer() const;
  int chamber() const;
  int roll() const;

  bool isME0() const { return isME0_; }

  GEMDetId getGEMDetId() const;

  ME0DetId getME0DetId() const;

private:
  bool isME0_;
};

std::ostream& operator<<( std::ostream& os, const EMTFGEMDetId& id );

#endif