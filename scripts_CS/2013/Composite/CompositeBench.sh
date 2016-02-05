#!/bin/bash -l

echo "------------------------------------"
echo "-------     ARGENTINA     -------"
echo "------------------------------------"
echo "ARGENTINA Bench processing ..."
./Composite_Argentina_All.sh > /data/s2agri/output/2013/Argentina/Composite_Argentina_All.log 2>&1

echo "------------------------------------"
echo "-------     BELGIQUE     -------"
echo "------------------------------------"
echo "BELGIQUE Bench processing ..."
./Composite_Belgique_All.sh > /data/s2agri/output/2013/Belgium/Composite_Belgique_All.log 2>&1

echo "------------------------------------"
echo "-------     CHINA     -------"
echo "------------------------------------"
echo "CHINA Bench processing ..."
./Composite_China_All.sh > /data/s2agri/output/2013/China/Composite_China_All.log 2>&1

echo "------------------------------------"
echo "-------     FRANCE     -------"
echo "------------------------------------"
echo "FRANCE Bench processing ..."
./Composite_France_All.sh > /data/s2agri/output/2013/France/Composite_France_All.log 2>&1

echo "------------------------------------"
echo "-------     MADAGASCAR     -------"
echo "------------------------------------"
echo "MADAGASCAR Bench processing ..."
./Composite_Madagascar_All.sh > /data/s2agri/output/2013/Madagascar/Composite_Madagascar_All.log 2>&1

echo "------------------------------------"
echo "-------     MOROCCO     -------"
echo "------------------------------------"
echo "MOROCCO Bench processing ..."
./Composite_Morocco_All.sh > /data/s2agri/output/2013/Morocco/Composite_Morocco_All.log 2>&1

echo "------------------------------------"
echo "-------     SOUTH AFRICA     -------"
echo "------------------------------------"
echo "SOUTH AFRICA Bench processing ..."
./Composite_SouthAfrica_All.sh > /data/s2agri/output/2013/SouthAfrica/Composite_SouthAfrica_All.log 2>&1

echo "------------------------------------"
echo "-------        UKRAINE       -------"
echo "------------------------------------"
echo "UKRAINE Bench processing ..."
./Composite_Ukraine_All.sh > /data/s2agri/output/2013/Ukraine/Composite_Ukraine_All.log 2>&1

echo "------------------------------------"
echo "-------        US-MARICOPA       -------"
echo "------------------------------------"
echo "US-MARICOPA Bench processing ..."
./Composite_US-Maricopa_All.sh > /data/s2agri/output/2013/US-Maricopa/Composite_US-Maricopa_All.log 2>&1
