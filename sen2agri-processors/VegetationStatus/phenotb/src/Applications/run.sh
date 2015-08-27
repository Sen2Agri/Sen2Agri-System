#!/bin/bash

year=2006

export ITK_AUTOLOAD_PATH=""
rm -rf /tmp/test-2*.gp
rm -rf /tmp/test-1*.gp
rm -rf /tmp/test-3*.gp
rm -rf /tmp/test-*.gp
rm -rf /tmp/NGdir/*
/home/inglada/Dev/builds/phenotb/Applications/explore-lai ~/stok/DATA/LAI_F2_Aurade/LAI_F2_Aurade.tif ~/stok/DATA/LAI_F2_Aurade/LAI_F2_Aurade_dates.txt $year -32768
mv /tmp/*NG.gp /tmp/NGdir/
python plot.py /tmp/p5.pdf /tmp/test-5*gp > /tmp/plots5.gp
python plot.py /tmp/p4.pdf /tmp/test-4*gp > /tmp/plots4.gp
python plot.py /tmp/p3.pdf /tmp/test-3*gp > /tmp/plots3.gp
python plot.py /tmp/p2.pdf /tmp/test-2*gp > /tmp/plots2.gp
python plot.py /tmp/p1.pdf /tmp/test-1*gp > /tmp/plots1.gp
gnuplot /tmp/plots1.gp & gnuplot /tmp/plots2.gp & gnuplot /tmp/plots5.gp & gnuplot /tmp/plots4.gp
gnuplot /tmp/plots3.gp
mv -f /tmp/p?.pdf /home/inglada/stok/DATA/LAI_F2_Aurade/resultats/$year/




# export ITK_AUTOLOAD_PATH=""
# /home/inglada/Dev/builds/phenotb/Applications/explore-maiseo ~/stok/DATA/LAI_F2_Aurade/Profils_NDVI_param_cult_${year}.csv $year
# python ./plot.py /tmp/maiseo.pdf /tmp/maiseo*gp > /tmp/maiseo.gp
# gnuplot /tmp/maiseo.gp 
