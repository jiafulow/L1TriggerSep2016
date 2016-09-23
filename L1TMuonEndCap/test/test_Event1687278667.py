import unittest

from FWLiteAnalyzer import FWLiteAnalyzer


class MyAnalyzer(FWLiteAnalyzer):

  def __init__(self):
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
    super(MyAnalyzer, self).__init__(inputFiles, handles)

  def process(self, event):
    self.getHandles(event)
    hits = self.handles["hits"].product()
    tracks = self.handles["tracks"].product()

    hit = hits[4]
    assert(hit.phi_fp      == 3588)
    assert(hit.theta_fp    == 32)
    assert((1<<hit.ph_hit) == 65536)
    assert(hit.phzvl       == 1)

    hit = hits[5]
    assert(hit.phi_fp      == 4314)
    assert(hit.theta_fp    == 13)
    assert((1<<hit.ph_hit) == 1048576)
    assert(hit.phzvl       == 1)

    hit = hits[6]
    assert(hit.phi_fp      == 4043)
    assert(hit.theta_fp    == 27)
    assert((1<<hit.ph_hit) == 8192)
    assert(hit.phzvl       == 1)

    hit = hits[7]
    assert(hit.phi_fp      == 4004)
    assert(hit.theta_fp    == 25)
    assert((1<<hit.ph_hit) == 4096)
    assert(hit.phzvl       == 1)

    hit = hits[8]
    assert(hit.phi_fp      == 3869)
    assert(hit.theta_fp    == 24)
    assert((1<<hit.ph_hit) == 256)
    assert(hit.phzvl       == 1)

    track = tracks[0]
    assert(track.rank      == 14)
    assert(track.mode      == 6)
    assert(track.ptlut_address == 416285735)

    return

class TestEvent1687278667(unittest.TestCase):
  def setUp(self):
    self.analyzer = MyAnalyzer()

  def test_one(self):
    self.analyzer.analyze()


# ______________________________________________________________________________
if __name__ == "__main__":

  unittest.main()
