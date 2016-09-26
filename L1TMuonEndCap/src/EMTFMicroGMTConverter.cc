#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFMicroGMTConverter.hh"


EMTFMicroGMTConverter::EMTFMicroGMTConverter() {

}

EMTFMicroGMTConverter::~EMTFMicroGMTConverter() {

}

void EMTFMicroGMTConverter::convert(
    const EMTFTrackExtra& in_track,
    l1t::RegionalMuonCand& out_cand
) const {
  l1t::tftype tftype = (in_track.endcap == 1) ? l1t::tftype::emtf_pos : l1t::tftype::emtf_neg;
  int sector = in_track.sector - 1;

  out_cand.setHwPt(in_track.gmt_pt);
  out_cand.setHwPhi(in_track.gmt_phi);
  out_cand.setHwEta(in_track.gmt_eta);
  out_cand.setHwSign(in_track.gmt_charge);
  out_cand.setHwSignValid(1);
  out_cand.setHwQual(in_track.gmt_quality);
  // jl: FIXME this has to be adapted to the new schema of saving in_track addresses
  //out_cand.setTrackSubAddress(l1t::RegionalMuonCand::kME12, in_trackaddress&0xf);
  //out_cand.setTrackSubAddress(l1t::RegionalMuonCand::kME22, in_trackaddress>>4);
  out_cand.setTFIdentifiers(sector, tftype);
}

void EMTFMicroGMTConverter::convert_all(
    const EMTFTrackExtraCollection& in_tracks,
    l1t::RegionalMuonCandBxCollection& out_cands
) const {
  out_cands.clear();
  out_cands.setBXRange(-2,2);

  for (const auto& in_track : in_tracks) {
    l1t::RegionalMuonCand out_cand;

    convert(in_track, out_cand);
    out_cands.push_back(in_track.bx, out_cand);
  }
}
