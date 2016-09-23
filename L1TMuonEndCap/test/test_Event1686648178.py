import unittest

from FWLiteAnalyzer import FWLiteAnalyzer


class MyAnalyzer(FWLiteAnalyzer):

  def __init__(self):
    inputFiles = ["Event1686648178_out.root"]
    # 3 1 4 2 1 1 15 10 27 3 0 86
    # 3 1 4 0 2 1 15 10 84 3 0 134
    # 3 1 4 0 3 1 15 10 84 3 0 24
    # 3 1 3 2 5 1 14 8 33 1 0 109
    # 3 1 3 0 5 1 15 10 96 4 0 114
    # 3 1 3 0 5 1 15 10 6 9 0 71

    handles = {
      "hits": ("std::vector<L1TMuonEndCap::EMTFHitExtra>", "simEmtfDigisData"),
      "tracks": ("std::vector<L1TMuonEndCap::EMTFTrackExtra>", "simEmtfDigisData"),
    }
    super(MyAnalyzer, self).__init__(inputFiles, handles)

  def process(self, event):
    self.getHandles(event)
    hits = self.handles["hits"].product()
    tracks = self.handles["tracks"].product()

    hit = hits[9]
    assert(hit.phi_fp      == 4764)
    assert(hit.theta_fp    == 36)
    assert((1<<hit.ph_hit) == 65536)
    assert(hit.phzvl       == 1)

    hit = hits[10]
    assert(hit.phi_fp      == 4788)
    assert(hit.theta_fp    == 36)
    assert((1<<hit.ph_hit) == 68719476736)
    assert(hit.phzvl       == 1)

    hit = hits[11]
    assert(hit.phi_fp      == 4788)
    assert(hit.theta_fp    == 36)
    assert((1<<hit.ph_hit) == 68719476736)
    assert(hit.phzvl       == 1)

    hit = hits[12]
    assert(hit.phi_fp      == 1277)
    assert(hit.theta_fp    == 44)
    assert((1<<hit.ph_hit) == 262144)
    assert(hit.phzvl       == 2)

    hit = hits[13]
    assert(hit.phi_fp      == 1028)
    assert(hit.theta_fp    == 43)
    assert((1<<hit.ph_hit) == 1073741824)
    assert(hit.phzvl       == 3)

    hit = hits[14]
    assert(hit.phi_fp      == 1080)
    assert(hit.theta_fp    == 43)
    assert((1<<hit.ph_hit) == 16384)
    assert(hit.phzvl       == 1)

    track = tracks[2]
    assert(track.rank      == 106)
    assert(track.mode      == 14)
    assert(track.ptlut_address == 489746456)

    track = tracks[3]
    assert(track.rank      == 57)
    assert(track.mode      == 13)
    assert(track.ptlut_address == 759344630)

    return

class TestEvent1686648178(unittest.TestCase):
  def setUp(self):
    self.analyzer = MyAnalyzer()

  def test_one(self):
    self.analyzer.analyze()


# ______________________________________________________________________________
if __name__ == "__main__":

  unittest.main()
