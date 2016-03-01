#!/bin/bash
./sen2agri-ctl.py submit-job -s Ukraine composite -i \
"/mnt/archive/maccs_final/" \
--resolution 10 --synthesis-date 20151222 --half-synthesis 25 -p processor.l3a.synthesis_date 20151222 \
-p processor.l3a.half_synthesis 25 -p processor.l3a.bandsmapping /usr/share/sen2agri/bands_mapping_s2.txt -p processor.l3a.preproc.scatcoeffs /usr/share/sen2agri/scattering_coeffs_10m.txt
