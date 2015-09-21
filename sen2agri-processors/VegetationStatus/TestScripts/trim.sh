#! /bin/bash

DECIMAL=$1

INTEGER=${DECIMAL%.*}
FRACT=${DECIMAL#*.}

echo "$INTEGER.${FRACT:0:1}"
