# A simple FWLite-based python analyzer
# Based on https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookFWLitePython
# Some snippets of codes are stolen from PhysicsTools/Heppy

from ROOT import gROOT, gSystem, AutoLibraryLoader
from DataFormats.FWLite import Events, Handle


class FWLiteAnalyzer(object):

  def __init__(self, inputFiles=None, handles=None, firstEvent=None, maxEvents=None):
    gROOT.SetBatch()        # don't pop up canvases

    if inputFiles:
      if isinstance(inputFiles, basestring):
        self.inputFiles = [inputFiles]
      else:
        self.inputFiles = inputFiles
    else:
      self.inputFiles = []

    self.events = Events(self.inputFiles)

    self.handles = {}
    self.handle_labels = {}
    if handles:
      for k, v in handles.iteritems():
        self.handles[k] = Handle(v[0])
        self.handle_labels[k] = v[1]

    self.setup = {}

    if firstEvent:
      self.firstEvent = firstEvent
    else:
      self.firstEvent = 0

    if maxEvents:
      self.maxEvents = maxEvents
    else:
      self.maxEvents = 0x7FFFFFFFFFFFFFFF  # TChain::kBigNumber
    return

  def beginLoop(self, setup):
    return

  def endLoop(self, setup):
    return

  def getHandles(self, event):
    for k, v in self.handles.iteritems():
      label = self.handle_labels[k]
      event.getByLabel(label, v)
    return

  def process(self, event):
    self.getHandles(event)
    return

  def analyze(self):
    self.beginLoop(self.setup)
    for ievt, evt in enumerate(self.events):
      if (ievt % 1000) == 0:
        print "Processing event: %i" % ievt
      self.process(evt)
    self.endLoop(self.setup)
    return


# ______________________________________________________________________________
if __name__ == "__main__":

  print "Loading FW Lite"
  gSystem.Load("libFWCoreFWLite")
  gROOT.ProcessLine("FWLiteEnabler::enable();")

  #gSystem.Load("libDataFormatsFWLite.so")
  #gSystem.Load("libDataFormatsPatCandidates.so")
  #gSystem.Load("libDataFormatsSep2016L1TMuon.so")

  analyzer = MyAnalyzer()
  analyzer.analyze()
