#!/bin/bash

MYPWD=`pwd`
SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"

cd ${SCRIPTPATH}
java -cp '../modules/*:../lib/*:../config/*' org.esa.sen2agri.ServicesStartup

cd ${MYPWD}
