import unittest

from FWLiteAnalyzer import FWLiteAnalyzer


class TestEvent1539957230(unittest.TestCase):
  def setUp(self):
    inputFiles = ["Event1539957230_out.root"]
    # 3 1 3 2 1 1 13 6 33 2 0 47
    # 3 1 3 0 2 1 15 10 77 2 0 141
    # 3 1 3 0 3 1 12 5 71 3 1 137
    # 3 1 3 0 4 1 13 7 74 3 1 104

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

    hit = hits[0]
    self.assertEqual(hit.phi_fp     , 3964)
    self.assertEqual(hit.theta_fp   , 37)
    self.assertEqual((1<<hit.ph_hit), 512)
    self.assertEqual(hit.phzvl      , 1)

    hit = hits[1]
    self.assertEqual(hit.phi_fp     , 3644)
    self.assertEqual(hit.theta_fp   , 33)
    self.assertEqual((1<<hit.ph_hit), 137438953472)
    self.assertEqual(hit.phzvl      , 1)

    hit = hits[2]
    self.assertEqual(hit.phi_fp     , 3879)
    self.assertEqual(hit.theta_fp   , 30)
    self.assertEqual((1<<hit.ph_hit), 256)
    self.assertEqual(hit.phzvl      , 1)

    hit = hits[3]
    self.assertEqual(hit.phi_fp     , 4146)
    self.assertEqual(hit.theta_fp   , 27)
    self.assertEqual((1<<hit.ph_hit), 65536)
    self.assertEqual(hit.phzvl      , 1)

  def test_tracks(self):
    tracks = self.analyzer.handles["tracks"].product()

    track = tracks[0]
    self.assertEqual(track.rank         , 40)
    self.assertEqual(track.mode         , 12)
    self.assertEqual(track.ptlut_address, 218120512)

    track = tracks[1]
    self.assertEqual(track.rank         , 23)
    self.assertEqual(track.mode         , 3)
    self.assertEqual(track.ptlut_address, 820437771)


# ______________________________________________________________________________
if __name__ == "__main__":

  unittest.main()
