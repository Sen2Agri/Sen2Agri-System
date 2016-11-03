# Extraction script -- USE OF OTB AND CUSTOM CPP FILES
# INPUT (5 files) : 
# - the list of original images (can be a mix Spot4 and Landsat8 ordered by date) : stacked radiance file (b1d1,b2d1,b3d1,b4d1,b1d2,...) ; 
# - the list of validity masks (binary images) which are products from the original cloud masks
# - "NDVI.tif" : ndvi stacked file (ndvid1,ndvid2,...) ; 
# - "DateList.txt" : datefile (txt file, one row = one date) ;
# - "mask4nan.tif" : mask of effective working area (0 where no data; 1 where data is available)
# OUTPUT (5 files) : stacked radiance (b1,b2,b3,b4) of corresponding to the date of the MAX NDVI value, NDVI slope, RED (band2) value and the MIN NDVI value, NDVI slope.

# Each site must be in a separated folder
# 
# This file is produced in the idea to compute the radiances coming from the 3 months after the 1st feb with the "cesbio's method" (closest to the threshold)
#
# PAY ATTENTION TO : 
#
# MUST specify server used (6,7,8) in str (serv variable)
# MUST change the fsmo_w function (l65) : site specific function depending the number of acquisition dates available for the site
#
# @ author : nmatton ; last update : 10/01/2015 (0.924)

rm(list=ls())
library(raster)
library(sp)
if (system('uname -n',intern=T) == "eliegeo06.oasis.uclouvain.be") {
  serv_name<-'six'
  serv_num<-6}
if (system('uname -n',intern=T) == "eliegeo07") {
  serv_name<-'sept'
  serv_num<-7}

if (serv_num=='6') ncores<-8 else ncores<-4
m_values<-FALSE  # Parameter to choose if we create the corresponding max/min value of ndvi/red/slope of if just the "wichmax" is enough
pkgs<-c('rgdal','sp','snow','raster','doParallel','foreach','ptw')
for(x in pkgs){
  if(!is.element(x, installed.packages()[,1]))
  {install.packages(x, repos ="http://cran.fhcrc.org")
  } else {print(paste(x, " library already installed"))}
}


library(raster)
library(rgdal)
library(stats)
library(snow)
library(doParallel)
library(foreach)
library(rgdal)
library(ptw)

#inSitesList<- c('/export/miro/DVD/jecam/WorkingData/Ukraine/2013/s4l8/02_Reflectances/','/export/miro/DVD/jecam/WorkingData/Belgium/2013/s4l8_v2/02_Reflectances/','/export/miro/DVD/jecam/WorkingData/Argentina/2013/s4l8/02_Reflectances/','/export/miro/DVD/jecam/WorkingData/China/2013/s4l8/02_Reflectances/','/export/miro/DVD/jecam/WorkingData/Madagascar/2013/s4l8/02_Reflectances/','/export/miro/DVD/jecam/WorkingData/Maricopa/2013/s4l8/02_Reflectances/','/export/miro/DVD/jecam/WorkingData/Morocco/2013/s4l8_v3/02_Reflectances/','/export/miro/DVD/jecam/WorkingData/SouthAfrica/2013/s4l8/02_Reflectances/','/export/miro/DVD/jecam/WorkingData/SudMiPy/2013/s4l8_v5/02_Reflectances/')
inSitesList <- c('/export/miro/DVD/jecam/WorkingData/Belgium/2013/whit_test/Ukraine/OUT/combined/')
# Functions


