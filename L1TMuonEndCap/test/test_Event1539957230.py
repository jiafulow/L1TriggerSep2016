import unittest

from FWLiteAnalyzer import FWLiteAnalyzer


class MyAnalyzer(FWLiteAnalyzer):

  def __init__(self):
    inputFiles = ["Event1539957230_out.root"]
    # 3 1 3 2 1 1 13 6 33 2 0 47
    # 3 1 3 0 2 1 15 10 77 2 0 141
    # 3 1 3 0 3 1 12 5 71 3 1 137
    # 3 1 3 0 4 1 13 7 74 3 1 104

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
    assert(hit.phi_fp      == 3964)
    assert(hit.theta_fp    == 37)
    assert((1<<hit.ph_hit) == 512)
    assert(hit.phzvl       == 1)

    hit = hits[1]
    assert(hit.phi_fp      == 3644)
    assert(hit.theta_fp    == 33)
    assert((1<<hit.ph_hit) == 137438953472)
    assert(hit.phzvl       == 1)

    hit = hits[2]
    assert(hit.phi_fp      == 3879)
    assert(hit.theta_fp    == 30)
    assert((1<<hit.ph_hit) == 256)
    assert(hit.phzvl       == 1)

    hit = hits[3]
    assert(hit.phi_fp      == 4146)
    assert(hit.theta_fp    == 27)
    assert((1<<hit.ph_hit) == 65536)
    assert(hit.phzvl       == 1)

    track = tracks[0]
    assert(track.rank      == 40)
    assert(track.mode      == 12)
    assert(track.ptlut_address == 218120512)

    track = tracks[1]
    assert(track.rank      == 23)
    assert(track.mode      == 3)
    assert(track.ptlut_address == 820437771)

    return

class TestEvent1539957230(unittest.TestCase):
  def setUp(self):
    self.analyzer = MyAnalyzer()

  def test_one(self):
    self.analyzer.analyze()


# ______________________________________________________________________________
if __name__ == "__main__":

  unittest.main()
