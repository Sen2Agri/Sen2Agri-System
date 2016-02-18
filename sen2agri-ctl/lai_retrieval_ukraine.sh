#!/bin/bash
./sen2agri-ctl.py submit-job -s Ukraine lai -i \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130206_N2A_EUkraineD0000B0000/" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130226_N2A_EUkraineD0000B0000/" \
"/mnt/Sen2Agri_DataSets/L2A/Spot4-T5/Ukraine/SPOT4_HRVIR1_XS_20130318_N2A_EUkraineD0000B0000/" \
--resolution 20 --genmodel 0 --reproc 1 --fitted 1 -p processor.l3b.lai.localwnd.bwr 2 -p processor.l3b.lai.localwnd.fwr 0 -p processor.l3b.lai.modelsfolder /mnt/scratch/L3B_GeneratedModels/
