add_subdirectory(DataExtraction)
add_subdirectory(TimeSeriesAnalysis)
add_subdirectory(LPISDataSelection)

install(FILES ConfigFiles/S4C_L4C_Config_NLD.cfg
              ConfigFiles/S4C_L4C_Config_CZE.cfg
              ConfigFiles/S4C_L4C_Config_LTU.cfg
              ConfigFiles/S4C_L4C_Config_ROU.cfg
              ConfigFiles/S4C_L4C_Config_ESP.cfg
              ConfigFiles/S4C_L4C_Config_ITA.cfg
        DESTINATION usr/share/sen2agri/S4C_L4C_Configurations)