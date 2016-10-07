#!/bin/bash

# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

cd ${LOCAL_TEST_DIR}
rm Event*_out.root

(cmsRun pippo_cfg.py inputFiles=file:Event1687278667.root outputFile=Event1687278667_out.root >/dev/null 2>&1) || die 'Failure running Event 1687278667' $?
(python test_Event1687278667.py) || die 'Failure running Event 1687278667' $?

(cmsRun pippo_cfg.py inputFiles=file:Event1541093157.root outputFile=Event1541093157_out.root >/dev/null 2>&1) || die 'Failure running Event 1541093157' $?
(python test_Event1541093157.py) || die 'Failure running Event 1541093157' $?

(cmsRun pippo_cfg.py inputFiles=file:Event1686648178.root outputFile=Event1686648178_out.root >/dev/null 2>&1) || die 'Failure running Event 1686648178' $?
(python test_Event1686648178.py) || die 'Failure running Event 1686648178' $?

(cmsRun pippo_cfg.py inputFiles=file:Event1540061587.root outputFile=Event1540061587_out.root >/dev/null 2>&1) || die 'Failure running Event 1540061587' $?
(python test_Event1540061587.py) || die 'Failure running Event 1540061587' $?

(cmsRun pippo_cfg.py inputFiles=file:Event1686541662.root outputFile=Event1686541662_out.root >/dev/null 2>&1) || die 'Failure running Event 1686541662' $?
(python test_Event1686541662.py) || die 'Failure running Event 1686541662' $?

(cmsRun pippo_cfg.py inputFiles=file:Event1539957230.root outputFile=Event1539957230_out.root >/dev/null 2>&1) || die 'Failure running Event 1539957230' $?
(python test_Event1539957230.py) || die 'Failure running Event 1539957230' $?

(cmsRun pippo_cfg.py inputFiles=file:Event1540745931.root outputFile=Event1540745931_out.root >/dev/null 2>&1) || die 'Failure running Event 1540745931' $?
(python test_Event1540745931.py) || die 'Failure running Event 1540745931' $?

(cmsRun pippo_cfg.py inputFiles=file:Event1687229747.root outputFile=Event1687229747_out.root >/dev/null 2>&1) || die 'Failure running Event 1687229747' $?
(python test_Event1687229747.py) || die 'Failure running Event 1687229747' $?

