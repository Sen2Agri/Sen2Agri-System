install(FILES   src_ini/config.ini
                src_ini/S1_orbit_list.ini
                src_ini/S2_granule.ini
                src_s1/compliancy.py
                src_s1/fusion.py
                src_s1/run_command.txt
                src_s1/S1_gmd.py
                src_s1/S1_main.py
                src_s2/compliancy.py
                src_s2/fusion.py
                src_s2/model_lib.py
                src_s2/pheno_func.py
                src_s2/run_command.txt
                src_s2/S2_gmd.py
                src_s2/S2_main.py
                src_val/validation.py
                README.txt
                
        DESTINATION usr/share/sen2agri/S4C_L4B_GrasslandMowing/
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)
