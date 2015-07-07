#!/bin/sh
cd tools
txr gen-serialization.txr
cd ..
tools/format.sh
