import unittest

from FWLiteAnalyzer import FWLiteAnalyzer


class TestEvent1687278667(unittest.TestCase):
  def setUp(self):
    inputFiles = ["Event1687278667_out.root"]
    # 3 1 2 2 1 1 12 5 22 1 1 91
    # 3 1 2 2 1 1 12 5 6 2 1 215
    # 3 1 2 0 2 1 15 10 62 3 0 41
    # 3 1 2 0 3 1 15 10 55 3 0 122
    # 3 1 2 0 4 1 14 8 59 3 0 139

    handles = {
      "hits": ("std::vector<L1TMuonEndCap::EMTFHitExtra>", "simEmtfDigisData"),
      "tracks": ("std::vector<L1TMuonEndCap::EMTFTrackExtra>", "simEmtfDigisData"),
    }

    self.analyzer = FWLiteAnalyzer(inputFiles, handles)
    self.analyzer.beginLoop()
    self.event = next(self.analyzer.processLoop())

  def tearDown(self):
    self.analyzer.endLoop()
    self.analyzer = None
    self.event = None

  def test_hits(self):
    hits = self.analyzer.handles["hits"].product()

    hit = hits[4]
    self.assertEqual(hit.phi_fp     , 3588)
    self.assertEqual(hit.theta_fp   , 32)
    self.assertEqual((1<<hit.ph_hit), 65536)
    self.assertEqual(hit.phzvl      , 1)

    hit = hits[5]
    self.assertEqual(hit.phi_fp     , 4314)
    self.assertEqual(hit.theta_fp   , 13)
    self.assertEqual((1<<hit.ph_hit), 1048576)
    self.assertEqual(hit.phzvl      , 1)

    hit = hits[6]
    self.assertEqual(hit.phi_fp     , 4043)
    self.assertEqual(hit.theta_fp   , 27)
    self.assertEqual((1<<hit.ph_hit), 8192)
    self.assertEqual(hit.phzvl      , 1)

    hit = hits[7]
    self.assertEqual(hit.phi_fp     , 4004)
    self.assertEqual(hit.theta_fp   , 25)
    self.assertEqual((1<<hit.ph_hit), 4096)
    self.assertEqual(hit.phzvl      , 1)

    hit = hits[8]
    self.assertEqual(hit.phi_fp     , 3869)
    self.assertEqual(hit.theta_fp   , 24)
    self.assertEqual((1<<hit.ph_hit), 256)
    self.assertEqual(hit.phzvl      , 1)

  def test_tracks(self):
    tracks = self.analyzer.handles["tracks"].product()

    track = tracks[0]
    self.assertEqual(track.rank         , 14)
    self.assertEqual(track.mode         , 6)
    self.assertEqual(track.ptlut_address, 416285735)


# ______________________________________________________________________________
if __name__ == "__main__":

  unittest.main()
