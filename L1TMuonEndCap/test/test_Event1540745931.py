import unittest

from FWLiteAnalyzer import FWLiteAnalyzer


class MyAnalyzer(FWLiteAnalyzer):

  def __init__(self):
    inputFiles = ["Event1540745931_out.root"]
    # 3 1 1 2 1 1 15 10 53 5 0 124
    # 3 1 1 0 2 1 15 10 24 8 0 130
    # 3 1 1 0 2 1 14 8 24 8 0 119
    # 3 1 1 0 3 1 15 10 36 8 0 26
    # 3 1 1 0 4 1 14 9 46 8 1 25

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
    assert(hit.phi_fp      == 4228)
    assert(hit.theta_fp    == 77)
    assert((1<<hit.ph_hit) == 131072)
    assert(hit.phzvl       == 1)

    hit = hits[1]
    assert(hit.phi_fp      == 4252)
    assert(hit.theta_fp    == 76)
    assert((1<<hit.ph_hit) == 262144)
    assert(hit.phzvl       == 1)

    hit = hits[2]
    assert(hit.phi_fp      == 4207)
    assert(hit.theta_fp    == 76)
    assert((1<<hit.ph_hit) == 65536)
    assert(hit.phzvl       == 1)

    hit = hits[3]
    assert(hit.phi_fp      == 4260)
    assert(hit.theta_fp    == 78)
    assert((1<<hit.ph_hit) == 524288)
    assert(hit.phzvl       == 2)

    hit = hits[4]
    assert(hit.phi_fp      == 4263)
    assert(hit.theta_fp    == 78)
    assert((1<<hit.ph_hit) == 1048576)
    assert(hit.phzvl       == 2)

    track = tracks[0]
    assert(track.rank      == 63)
    assert(track.mode      == 15)
    assert(track.ptlut_address == 1047278616)

    return

class TestEvent1540745931(unittest.TestCase):
  def setUp(self):
    self.analyzer = MyAnalyzer()

  def test_one(self):
    self.analyzer.analyze()


# ______________________________________________________________________________
if __name__ == "__main__":

  unittest.main()
