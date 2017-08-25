#include "L1Trigger/L1TMuonEndCap/interface/EMTFGEMDetId.h"

#include "DataFormats/MuonDetId/interface/GEMDetId.h"
#include "DataFormats/MuonDetId/interface/ME0DetId.h"


EMTFGEMDetId::EMTFGEMDetId(const GEMDetId& id) :
    DetId(id),
    isME0_(false)
{

}

EMTFGEMDetId::EMTFGEMDetId(const ME0DetId& id) :
    DetId(id),
    isME0_(true)
{

}

/// Sort Operator based on the raw detector id
bool EMTFGEMDetId::operator < (const EMTFGEMDetId& r) const {
  if (!isME0() && !r.isME0()) {
    return getGEMDetId() < r.getGEMDetId(); // compare GEM with GEM
  } else if (r.isME0() && r.isME0()) {
    return getME0DetId() < r.getME0DetId(); // compare ME0 with ME0
  } else {
    return !r.isME0();                      // compare GEM with ME0
  }
}

/// The identifiers
int EMTFGEMDetId::region() const {
  if (!isME0())
    return getGEMDetId().region();
  else
    return getME0DetId().region();
}

int EMTFGEMDetId::ring() const {
  if (!isME0())
    return getGEMDetId().ring();
  else
    //return getME0DetId().ring();
    return 4;  // NOTE: use ME0 --> ring 4 convention
}

int EMTFGEMDetId::station() const {
  if (!isME0())
    return getGEMDetId().station();
  else
    //return getME0DetId().station();
    return 1;  // use ME0 --> station 1 convention
}

int EMTFGEMDetId::layer() const {
  if (!isME0())
    return getGEMDetId().layer();
  else
    return getME0DetId().layer();
}

int EMTFGEMDetId::chamber() const {
  if (!isME0())
    return getGEMDetId().chamber();
  else
    return getME0DetId().chamber();
}

int EMTFGEMDetId::roll() const {
  if (!isME0())
    return getGEMDetId().roll();
  else
    return getME0DetId().roll();
}

GEMDetId EMTFGEMDetId::getGEMDetId() const {
  return GEMDetId(rawId());
}

ME0DetId EMTFGEMDetId::getME0DetId() const {
  return ME0DetId(rawId());
}


std::ostream& operator<<( std::ostream& os, const EMTFGEMDetId& id ) {
  if (!id.isME0())
    os << GEMDetId(id.rawId());
  else
    os << ME0DetId(id.rawId());
  return os;
}
