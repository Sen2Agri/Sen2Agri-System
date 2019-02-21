The package contains:

 - src_ini: directory with the configuration files:
            - config.ini:        contains processing parameters, input data directories and filename templates, compliance parameters, validation parameters
            - S1_orbit_list.ini: contains S1 orbit covering the area of interest and the corresponding orbit types (ASC and DESC)
            - S2_granule.ini:    contains S2 granules covering the area of interest

 - src_s1: directory with the python scripts for the S1 based chain:
            - S1_main.py:      main script
            - S1_gmd.py:       routines implementing algorithms and data management
            - fusion.py:       functions for writing results in the output shp file and implementing fusion criteria
            - compliancy.py:   routines to calculate the compliancy
            - run_command.txt: examples of command line to run the processing

 - src_s2: directory with the python scripts for the S2 based chain:
            - S2_main.py:      main script
            - S2_gmd.py:       routines implementing algorithms and data management
            - model_lib.py:    routines to calculate the model (used to perform mowing detection)
            - pheno_func.py:   function to fit the phenological model
            - fusion.py:       functions for writing results in the output shp file and implementing fusion criteria
            - compliancy.py:   routines to calculate the compliancy
            - run_command.txt: examples of command line to run the processing

 - src_val: directory with the python script for validation:
            - validation.py:  functions to perform validation


