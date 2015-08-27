#!/bin/bash

export OTB_BV_SRC_DIR=${HOME}/Dev/otb-bv/src
export OTB_BV_HOME=${HOME}/Dev/builds/otb-bv
export PATH=${OTB_BV_HOME}/applications:$PATH
export LD_LIBRARY_PATH=${OTB_BV_HOME}/library:${OTB_BV_HOME}/applications:${LD_LIBRARY_PATH}
export ITK_AUTOLOAD_PATH=${OTB_HOME}/lib/otb/applications:${OTB_BV_HOME}/applications
export PYTHONPATH=${OTB_BV_SRC_DIR}/scripts:${PYTHONPATH}