## Looking for max/min in raster
fmean  <- function(x) calc(x,mean)
fwhmax <- function(x) calc(x, fun       =function(y) ifelse(is.na(y) ==FALSE,which.max(y),0))
fmax   <- function(x) calc(x,max)
fwhmin <- function(x) calc(x, fun       =function(y) ifelse(is.na(y) ==FALSE,which.min(y),0))
fmin   <- function(x) calc(x,min)
fadd   <- function(x) calc(x,fun        =function(y)(y+2))
fnan   <- function(x,y) overlay(x,y,fun =function(x,y){x*y}) # linked to the wittaker
fsmo   <- function(x) calc(x,fun        =function(x){whit1(x,2)}) #Whitaker filter from the previous release using the smoothed band as input
fsmo_w <- function(x) calc(x,fun=function(x){whit1(x[1:28],2,x[29:56])}) #### NEW WHITAKER FILTER using the original band and the validity filters as input ###### Values in x[$] site dependent !! ###
#fsmo_w <- function(x) calc(x,fun=function(x){whit1(x[1:nlayers(x)/2],2,x[(nlayers(x)/2)+1:nlayers(x)])}) #### $ Values in x[$] site dependent !! - nlayers/length/defining function later on script does not work


## Whittaker function (smooth)
#fwsm<-function(x) {
#  out<-whit1(x,2)
#  return(out)
#}

#rasterOptions(tmpdir='/scratch/')
dir.create(paste('/scratch/',serv_name,'/',sep=''),showWarnings=F)
rasterOptions(tmpdir=paste('/scratch/',sep=''))


#### Script begining ####

