install(FILES   src_ini/config.ini
                src_ini/S1_orbit_list.ini
                src_ini/S2_granule.ini
        DESTINATION usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/src_ini/
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

install(FILES   src_s1/compliancy.py
                src_s1/fusion.py
                src_s1/run_command.txt
                src_s1/S1_gmd.py
                src_s1/S1_main.py
        DESTINATION usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/src_s1/
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

install(FILES   src_s2/compliancy.py
                src_s2/fusion.py
                src_s2/model_lib.py
                src_s2/pheno_func.py
                src_s2/run_command.txt
                src_s2/S2_gmd.py
                src_s2/S2_main.py
        DESTINATION usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/src_s2/
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

install(FILES   src_val/validation.py
                README.txt
        DESTINATION usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/src_val/
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

install(FILES   grassland_mowing.sh
                generate_grassland_mowing_input_shp.py
        DESTINATION usr/share/sen2agri/S4C_L4B_GrasslandMowing/Bin/
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

install(FILES   README.txt
        DESTINATION usr/share/sen2agri/S4C_L4B_GrasslandMowing/
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)
