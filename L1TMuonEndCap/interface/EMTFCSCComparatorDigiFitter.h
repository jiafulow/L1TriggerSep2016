// Based on TAMU comparator digi fitter
//     L1Trigger/CSCTriggerPrimitives/src/CSCComparatorDigiFitter.h
//     L1Trigger/CSCTriggerPrimitives/src/CSCComparatorDigiFitter.cc

#ifndef L1TMuonEndCap_EMTFCSCComparatorDigiFitter_h
#define L1TMuonEndCap_EMTFCSCComparatorDigiFitter_h

#include "DataFormats/CSCDigi/interface/CSCComparatorDigi.h"
#include "L1Trigger/CSCCommonTrigger/interface/CSCConstants.h"

#include <Math/Functions.h>
#include <Math/SVector.h>
#include <Math/SMatrix.h>

#include <stdexcept>
#include <vector>
#include <string>


class EMTFCSCComparatorDigiFitter {
public:
  // Constructor
  EMTFCSCComparatorDigiFitter();

  // Destructor
  ~EMTFCSCComparatorDigiFitter();

  typedef CSCComparatorDigi CompDigi;

  typedef std::pair<float, float> FitResult;

  // For doing least square fit
  typedef ROOT::Math::SMatrix<double,2> SMatrix22;
  typedef ROOT::Math::SMatrix<double,2,2,ROOT::Math::MatRepSym<double,2> > SMatrixSym2;
  typedef ROOT::Math::SVector<double,2> SVector2;

  // Fit comp digis
  // Return (bend, quality) = (deltaPhi a la ME0, chi2/ndof)
  FitResult fit(const std::vector<std::vector<CompDigi> >& compDigisAllLayers, const std::vector<int>& stagger, int keyStrip) const;

  // Least square fit with local x & y coordinates
  FitResult fitlsq(const std::vector<float>& x, const std::vector<float>& y) const;

  // For making combinations
  class StopIteration : public std::exception {
  public:
    explicit StopIteration(const std::string& what_arg) {}
  };

  // Make combinations
  std::vector<std::vector<int> > make_combinations(const std::vector<std::vector<CompDigi> >& compDigisAllLayers) const;

private:
  static const unsigned int min_nhits  = 3;
  static const unsigned int max_ncombs = 8;
  static constexpr float    max_dx     = 2.;
};

#endif  // L1TMuonEndCap_EMTFCSCComparatorDigiFitter_h
