# L1TriggerSep2016/L1TMuonEndCap

This is a CMSSW package that provides the overhauled emulator of the L1 Endcap Muon Track Finder (EMTF). Up to v0.2.X, it existed as a standalone package that can be run in parallel with the existing EMTF emulator. From v0.3.X, it is meant to replace the existing emulator and cannot be run in parallel anymore.

[![Build Status](https://travis-ci.org/jiafulow/L1TriggerSep2016.svg)](https://travis-ci.org/jiafulow/L1TriggerSep2016)
[![CMSSW version](https://img.shields.io/badge/cmssw-CMSSW__9__2__X-002963.svg)](https://github.com/cms-sw/cmssw)
[![Latest tag](https://img.shields.io/github/tag/jiafulow/L1TriggerSep2016.svg)](https://github.com/jiafulow/L1TriggerSep2016)

## Build

```shell
cd $CMSSW_BASE/src
git clone git@github.com:jiafulow/DataFormatsSep2016.git DataFormats
git clone git@github.com:jiafulow/L1TriggerSep2016.git L1Trigger
scram b -j 8
```

## Develop

Please do not work on the 'master' branch directly. Create a new branch for new features.

## Versions

- v0.4.X (since Jun 2017): Ported to CMSSW_9_2_0 release
- v0.3.X (since Mar 2017): Class renaming and variable renaming
- v0.2.X (since Mar 2017): Improved accuracy
- v0.1.X (since Nov 2016): First feature-complete version
- v0.0.X: Initial development

