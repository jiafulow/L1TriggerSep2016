import unittest

from FWLiteAnalyzer import FWLiteAnalyzer


class MyAnalyzer(FWLiteAnalyzer):

  def __init__(self):
    inputFiles = ["Event1541093157_out.root"]
    # 3 1 2 1 1 1 12 5 11 5 1 65
    # 3 1 2 0 3 1 14 8 5 5 0 47
    # 3 1 2 0 4 1 14 8 10 5 0 65

    handles = {
      "emtf": ("std::vector<L1TMuonEndCap::EMTFHitExtra>", "simEmtfDigisData"),
    }
    super(MyAnalyzer, self).__init__(inputFiles, handles)

  def process(self, event):
    self.getHandles(event)
    emtf = self.handles["emtf"].product()

    hit = emtf[0]
    assert(hit.phi_fp      == 2194)
    assert(hit.theta_fp    == 55)
    assert((1<<hit.ph_hit) == 1024)
    assert(hit.phzvl       == 1)

    hit = emtf[1]
    assert(hit.phi_fp      == 2377)
    assert(hit.theta_fp    == 49)
    assert((1<<hit.ph_hit) == 131072)
    assert(hit.phzvl       == 3)

    hit = emtf[2]
    assert(hit.phi_fp      == 2305)
    assert(hit.theta_fp    == 47)
    assert((1<<hit.ph_hit) == 32768)
    assert(hit.phzvl       == 1)

    return

class TestEvent1541093157(unittest.TestCase):
  def setUp(self):
    self.analyzer = MyAnalyzer()

  def test_one(self):
    self.analyzer.analyze()


# ______________________________________________________________________________
if __name__ == "__main__":

  unittest.main()