for (site in inSitesList){
  # Chose here which data you want to produce
  calc_c0<-FALSE # 3 months of data since 1st feb
  calc_c1<-FALSE # 6 months of data since 1st feb
  calc_c2<-FALSE  # 9 months of data since 1st feb
  calc_c3<-TRUE # all data
  print('script begining for :')
  print(site)
  print(Sys.time()) 
  dir.create(paste(site,'slopes/',sep=''),showWarnings=F) #Creating usefull folder (nothing done if folder already exist)
  dir.create(paste(site,'smooth/',sep=''),showWarnings=F) #Creating usefull folder (nothing done if folder already exist)
  dir.create(paste(site,'vrt/',sep=''),showWarnings=F) #Creating usefull folder (nothing done if folder already exist)
  dir.create(paste(site,'temp/',sep=''),showWarnings=F) #Creating usefull folder (nothing done if folder already exist)
  #for (i in 1:4){
  #dir.create(paste(site,'vrt/',i,'/',sep=''),showWarnings=F) #Creating usefull folder (nothing done if folder already exist)
  #}
  
  ## -------- This is for the correction of the date problems ---- TO REMOVE FOR COMMON USE -----------
  if (grepl('Ukraine',site)) calc_c0<-FALSE
  if (grepl('SouthAfrica',site)) calc_c2<-TRUE

  ## --------------------------------------------------------------------------------------------------

  # Dealing with dates (extracting, conversing, finding the 6,9,12 months dates)  ####
  dates<-scan(paste(site,'SL_MultiTempGapF_4bpi_DateList.txt',sep=''),what=character(),sep=';') # %%%%%%%%%%%%%%%%%%%%%%%% NAME SPECIFIC LINE -- 
    sdates <- strptime(dates, format = "%Y%m%d")
    diffdate<-rep(0,length(sdates)-1)
    for (i in 1:length(sdates)-1){
      diffdate[i] <- as.integer(sdates[i+1]-sdates[i])    # Computing the number of day separating each data
    }
    diffrefdate<-rep(0,length(sdates))
    for (i in 1:length(sdates)){
      diffrefdate[i] <- as.integer(sdates[i]-strptime("20130201", format = "%Y%m%d"))    # Computing the number of day separating each data
    }
    cumdates<-cumsum(diffdate)
    nmt3<-!any(diffrefdate>=88)
    nmt6<-!any(diffrefdate>=180)
    nmt9<-!any(diffrefdate>=270)
    i<-1  
    # while (diffrefdate[i]<=90 & i<length(diffrefdate)){i<-i+1}
    m3<-which.min(abs(diffrefdate-88))
    if (nmt3) {
      m3<-length(sdates)
    }
    m6<-which.min(abs(diffrefdate-180))
    if (nmt6) {
      m6<-length(sdates)
    }
    while (diffrefdate[i]<=270 & i<length(diffrefdate) & !nmt6){i<-i+1}
    m9<-which.min(abs(diffrefdate-270))
    if (nmt9) {
      m9<-length(sdates)
    } 
  
  ### Creation of intermediate Output ####
  
  ## Creating separated bands files ####
  #inFile <- paste(site,'SL_MultiTempGapF_4bpi.tif',sep='') # %%%%%%%%%%%%%%%%%%%%%%%%  NAME SPECIFIC LINE -- 
  inFileList <- list.files(paste0(site,'/reflectances/'),full.name=TRUE) #<- paste(site,'SL_MultiTempGapF_4bpi.tif',sep='') # %%%%%%%%%%%%%%%%%%%%%%%%  NAME SPECIFIC LINE -- 
  crs_ref<-crs(raster(inFileList[1])) # getting current crs for output
  print('date selection done, creating separated band vrt files')
  print(Sys.time()) 
  for (sband in 1:4){
    outFileName<-paste(site,'band',sband,'.tif',sep='')
    if (file.exists(outFileName)) next
    s<-stack()
    for (inFile in inFileList) {
      s<-stack(s,raster(inFile,band=sband))
    }
    crs(s)<-crs(crs_ref)
    r<-writeRaster(s,outFileName)
    print(paste('band',sband,'done'))
  }


    print(paste('smoothing NDVI; time :',Sys.time()))
  ## Creating a smoothed stacked ndvi file ####
  ndvi_smo <- paste(site,'NDVIsmo.tif',sep='')
  if (!file.exists(ndvi_smo)){
    ndvi_s<-paste(site,'/NDVI.tif',sep='') # raw NDVI datafile #
    rweights<-stack(list.files(paste0(site,'/masks/'),full.name=TRUE))
    print('stack done, replacing NA by 0')
    rweights_z<-calc(rweights,function(x) { x[is.na(x)] <- 0; return(x) })
    print('NA replaced, making stacks')
    raw_ndvi<-stack(ndvi_s)
    print('stack done, replacing NA by 0')
    raw_ndvi_z<-calc(raw_ndvi,function(x) { x[is.na(x)] <- 0; return(x) })
    instndvi_w<-stack(raw_ndvi_z,rweights_z)
    print('stacks done, apllying whitaker')
    beginCluster(10)
    outW_ndvi<-clusterR(instndvi_w,fun=fsmo_w)
    endCluster()
    crs(outW_ndvi)<-crs(crs_ref)
    print('smooth done, writing...')
    cr<-writeRaster(outW_ndvi,ndvi_smo,datatype='FLT4S',options=c("INTERLEAVE=BAND","COMPRESS=NONE", "TFW=YES"),overwrite=TRUE)
  }

 ## Creating a smoothed stacked band file####
  print('smoothing the bands')
  print(Sys.time()) 
  for (sband in 1:4){
    if (file.exists(paste(site,'band',sband,'_smo.tif',sep=''))) next
    inFile<-paste(site,'/band',sband,'.tif',sep='')
    raw_band<-stack(inFile)
    #instndvi<-overlay(raw_band,rnan,fun=function(x,y){x*y})
    print('stack done, replacing NA by 0')
    if(!exists('rweights_z')) {
      rweights<-stack(list.files(paste0(site,'/masks/'),full.name=TRUE))
      rweights_z<-calc(rweights,function(x) { x[is.na(x)] <- 0; return(x) })
    }
    raw_band_z<-calc(raw_band,function(x) { x[is.na(x)] <- 0; return(x) })
    instbd_w<-stack(raw_band_z,rweights_z)
    print('stacks done, aplying whitaker')
    beginCluster(10)
    out_band<-clusterR(instbd_w,fun=fsmo_w)
    endCluster()
    #out_band<-calc(r,fun=fwsm)
    crs(out_band)<-crs(crs_ref)
    band_smo<-paste(site,'band',sband,'_smo.tif',sep='')
    print('smooth done, writing...')
    cr<-writeRaster(out_band,band_smo,datatype='INT2S',options=c("INTERLEAVE=BAND","COMPRESS=NONE", "TFW=YES"),overwrite=TRUE)
    
  }

  print('computing the slopes')
  print(Sys.time()) 
  ## Computing the slopes
  for (i in 3:(length(dates))) {
    date1      <- strptime(dates[i-2], format = "%Y%m%d")
    date2      <- strptime(dates[i], format = "%Y%m%d")
    diffdate   <- as.integer(date2-date1)
    if (i<11) ndvi_slope <- paste(site,'slopes/NDVI_slope_0',i-1,'.tif',sep='') else ndvi_slope <- paste(site,'slopes/NDVI_slope_',i-1,'.tif',sep='')
    if (file.exists(ndvi_slope)) next
    batch_line <- paste(' /home/OASIS/julien/',serv_name,'/OTBbin/bin/otbcli_BandMath -il ',ndvi_smo,' -out ',ndvi_slope, " -exp '(im1b",i-2,"-im1b",i,")/",diffdate,"'",sep='')
    system(batch_line)
  }

  print('Intermediate Output production Finished :::::  EXTRACTION OF Features :::::')
  print(Sys.time()) 
  ### Intermediate Output production Finished ::::: EXTRACTION OF Features :::::  ####
  inFileList <- sort(list.files(paste(site,'/slopes/',sep=''),pattern='_slope_',full.names = T))
  num_of_case<-3
  if (nmt9) num_of_case<-2
  if (nmt6) num_of_case<-1
  if (nmt3) num_of_case<-0
  system(paste('rm ',site,'num_of_case.txt -f',sep=''))
  system(paste("echo '",num_of_case,"' > ",site,"/num_of_case.txt",sep='')) # output num of case for the unsup and so on..
  for (case in 0:num_of_case) {
    if (case==0 & !calc_c0) next
    if (case==1 & !calc_c1) next
    if (case==2 & !calc_c2) next
    if (case==3 & !calc_c3) next
    if (case==0) fstack <- stack(inFileList[1:(m3-2)])
    if (case==1) fstack <- stack(inFileList[1:(m6-2)])
    if (case==2) fstack <- stack(inFileList[1:(m9-2)])
    if (case==3) fstack <- stack(inFileList)
    print(paste('processing case ',case))
    print(Sys.time()) }

  ## Look for max/min ndvi slope####
  
      # Stacking the slopes
  
  
  crs(fstack)<-crs(crs_ref)
  
  print('looking for max/min ndvi slope')
  print(Sys.time()) 
  beginCluster(10)
  whmax <- clusterR(fstack, fwhmax, m=50)
  crs(whmax)<-crs(crs_ref)
  whmin <- clusterR(fstack, fwhmin, m=50)
  crs(whmin)<-crs(crs_ref)
  
  # If the exact max/min slope values are wanted (not just "when" they are) : m_values definition at the begining of the file
  if (m_values){
    print('looking for max/min ndvi slope value')
    vmax  <- clusterR(fstack, fmax, m=50)
    vmin  <- clusterR(fstack, fmin, m=50)
    pmax  <- stack(vmax, bands=1)
    crs(pmax)<-crs(crs_ref)
    pmin  <- stack(vmin, bands=1)
    crs(pmin)<-crs(crs_ref)
    print('writing max_slope-c3')
    rmax   <- writeRaster(pmax,paste(site,'max_slope-c',case,'.tif',sep=''),datatype='INT1U',options=c("COMPRESS=NONE", "TFW=YES"),overwrite=TRUE)
    print('writing min_slope-c3')
    rmin   <- writeRaster(pmin,paste(site,'min_slope-c',case,'.tif',sep=''),datatype='INT1U',options=c("COMPRESS=NONE", "TFW=YES"),overwrite=TRUE)
  }
  endCluster()
  print('writing results')
  print(Sys.time()) 
  # Writing results
  rwhmax <- writeRaster(whmax,paste(site,'whichmax_slope0-c',case,'.tif',sep=''),datatype='INT1U',options=c("COMPRESS=NONE", "TFW=YES"),overwrite=TRUE)
  rwhmin <- writeRaster(whmin,paste(site,'whichmin_slope0-c',case,'.tif',sep=''),datatype='INT1U',options=c("COMPRESS=NONE", "TFW=YES"),overwrite=TRUE)
  
  
  ## Look for max/min ndvi value####
  
  if (case==0) fstack <- stack(ndvi_smo,bands=1:m3)
  if (case==1) fstack <- stack(ndvi_smo,bands=1:m6)
  if (case==2) fstack <- stack(ndvi_smo,bands=1:m9)
  if (case==3) fstack <- stack(ndvi_smo)
  
  #fstack <- stack(ndvi_smo)
  
  print('looking for max/min ndvi value')
  print(Sys.time()) 
  beginCluster(10)
  whMm <- clusterR(fstack, fwhmax, m=50)
  crs(whMm)<-crs(crs_ref)
  whmm <- clusterR(fstack, fwhmin, m=50)
  crs(whmm)<-crs(crs_ref)
  
  # If the exact max/min ndvi values are wanted (not just "when" they are) : m_values definition at the begining of the file
  if (m_values){
    vmax  <- clusterR(fstack, fmax, m=50)
    vmin  <- clusterR(fstack, fmin, m=50)
    pmax  <- stack(vmax, bands=1)
    crs(pmax)<-crs(crs_ref)
    pmin  <- stack(vmin, bands=1)
    crs(pmin)<-crs(crs_ref)
    print('writing results...')
    rmax   <- writeRaster(pmax, paste(site,'max_ndvi-c',case,'.tif',sep=''),overwrite=TRUE)
    rmin   <- writeRaster(pmin, paste(site,'min_ndvi-c',case,'.tif',sep=''),overwrite=TRUE)
  }
  endCluster()
  print('writing results')
  print(Sys.time()) 
  # Write results
  rwhmax <- writeRaster(whMm, paste(site,'whichmax_ndvi-c',case,'.tif',sep=''),datatype='INT1U',options=c("COMPRESS=NONE", "TFW=YES"),overwrite=TRUE)
  rwhmin <- writeRaster(whmm, paste(site,'whichmin_ndvi-c',case,'.tif',sep=''),datatype='INT1U',options=c("COMPRESS=NONE", "TFW=YES"),overwrite=TRUE)
  
  ## Look for max red value####

    if (case==0) fstack <- stack(paste(site,'band2_smo.tif',sep=''),bands=1:m3)
    if (case==1) fstack <- stack(paste(site,'band2_smo.tif',sep=''),bands=1:m6)
    if (case==2) fstack <- stack(paste(site,'band2_smo.tif',sep=''),bands=1:m9)
    if (case==3) fstack <- stack(paste(site,'band2_smo.tif',sep=''))
    
    #fstack <-stack(paste(site,'band2_smo.tif',sep=''))
    print('looking for max red (band2)')
    print(Sys.time()) 
    beginCluster(10)
    whmax <- clusterR(fstack, fwhmax, m=50)
    crs(whmax)<-crs(crs_ref)
    # If the exact max red values are wanted (not just "when" they are) : m_values definition at the begining of the file
    if (m_values){
      vmax  <- clusterR(fstack, fmax, m=50)
      pmax  <- stack(vmax, bands=1)
      crs(pmax)<-crs(crs_ref)
      rmax   <- writeRaster(pmax, paste(site,'max_red-c',case,'.tif',sep=''),,overwrite=TRUE)
    }
    endCluster()
    print('writing results')
    print(Sys.time()) 
    # Write results
    rwhmax <- writeRaster(whmax, paste(site,'whichmax_red-c',case,'.tif',sep=''),datatype='INT1U',options=c("COMPRESS=NONE", "TFW=YES"),overwrite=TRUE)
    
  print("correction of index for slope features")
  # Correction of index to correspond with smoothed data (since we cannot compute the slope for the first&last dates) => offset=+1 (+2 if raw data extraction; here : smoothed data) ####
  for (ele in c('max','min')){
    fileIn<-paste(site,'which',ele,'_','slope0-c',case,'.tif',sep='')
    fileOut<-paste(site,'which',ele,'_','slope-c',case,'.tif',sep='')
    batch_line<-paste('/home/OASIS/julien/',serv_name,'/OTBbin/bin/otbcli_BandMath -il ',fileIn,' -out ',fileOut, " -exp 'im1b1+1'",sep='')
    system(batch_line)
  }

  ## Finding the features dones ::: Extracting the radiances now ####
  print('extracting the radiances from the features')
  print(Sys.time()) 
  # Extraction of bands for max/min slope and ndvi
  sel_file<-c('ndvi','slope')
  for(sel in sel_file){
    for (tmm in c('max','min')){
      outfile<-paste(site,"rad_",tmm,sel,"-c",case,".tif",sep='')
      infile<-paste(site,"which",tmm,'_',sel,"-c",case,".tif",sep='')
      inbands<-paste(site,'band1_smo.tif ',site,'band2_smo.tif ',site,'band3_smo.tif ',site,'band4_smo.tif',sep='')
      batch_line<-paste("/export/miro/sepulcrecant/git/code/otb/pixel/build/build",serv_num,"/selectInList ",outfile,' ',infile,' ',inbands,sep="")
      system(batch_line)
    }
  }

  # Extraction of bands for max red
  outfile<-paste(site,"rad_maxred-c",case,".tif",sep='')
  infile<-paste(site,"whichmax_red-c",case,".tif",sep='')
  inbands<-paste(site,'band1_smo.tif ',site,'band2_smo.tif ',site,'band3_smo.tif ',site,'band4_smo.tif',sep='')
  batch_line<-paste("/export/miro/sepulcrecant/git/code/otb/pixel/build/build",serv_num,"/selectInList ",outfile,' ',infile,' ',inbands,sep="")
  system(batch_line)
  
  print('creation of some vrts...')
  print(Sys.time()) 
  # Creation of separated vrt files -- usefull for unsup classif ####
  bash_group<-''
  bash_line<-''
  for (sel in sel_file){
    for (tmm in c('max','min')){
      outfile<-paste(site,"rad_",tmm,sel,"-c",case,".tif",sep='')
      for (band in 1:4){
        bash_line[band]<-paste('gdalbuildvrt ',site,'vrt/',tmm,'_',sel,band,'-c',case,'.vrt ',outfile,' -b ',band,sep='')
        show(paste(tmm,sel,band))
      } 
      bash_group<-append(bash_group,bash_line)
    }  
  }

  for (band in 1:4){
    outfile<-paste(site,"rad_maxred-c",case,".tif",sep='')
    bash_line[band]<-paste('gdalbuildvrt ',site,'vrt/max_red',band,'-c',case,'.vrt ',outfile,' -b ',band,sep='')    
  }

  bash_group<-append(bash_group,bash_line)
  bash_group<-bash_group[! bash_group %in% '']
  outbash<-paste(site,'vrt/bash_file.sh',sep='')
  write(bash_group,outbash)
  system(paste('bash',outbash),invisible = F)
  print(Sys.time()) 
  }

print('process done, cleaning the temp folder (scratch)')
#system(paste('rm -rf /scratch/',serv_name,'/*',sep=''))
print(Sys.time()) 
}
print(Sys.time()) 