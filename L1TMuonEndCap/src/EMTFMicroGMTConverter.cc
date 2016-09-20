#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFMicroGMTConverter.hh"


EMTFMicroGMTConverter::EMTFMicroGMTConverter() {

}

EMTFMicroGMTConverter::~EMTFMicroGMTConverter() {

}

void EMTFMicroGMTConverter::convert(
    const EMTFTrackExtra& in_track,
    l1t::RegionalMuonCand& out_cand
) {
  //FIXME: IMPLEMENT THIS
}

void EMTFMicroGMTConverter::convert_many(
    const EMTFTrackExtraCollection& in_tracks,
    l1t::RegionalMuonCandBxCollection& out_cands
) {
  out_cands.clear();
  out_cands.setBXRange(-2,2);

  for (const auto& in_track: in_tracks) {
    l1t::RegionalMuonCand out_cand;

    convert(in_track, out_cand);
    out_cands.push_back(in_track.bx, out_cand);
  }
}
