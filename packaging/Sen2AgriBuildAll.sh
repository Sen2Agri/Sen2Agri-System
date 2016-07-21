#!/bin/bash
#set -x #echo on

##
## SCRIPT: BUILD SEN2AGRI
##
##
## SCRIPT STEPS
##     - CREATE DIR Sen2AgriRPM where generated RPMs will be copied
###########################CONFIG PART###########################################################
### CONFIG PATHS FOR SCRIPT
: ${DEFAULT_RPM_DIR:="Sen2AgriRPM"}
: ${DEFAULT_PATH:=$(pwd)}
################################################################################################
#-----------------------------------------------------------#
function build_working_dir()
{
   ##go to default dir
   cd ${DEFAULT_PATH}

   ##create working dir
   if [ ! -d ${DEFAULT_RPM_DIR} ]; then
      mkdir -p ${DEFAULT_RPM_DIR}
   fi
}
#-----------------------------------------------------------#
function show_menu(){
    NORMAL=`echo "\033[m"`
    MENU=`echo "\033[36m"` #Blue
    NUMBER=`echo "\033[33m"` #yellow
    FGRED=`echo "\033[41m"`
    RED_TEXT=`echo "\033[31m"`
    ENTER_LINE=`echo "\033[33m"`
    echo -e "${MENU}*********************************************${NORMAL}"
    echo -e "${MENU}**${NUMBER} 1)${MENU} Install BUILD PREREQUISITES ${NORMAL}"
    echo -e "${MENU}**${NUMBER} 2)${MENU} BUILD Sen2AgriPlatform : OTB and GDAL ${NORMAL}"
    echo -e "${MENU}**${NUMBER} 3)${MENU} BUILD Sen2AgriProcessors, Downloaders and Demmaccs ${NORMAL}"
    echo -e "${MENU}**${NUMBER} 4)${MENU} BUILD Sen2AgriApplication ${NORMAL}"
    echo -e "${MENU}**${NUMBER} 5)${MENU} BUILD Sen2AgriWebsite ${NORMAL}"
    echo -e "${MENU}**${NUMBER} 6)${MENU} BUILD Selective: Enter list of options (ex: platform, processors, website, app)  ${NORMAL}"
    echo -e "${MENU}**${NUMBER} 7)${MENU} BUILD Sen2Agri All Components (no OTB, GDAL)  ${NORMAL}"
    echo -e "${MENU}*********************************************${NORMAL}"
    echo -e "${ENTER_LINE}Please enter a menu option and enter or ${RED_TEXT}enter to exit. ${NORMAL}"
    read opt
}
#-----------------------------------------------------------#
function option_picked() {
    COLOR='\033[01;31m' # bold red
    RESET='\033[00;00m' # normal white
    MESSAGE=${@:-"${RESET}Error: No message passed"}
    echo -e "${COLOR}${MESSAGE}${RESET}"
}
###########################################################
##### PREPARE ENVIRONEMENT FOR SEN2AGRI BUILD          ###
###########################################################
##create working folder
build_working_dir
###########################################################
clear
show_menu
while [ opt != '' ]
    do
    if [[ $opt = "" ]]; then
            exit;
    else
        case $opt in
        1) clear;
           option_picked "Option 1 Picked";
           echo "====INSTALL PREREQUISITES FOR BUILD====";
           #get script path
           script_path=$(dirname $0);
           sudo sh ./$script_path/Sen2AgriPrereqForBuild.sh;
           wait;
           show_menu;
           ;;

        2) clear;
           option_picked "Option 2 Picked";
           echo "====BUILD Sen2AgriPlatform : OTB and GDAL===="
           #get script path
           script_path=$(dirname $0);

           #execute all the scripts
           sh ./$script_path/Sen2AgriPlatformBuild.sh;
           wait;

	   #copy generated RMPs
           cp -f ${DEFAULT_PATH}/Sen2AgriPlatform/rpm_binaries/*.rpm ${DEFAULT_PATH}/${DEFAULT_RPM_DIR};
           echo "====Generated RPM file moved to ${DEFAULT_RPM_DIR}====";
           rm -rf ${DEFAULT_PATH}/Sen2AgriPlatform;
           show_menu;
           ;;

        3) clear;
           option_picked "Option 3 Picked";
           echo "====Sen2AgriProcessors, Downloaders and Demmaccs====";
           #get script path
           script_path=$(dirname $0);

           #execute all the scripts
           sh ./$script_path/Sen2AgriProcessorsBuild.sh;
           wait;

	   #copy generated RMPs
           cp -f ${DEFAULT_PATH}/Sen2AgriProcessors/rpm_binaries/*.rpm ${DEFAULT_PATH}/${DEFAULT_RPM_DIR};
           echo "====Generated RPM file moved to ${DEFAULT_RPM_DIR}====";
           rm -rf ${DEFAULT_PATH}/Sen2AgriProcessors;
           show_menu;
           ;;

        4) clear;
           option_picked "Option 4 Picked";
           echo "====BUILD Sen2AgriApplication====";
           #get script path
           script_path=$(dirname $0);

           #execute all the scripts
           sh ./$script_path/Sen2AgriAppBuild.sh;
           wait;

	   #copy generated RMPs
           cp -f ${DEFAULT_PATH}/Sen2AgriApp/rpm_binaries/*.rpm ${DEFAULT_PATH}/${DEFAULT_RPM_DIR};
           echo "====Generated RPM file moved to ${DEFAULT_RPM_DIR}====";
           rm -rf ${DEFAULT_PATH}/Sen2AgriApp;
           show_menu;
           ;;
        5) clear;
           option_picked "Option 5 Picked";
           echo "====BUILD Sen2AgriWebsite====";
           #get script path
           script_path=$(dirname $0);

           #execute all the scripts
           sh ./$script_path/Sen2AgriWebSiteBuild.sh;
           wait;

	   #copy generated RMPs
           cp -f ${DEFAULT_PATH}/Sen2AgriWebSite/rpm_binaries/*.rpm ${DEFAULT_PATH}/${DEFAULT_RPM_DIR};
           echo "====Generated RPM file moved to ${DEFAULT_RPM_DIR}====";
           rm -rf ${DEFAULT_PATH}/Sen2AgriWebSite;
           show_menu;
           ;;
        6) clear;
           option_picked "Option 6 Picked";
           echo "====BUILD Selective: Enter list of options.(ex: platform, processors, website, app)====";
           echo "Type components name to be build and hit [ENTER]:";
           read components_list;

           #get script path dir
           script_path=$(dirname $0);
           if [[ "$components_list" =~ "processors" ]];
           then
              echo "====PROCESSORS BUILD===="
              #execute all the scripts
              sh ./$script_path/Sen2AgriProcessorsBuild.sh;
              wait;

	      #copy generated RMPs
              cp -f ${DEFAULT_PATH}/Sen2AgriProcessors/rpm_binaries/*.rpm ${DEFAULT_PATH}/${DEFAULT_RPM_DIR};
              echo "====Generated RPM files moved to ${DEFAULT_RPM_DIR}====";
              rm -rf ${DEFAULT_PATH}/Sen2AgriProcessors;
           fi

           if [[ "$components_list" =~ "platform" ]];
           then
              echo "====PLATFORM BUILD===="
              #execute all the scripts
              sh ./$script_path/Sen2AgriPlatformBuild.sh;
              wait;

              #copy generated RMPs
              cp -f ${DEFAULT_PATH}/Sen2AgriPlatform/rpm_binaries/*.rpm ${DEFAULT_PATH}/${DEFAULT_RPM_DIR};
              echo "====Generated RPM file moved to ${DEFAULT_RPM_DIR}====";
              rm -rf ${DEFAULT_PATH}/Sen2AgriPlatform;
           fi

           if [[ "$components_list" =~ "website" ]];
           then
              echo "====WEBSITE BUILD====";
              #execute all the scripts
              sh ./$script_path/Sen2AgriWebSiteBuild.sh;
              wait;

              #copy generated RMPs
              cp -f ${DEFAULT_PATH}/Sen2AgriWebSite/rpm_binaries/*.rpm ${DEFAULT_PATH}/${DEFAULT_RPM_DIR};
              echo "====Generated RPM file moved to ${DEFAULT_RPM_DIR}====";
              rm -rf ${DEFAULT_PATH}/Sen2AgriWebSite;
           fi

           if [[ "$components_list" =~ "app" ]];
           then
              echo "====SERVICES BUILD====";
              #execute all the scripts
              sh ./$script_path/Sen2AgriAppBuild.sh;
              wait;

              #copy generated RMPs
              cp -f ${DEFAULT_PATH}/Sen2AgriApp/rpm_binaries/*.rpm ${DEFAULT_PATH}/${DEFAULT_RPM_DIR};
              echo "====Generated RPM file moved to ${DEFAULT_RPM_DIR}====";
              rm -rf ${DEFAULT_PATH}/Sen2AgriApp;
           fi
           show_menu;
           ;;

        7) clear;
           option_picked "Option 7 Picked";
           echo "====BUILD Sen2Agri ALL COMPONENTS (no OTB, GDAL)====";

           #get script path
           script_path=$(dirname $0);

           #execute all the scripts
           sh ./$script_path/Sen2AgriProcessorsBuild.sh;
           wait;
           sh ./$script_path/Sen2AgriWebSiteBuild.sh;
           wait;
           sh ./$script_path/Sen2AgriAppBuild.sh;
           wait;
           #copy generated RMPs
           cp -f ${DEFAULT_PATH}/Sen2AgriProcessors/rpm_binaries/*.rpm ${DEFAULT_PATH}/${DEFAULT_RPM_DIR};
           cp -f ${DEFAULT_PATH}/Sen2AgriWebSite/rpm_binaries/*.rpm ${DEFAULT_PATH}/${DEFAULT_RPM_DIR};
           cp -f ${DEFAULT_PATH}/Sen2AgriApp/rpm_binaries/*.rpm ${DEFAULT_PATH}/${DEFAULT_RPM_DIR};
           echo "====Generated RPMs files moved to ${DEFAULT_RPM_DIR}====";
           rm -rf ${DEFAULT_PATH}/Sen2AgriProcessors;
           rm -rf ${DEFAULT_PATH}/Sen2AgriWebSite;
           rm -rf ${DEFAULT_PATH}/Sen2AgriApp;
           show_menu;
           ;;

        x)exit;
        ;;

        \n)exit;
        ;;

        *)clear;
        option_picked "Pick an option from the menu";
        show_menu;
        ;;
    esac
fi
done
