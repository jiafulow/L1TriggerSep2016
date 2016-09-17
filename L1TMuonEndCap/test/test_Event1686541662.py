import unittest

from FWLiteAnalyzer import FWLiteAnalyzer


class MyAnalyzer(FWLiteAnalyzer):

  def __init__(self):
    inputFiles = ["Event1686541662_out.root"]
    # 1 2 4 1 1 1 13 7 15 3 1 91
    # 3 2 4 1 1 1 12 4 4 1 0 214
    # 3 2 4 1 1 1 14 8 4 1 0 202
    # 3 2 4 0 2 1 15 10 13 1 0 123
    # 3 2 4 0 3 1 13 7 20 1 1 15
    # 3 2 3 2 5 1 13 6 0 1 0 136

    handles = {
      "emtf": ("std::vector<L1TMuonEndCap::EMTFHitExtra>", "simEmtfDigisData"),
    }
    super(MyAnalyzer, self).__init__(inputFiles, handles)

  def process(self, event):
    self.getHandles(event)
    emtf = self.handles["emtf"].product()

    hit = emtf[4]
    assert(hit.phi_fp      == 2717)
    assert(hit.theta_fp    == 20)
    assert((1<<hit.ph_hit) == 512)
    assert(hit.phzvl       == 1)

    hit = emtf[5]
    assert(hit.phi_fp      == 1403)
    assert(hit.theta_fp    == 9)
    assert((1<<hit.ph_hit) == 32)
    assert(hit.phzvl       == 1)

    hit = emtf[6]
    assert(hit.phi_fp      == 1482)
    assert(hit.theta_fp    == 9)
    assert((1<<hit.ph_hit) == 128)
    assert(hit.phzvl       == 1)

    hit = emtf[7]
    assert(hit.phi_fp      == 1604)
    assert(hit.theta_fp    == 10)
    assert((1<<hit.ph_hit) == 4096)
    assert(hit.phzvl       == 1)

    hit = emtf[8]
    assert(hit.phi_fp      == 1446)
    assert(hit.theta_fp    == 13)
    assert((1<<hit.ph_hit) == 64)
    assert(hit.phzvl       == 1)

    hit = emtf[9]
    assert(hit.phi_fp      == 1322)
    assert(hit.theta_fp    == 7)
    assert((1<<hit.ph_hit) == 2097152)
    assert(hit.phzvl       == 1)

    return

class TestEvent1686541662(unittest.TestCase):
  def setUp(self):
    self.analyzer = MyAnalyzer()

  def test_one(self):
    self.analyzer.analyze()


# ______________________________________________________________________________
if __name__ == "__main__":

  unittest.main()
