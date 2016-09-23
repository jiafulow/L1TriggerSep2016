import unittest

from FWLiteAnalyzer import FWLiteAnalyzer


class MyAnalyzer(FWLiteAnalyzer):

  def __init__(self):
    inputFiles = ["Event1541093157_out.root"]
    # 3 1 2 1 1 1 12 5 11 5 1 65
    # 3 1 2 0 3 1 14 8 5 5 0 47
    # 3 1 2 0 4 1 14 8 10 5 0 65

    handles = {
      "hits": ("std::vector<L1TMuonEndCap::EMTFHitExtra>", "simEmtfDigisData"),
      "tracks": ("std::vector<L1TMuonEndCap::EMTFTrackExtra>", "simEmtfDigisData"),
    }
    super(MyAnalyzer, self).__init__(inputFiles, handles)

  def process(self, event):
    self.getHandles(event)
    hits = self.handles["hits"].product()
    tracks = self.handles["tracks"].product()

    hit = hits[0]
    assert(hit.phi_fp      == 2194)
    assert(hit.theta_fp    == 55)
    assert((1<<hit.ph_hit) == 1024)
    assert(hit.phzvl       == 1)

    hit = hits[1]
    assert(hit.phi_fp      == 2377)
    assert(hit.theta_fp    == 49)
    assert((1<<hit.ph_hit) == 131072)
    assert(hit.phzvl       == 3)

    hit = hits[2]
    assert(hit.phi_fp      == 2305)
    assert(hit.theta_fp    == 47)
    assert((1<<hit.ph_hit) == 32768)
    assert(hit.phzvl       == 1)

    track = tracks[0]
    assert(track.rank      == 7)
    assert(track.mode      == 3)
    assert(track.ptlut_address == 830547016)

    return

class TestEvent1541093157(unittest.TestCase):
  def setUp(self):
    self.analyzer = MyAnalyzer()

  def test_one(self):
    self.analyzer.analyze()


# ______________________________________________________________________________
if __name__ == "__main__":

  unittest.main()
