# L1TriggerSep2016/L1TMuonEndCap

This is a CMSSW package that provides the overhauled emulator of the L1 Endcap Muon Track Finder (EMTF). Currently it exists as a standalone package that can be run in parallel with the existing EMTF emulator.

[![Build Status](https://travis-ci.org/jiafulow/L1TriggerSep2016.svg)](https://travis-ci.org/jiafulow/L1TriggerSep2016)
[![CMSSW version](https://img.shields.io/badge/cmssw-CMSSW__8__0__X-002963.svg)](https://github.com/cms-sw/cmssw)
[![Latest tag](https://img.shields.io/github/tag/jiafulow/L1TriggerSep2016.svg)](https://github.com/jiafulow/L1TriggerSep2016)

## Build

```shell
cd $CMSSW_BASE/src
git clone git@github.com:jiafulow/DataFormatsSep2016.git
git clone git@github.com:jiafulow/L1TriggerSep2016.git
scram b -j 8
```

## Develop

Please do not work on the 'master' branch directly. Create a new branch for new features.

