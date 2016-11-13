#!/bin/bash

# From Docker
echo ">> HOME=$HOME"
echo ">> HOSTNAME=$HOSTNAME"
echo ">> USER=$USER"
echo ">> PWD=$PWD"

# From CVMFS
echo ">> CMSSW_PATH=$CMS_PATH"
echo ">> VO_CMS_SW_DIR=$VO_CMS_SW_DIR"

# From Travis
echo ">> SCRAM_ARCH=$SCRAM_ARCH"
echo ">> CMSSW_VERSION=$CMSSW_VERSION"

echo "Setting up ..."

mkdir -p /home/build && cd /home/build
scramv1 project CMSSW $CMSSW_VERSION
cd $CMSSW_VERSION/src
eval `scramv1 runtime -sh`
git clone --depth 1 https://github.com/jiafulow/L1TriggerSep2016.git
git clone --depth 1 https://github.com/jiafulow/DataFormatsSep2016.git

echo "Compiling ..."

scram b -j 10
EXIT_STATUS=$?
if [ $EXIT_STATUS -ne 0 ]; then exit $EXIT_STATUS; fi

echo "Running tests ..."

cd L1TriggerSep2016/L1TMuonEndCap/
scram b runtests
EXIT_STATUS=$?
if [ $EXIT_STATUS -ne 0 ]; then exit $EXIT_STATUS; fi

echo "DONE"
