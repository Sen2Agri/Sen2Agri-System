#!/usr/bin/env Rscript
library(ranger)
library(dplyr)
library(e1071)
library(caret)
library(smotefamily)
library(readr)
library(gsubfn)


#Implement variables
variable <- commandArgs(trailingOnly=TRUE)
print(variable)
workdir <- variable[1]  #transform to numeric !!
InputSAR <- variable[2]
InputOpt <- variable[3]
InputSAR_tempStats <- variable[4]
Shape_filename <- variable[5]
training_ratio <- as.numeric(variable[6])
samplingmethod <-variable[7]
numtrees <- as.numeric(variable[8])
sample_size <- as.numeric(variable[9])
count_thresh <- as.numeric(variable[10])
count_min <- as.numeric(variable[11])
smotesize <- as.numeric(variable[12])
k <- as.numeric(variable[13])

if (!(samplingmethod %in% c("Random","Areaweighted","Overareaweighted","Overunderareaweighted","Smote","Fixunder"))) {
  stop('Wrong argument: choose out of "Random","Areaweighted","Overareaweighted","Overunderareaweighted","Smote","Fixunder"', call.=FALSE)
}
#
# samplingmethod=c("Random","Areaweighted","Overareaweighted","Overunderareaweighted","Smote")
# samplingmethod=samplingmethod[1]
# #Random is a random sampling among the NewIDs in the calibration subset
# #Areaweighted is a weighted sampling among the calibration dataset based on the area of the whole dataset
# #Overareaweighted //Areaweighted + deleting classes below min and oversampling classes below threshold to threshold number
#   #of records (or all the records if count<threshold)) ; 'resulting sample_size' > 'sample_size' (>> function of threshold value)
# #Overunderareaweighted //Overareaweighted + undersampling classes above threshold to keep sample_size=sample_size
# #Smote sampling //Overareaweighted with additional artificial observations to ensure all classes reach the threshold value
#
#import features with NewID column as CSV

#import shapefile (nrow = nrow(Features)) as CSV
print("Importing shapefile...")
Shapefile=read_csv(Shape_filename, col_types = "idii")

    if (length(which(names(Shapefile) %in% c("AREA","CTnum","NewID")))!=3) {
      stop('Wrong argument: Shapefile names do not contain "AREA","CTnum" and/or "NewID"', call.=FALSE)
    }

names(Shapefile)[which(names(Shapefile)=="CTnum")]="TARGET"

#remove shapefile rows containing any NAs
#Shapefile[Shapefile==0] <- NA

#print(paste("Removed column from shapefile containing NAs:",Shapefile %>% select_if(~ any(is.na(.))) %>% names()))
#Shapefile = Shapefile %>%  select_if(~ !any(is.na(.)))

#join features and shapefile by "NewID" (RF Features (and no other variable) should start with XX)
if (InputSAR==0 & InputOpt!=0) {
  print("Optical only")

  print("Importing Opt_Features...")
  ncol_Optcsv=system(paste("head -1",InputOpt,"| sed 's/[^,]//g' | wc -c"),intern=TRUE)
  Opt_features=read_csv(InputOpt,col_types=paste0("i",paste(rep("d",as.numeric(ncol_Optcsv)-1),collapse="")))

  Opt_features[Opt_features==0] <- NA

  data_joined=inner_join(Shapefile,Opt_features,by="NewID")

} else if (InputOpt==0 & InputSAR!=0) {
  print("SAR only")

  print("Importing SAR_Features...")
  SAR_features=read_csv(InputSAR)

  SAR_features[SAR_features==0] <- NA

  data_joined=inner_join(Shapefile,SAR_features,by="NewID")

} else if (InputSAR!=0 & InputOpt!=0 & InputSAR_tempStats!=0){
#} else if (InputSAR!=0 & InputOpt!=0){
#  print(mem_used())
  print("Optical, SAR and SAR_tempStats ")

  print("Importing Opt_Features...")
  ncol_Optcsv=system(paste("head -1",InputOpt,"| sed 's/[^,]//g' | wc -c"),intern=TRUE)
  Opt_features=read_csv(InputOpt,col_types=paste0("i",paste(rep("d",as.numeric(ncol_Optcsv)-1),collapse="")))

  print("Importing SAR_Features...")
  ncol_SARcsv=system(paste("head -1",InputSAR,"| sed 's/[^,]//g' | wc -c"),intern=TRUE)
  SAR_features=read_csv(InputSAR,col_types=paste0("i",paste(rep("d",as.numeric(ncol_SARcsv)-1),collapse="")))

  print("Importing SAR Temporal Features...")
  ncol_SARcsv=system(paste("head -1",InputSAR_tempStats,"| sed 's/[^,]//g' | wc -c"),intern=TRUE)
  SAR_tempStats=read_csv(InputSAR_tempStats,col_types=paste0("i",paste(rep("d",as.numeric(ncol_SARcsv)-1),collapse="")))

  print("SAR, Optical and SAR temporal features imported successfully")
  #print("SAR and Optical features imported successfully")
  #print(length(which(Opt_features==0)))
  #Opt_features[Opt_features==0] <- NA
  #SAR_features[SAR_features==0] <- NA
  #SAR_features[SAR_features<0] <- NA
  #SAR_tempStats[SAR_tempStats==0] <- NA
  #print("Null values replaced by NA in Optical, SAR, and SA temporal data")
#STEP ADDED for Czech site where some images are missing in the timeseries, resutling in features mostly full of NA...


    #na_count <-sapply(SAR_features, function(y) sum(length(which(is.na(y)))))
    #n_parcels=dim(SAR_features)[1]
    #n_NA2rm=round(n_parcels*0.5,digits=0)
    #n_na_col<-length(which(na_count > n_NA2rm ))
    #print(paste("There are",n_na_col,"columns with more than",n_NA2rm,"NA"))
    #print("...Removing these columns")
    #if (n_na_col != 0){
    #SAR_features=SAR_features[-which(na_count > n_NA2rm)]
    #}

    #na_count <-sapply(SAR_tempStats, function(y) sum(length(which(is.na(y)))))
    #n_parcels=dim(SAR_tempStats)[1]
    #n_NA2rm=round(n_parcels*0.5,digits=0)
    #n_na_col<-length(which(na_count > n_NA2rm ))
    #print(paste("There are",n_na_col,"columns with more than",n_NA2rm,"NA"))
    #print("...Removing these columns")
    #if (n_na_col != 0){
    #SAR_tempStats=SAR_tempStats[-which(na_count > n_NA2rm)]
    #}

print("by 1 ok")
data_joined=inner_join(Shapefile,SAR_features,by="NewID")
print("by 2 ok")
data_joined=inner_join(data_joined,Opt_features,by="NewID")
print("by 3 ok")
data_joined=inner_join(data_joined,SAR_tempStats,by="NewID")
print("by 4 ok")
} else if (InputSAR==0 & InputOpt==0){
  print("Define SAR or Optical parameters")
}

data_joined=as.data.frame(data_joined)
print(dim(data_joined))

##check column names i.e. "AREA", "TARGET",

# factorize "class" variable

#data_joined$TARGET=factor(data_joined$TARGET)
print(paste('Dimensions after filtering by LC:',dim(data_joined)))
data_joined=na.omit(data_joined)
print(paste('Dimensions after removing NAs:',dim(data_joined)))

rm(SAR_features,Opt_features, SAR_tempStats)
# print(mem_used())

#define NewIDS corresponding to validation dataset
#example NL
#valid_ID=as.numeric(data_joined[which(data_joined$Teledetect=='J'|data_joined$NVWA_contr=='J'),"NewID"])

#sampling parameters
# sample_size=20000
# set.seed(42)
# count_thresh=30
# count_min=10
# smotesize=50 #number of items to reach (originals + synthetics) for the smote algorithm
# k=5 #number of neighbours for smote algorithm
#
# #main Ranger forest parameters (no modifications allowed here)
# num.trees = 100   ##S2 = 100 #Ranger default=500
# mtry = NULL  ##variables to split (default = sqrt)
# importance = "impurity"  ##corresponds to the Gini index
# write.forest = TRUE  #required for prediction
# min.node.size = 10 #10 for proba, 1 for classif, 5 or 25 for S2
# replace = TRUE #sample with replacement
# verbose = TRUE
# seed = 42
# holdout = FALSE #could use 0 weighted cases for importance and prediction errors
# save.memory = FALSE


###Clean data set

# SAR_features=read.csv("C:/Users/rvanbaalen/Desktop/Netherlands/SAR_features.csv")
# SAR_features=read.csv("/export/miro/rvanbaalen/03_Netherlands/SAR_features.csv")
# SHP_NL=shapefile("C:/Users/rvanbaalen/Desktop/Netherlands/Percelen_BBR_2017_Sen4CAP_tiled_31UFU_UTM_reclass_over5000_buffer15m_minus0.shp")
# SHP_NL=SHP_NL@data
# SHP_NL$NewID=as.numeric(SHP_NL$NewID)
# extract_df=data.frame(extract)
# colnames(extract_df)[1]="NewID"
# print(mem_used())

# create calibration and validation datasets
print('Internal validation')
set.seed(42)
trainindex=createDataPartition(data_joined$TARGET, times = 1, p = training_ratio, list = FALSE)
data_valid=data_joined[-trainindex,]
data_calib=data_joined[trainindex,]
print(dim(data_valid))
print(dim(data_calib))

print(paste('Dimensions data_calib:',dim(data_calib)))
print(paste('Dimensions data_valid:',dim(data_valid)))

if(sample_size==0) sample_size=nrow(data_calib) else sample_size=sample_size
if(sample_size>0 & sample_size<=1) sample_size=round(sample_size*nrow(data_joined)) else sample_size=sample_size
print(sample_size)

# remove non-features columns
#print(names(data_calib))
#data_calib_red <- data_calib[c(1, 5:ncol(data_calib), 3)]
data_calib_red <- data_calib %>% dplyr:: select(union(starts_with("NewID"),union(starts_with("XX"),starts_with("TARGET"))))
# print(mem_used())

#data=cbind(cid=data_calib$CROPDIVNUM,data)
#data=subset(data_calib,select=-c(X,NewID,fid))

##sample data

#summarize stats for the all dataset and the calibration dataset

if ( samplingmethod=="Random") {
  set.seed(42)
  sample_temp=sample_n(data_calib,sample_size)
  sample_ID=as.numeric(sample_temp$NewID)
  data_calib_red=data_calib_red[which(data_calib$NewID %in% sample_ID),]

  write.csv(sample_ID,paste0(workdir,paste("Calib_NewIDs",sample_size,format(Sys.time(), "%m%d-%H%M"),sep="_"),".csv"))


  } else if ( samplingmethod=="Areaweighted") {
    Sample_areaweighted=NA

    grp_data=group_by(data_joined,TARGET)
    nrow=nrow(data_joined)
    areatot=sum(as.numeric(data_joined$AREA),na.rm=TRUE)
    Declarations_summary = summarise(grp_data, count=n(),area=sum(as.numeric(AREA)),arearatio=sum(as.numeric(AREA))/areatot, countratio=n()/nrow)

    grp_data=group_by(data_calib,TARGET)
    nrow=nrow(data_calib)
    areatot=sum(as.numeric(data_calib$AREA),na.rm=TRUE)
    Declarations_summary_calib = summarise(grp_data, count=n(),area=sum(as.numeric(AREA)),arearatio=sum(as.numeric(AREA))/areatot, countratio=n()/nrow)

    Declarations_summary_join=left_join(Declarations_summary_calib,Declarations_summary,by="TARGET",suffix = c(".calib",""))


    sample_size_area=round(sample_size*Declarations_summary_join$arearatio)

    sample_size_area_corr=ifelse(sample_size_area<Declarations_summary_join$count.calib,sample_size_area,Declarations_summary_join$count.calib)

    Sample_areaweighted=data.frame(matrix(nrow=0, ncol=ncol(data_calib)))
    for (i in 1:nrow(Declarations_summary_join)){
      set.seed(42)
      temp=sample_n(data_calib[which(data_calib$TARGET==Declarations_summary_join$TARGET[i]),],sample_size_area_corr[i])
      Sample_areaweighted=rbind(Sample_areaweighted,temp)
    }

    sample_ID=as.numeric(Sample_areaweighted$NewID)
    data_calib_red=data_calib_red[which(data_calib$NewID %in% sample_ID),]

    write.csv(sample_ID,paste0(workdir,paste("Calib_NewIDs",sample_size,format(Sys.time(), "%m%d-%H%M"),sep="_"),".csv"))

  } else if ( samplingmethod=="Overunderareaweighted") {
    grp_data=group_by(data_joined,TARGET)
    nrow=nrow(data_joined)
    areatot=sum(as.numeric(data_joined$AREA),na.rm=TRUE)
    Declarations_summary = summarise(grp_data, count=n(),area=sum(as.numeric(AREA)),arearatio=sum(as.numeric(AREA))/areatot, countratio=n()/nrow)

    grp_data=group_by(data_calib,TARGET)
    nrow=nrow(data_calib)
    areatot=sum(as.numeric(data_calib$AREA),na.rm=TRUE)
    Declarations_summary_calib = summarise(grp_data, count=n(),area=sum(as.numeric(AREA)),arearatio=sum(as.numeric(AREA))/areatot, countratio=n()/nrow)

    Declarations_summary_join=left_join(Declarations_summary_calib,Declarations_summary,by="TARGET",suffix = c(".calib",""))


    index=which(Declarations_summary_join$count.calib>count_min)
    Declarations_summary_join=Declarations_summary_join[index,]

    index_sample=which((Declarations_summary_join$arearatio*sample_size)<count_thresh)
    index_count=which(Declarations_summary_join$count.calib<count_thresh)

    index=union(index_sample,index_count)

    index2=which((Declarations_summary_join$arearatio*sample_size)>Declarations_summary_join$count.calib)

    temp_area=sum(Declarations_summary_join$area[-index])
    temp_count=ifelse(Declarations_summary_join$count.calib[index]>count_thresh,count_thresh,Declarations_summary_join$count.calib[index])
    temp_size=sample_size-sum(temp_count)

    factor=sum(Declarations_summary_join$area/temp_area)
    sample_size_area_overunder=floor(temp_size*Declarations_summary_join$arearatio*factor)
    sample_size_area_overunder[index]=temp_count
    sample_size_area_overunder[setdiff(index2,index)]=Declarations_summary_join$count.calib[setdiff(index2,index)]

    sample_size_area=round(sample_size*Declarations_summary_join$arearatio)

    sample_size_area_over=ifelse(sample_size_area>sample_size_area_overunder,sample_size_area,sample_size_area_overunder)
    sample_size_area_over[setdiff(index2,index)]=Declarations_summary_join$count.calib[setdiff(index2,index)]

    sample_size_area_over=ifelse(sample_size_area_over>Declarations_summary_join$count.calib,Declarations_summary_join$count.calib,sample_size_area_over)

    Sample_areaweighted=NA
    for (i in 1:nrow(Declarations_summary_join)){
      set.seed(42)
      temp=sample_n(data_calib[which(data_calib$TARGET==Declarations_summary_join$TARGET[i]),],sample_size_area_overunder[i])
      Sample_areaweighted=rbind(Sample_areaweighted,temp)
    }

    sample_ID=as.numeric(Sample_areaweighted$NewID)
    data_calib_red=data_calib_red[which(data_calib$NewID %in% sample_ID),]

    write.csv(sample_ID,paste0(workdir,paste("Calib_NewIDs",sample_size,format(Sys.time(), "%m%d-%H%M"),sep="_"),".csv"))



  } else if ( samplingmethod=="Overareaweighted") {
    grp_data=group_by(data_joined,TARGET)
    nrow=nrow(data_joined)
    areatot=sum(as.numeric(data_joined$AREA),na.rm=TRUE)
    Declarations_summary = summarise(grp_data, count=n(),area=sum(as.numeric(AREA)),arearatio=sum(as.numeric(AREA))/areatot, countratio=n()/nrow)

    grp_data=group_by(data_calib,TARGET)
    nrow=nrow(data_calib)
    areatot=sum(as.numeric(data_calib$AREA),na.rm=TRUE)
    Declarations_summary_calib = summarise(grp_data, count=n(),area=sum(as.numeric(AREA)),arearatio=sum(as.numeric(AREA))/areatot, countratio=n()/nrow)

    Declarations_summary_join=left_join(Declarations_summary_calib,Declarations_summary,by="TARGET",suffix = c(".calib",""))


    index=which(Declarations_summary_join$count.calib>count_min)
    Declarations_summary_join=Declarations_summary_join[index,]

    index_sample=which((Declarations_summary_join$arearatio*sample_size)<count_thresh)
    index_count=which(Declarations_summary_join$count.calib<count_thresh)

    index=union(index_sample,index_count)

    index2=which((Declarations_summary_join$arearatio*sample_size)>Declarations_summary_join$count.calib)

    temp_area=sum(Declarations_summary_join$area[-index])
    temp_count=ifelse(Declarations_summary_join$count.calib[index]>count_thresh,count_thresh,Declarations_summary_join$count.calib[index])
    temp_size=sample_size-sum(temp_count)


    factor=sum(Declarations_summary_join$area/temp_area)
    sample_size_area_overunder=floor(temp_size*Declarations_summary_join$arearatio*factor)
    sample_size_area_overunder[index]=temp_count
    sample_size_area_overunder[setdiff(index2,index)]=Declarations_summary_join$count.calib[setdiff(index2,index)]

    sample_size_area=round(sample_size*Declarations_summary_join$arearatio)

    sample_size_area_over=ifelse(sample_size_area>sample_size_area_overunder,sample_size_area,sample_size_area_overunder)
    sample_size_area_over[setdiff(index2,index)]=Declarations_summary_join$count.calib[setdiff(index2,index)]

    sample_size_area_over=ifelse(sample_size_area_over>Declarations_summary_join$count.calib,Declarations_summary_join$count.calib,sample_size_area_over)

    Sample_areaweighted=NA
        for (i in 1:nrow(Declarations_summary_join)){
      set.seed(42)
      temp=sample_n(data_calib[which(data_calib$TARGET==Declarations_summary_join$TARGET[i]),],sample_size_area_over[i])
      Sample_areaweighted=rbind(Sample_areaweighted,temp)
    }

    sample_ID=as.numeric(Sample_areaweighted$NewID)
    data_calib_red=data_calib_red[which(data_calib$NewID %in% sample_ID),]

    write.csv(sample_ID,paste0(workdir,paste("Calib_NewIDs",sample_size,format(Sys.time(),"%m%d-%H%M"),sep="_"),".csv"))



  } else if ( samplingmethod=="Smote") {
    print("Starting Smote")

    grp_data=group_by(data_joined,TARGET)
    nrow=nrow(data_joined)
    areatot=sum(as.numeric(data_joined$AREA),na.rm=TRUE)
    Declarations_summary = summarise(grp_data, count=n(),area=sum(as.numeric(AREA)),arearatio=sum(as.numeric(AREA))/areatot, countratio=n()/nrow)

    grp_data=group_by(data_calib,TARGET)
    nrow=nrow(data_calib)
    areatot=sum(as.numeric(data_calib$AREA),na.rm=TRUE)
    Declarations_summary_calib = summarise(grp_data, count=n(),area=sum(as.numeric(AREA)),arearatio=sum(as.numeric(AREA))/areatot, countratio=n()/nrow)

    Declarations_summary_join=left_join(Declarations_summary_calib,Declarations_summary,by="TARGET",suffix = c(".calib",""))


    Smoted_data=NA
    index=which(Declarations_summary_join$count.calib>count_min)
    Declarations_summary_join=Declarations_summary_join[index,]

    for (i in 1:nrow(Declarations_summary_join)){

      SMOTEd=NA
      Sample_areaweighted=NA
      print(paste("Iteration",i,"on",nrow(Declarations_summary_join)))
      data_calib_red$test=ifelse(data_calib$TARGET==Declarations_summary_join$TARGET[i],1,0)
      dupsize=smotesize/length(which(data_calib$TARGET==Declarations_summary_join$TARGET[i]))
      #dupsize=ifelse(dupsize<1,1,dupsize)
      #nums <- sapply(data_calib_red, is.numeric)
      # data_smote=data_calib_red[,which(nums)]
      #data_smote <- data_calib_red %>% dplyr:: select(union(starts_with("X"),starts_with("test")))
      #data_smote = cbind(TARGET=data_calib[,"TARGET"],data_smote)
      #data_smote = na.omit(data_calib_red)
      #data_smote[,c("TARGET")]=as.numeric(as.character(data_smote[,c("TARGET")]))
      #data_smote[,!(names(data_smote) %in% c("TARGET"))]
      data_smote <- data_calib_red

      if ( dupsize>1) {

      SMOTEd=SMOTE(data_smote[,!(names(data_smote) %in% c("test","TARGET"))],as.numeric(data_smote[,c("test")]),K=k,dup_size=dupsize)

      originals=SMOTEd$orig_P
      synthetics=SMOTEd$syn_data
      rm(SMOTEd)
      originals$SMOTE=1
      synthetics$SMOTE=0

      synthetics$TARGET=Declarations_summary_join$TARGET[i]
      originals$TARGET=Declarations_summary_join$TARGET[i]
      Smoted_data=rbind(Smoted_data,originals,synthetics)
      } else

      originals=data_smote[which(data_smote$TARGET==Declarations_summary_join$TARGET[i]),]
      colnames(originals)[which(names(originals) == "test")] <- "class"
      #originals$class=NA
      #synthetics=NA
      originals$SMOTE=1
      #synthetics$SMOTE=0
      #synthetics$TARGET=originals$TARGET[1]
      Smoted_data=rbind(Smoted_data,originals)
    }
    rm(data_smote)


    grp_data=group_by(data_joined,TARGET)
    nrow=nrow(data_joined)
    areatot=sum(as.numeric(data_joined$AREA),na.rm=TRUE)
    Declarations_summary = summarise(grp_data, count=n(),area=sum(as.numeric(AREA)),arearatio=sum(as.numeric(AREA))/areatot, countratio=n()/nrow)

    grp_data=group_by(data_calib,TARGET)
    nrow=nrow(data_calib)
    areatot=sum(as.numeric(data_calib$AREA),na.rm=TRUE)
    Declarations_summary_calib = summarise(grp_data, count=n(),area=sum(as.numeric(AREA)),arearatio=sum(as.numeric(AREA))/areatot, countratio=n()/nrow)

    Declarations_summary_join=left_join(Declarations_summary_calib,Declarations_summary,by="TARGET",suffix = c(".calib",""))


    index=which(Declarations_summary_join$count.calib>count_min)
    Declarations_summary_join=Declarations_summary_join[index,]

    index_sample=which((Declarations_summary_join$arearatio*sample_size)<count_thresh)
    index_count=which(Declarations_summary_join$count.calib<count_thresh)

    index=union(index_sample,index_count)

    index2=which((Declarations_summary_join$arearatio*sample_size)>Declarations_summary_join$count.calib)

    temp_area=sum(Declarations_summary_join$area[-index])
    temp_count=ifelse(Declarations_summary_join$count.calib[index]>count_thresh,count_thresh,Declarations_summary_join$count.calib[index])
    temp_size=sample_size-sum(temp_count)


    factor=sum(Declarations_summary_join$area/temp_area)
    sample_size_area_overunder=floor(temp_size*Declarations_summary_join$arearatio*factor)
    sample_size_area_overunder[index]=temp_count
    sample_size_area_overunder[setdiff(index2,index)]=Declarations_summary_join$count.calib[setdiff(index2,index)]

    sample_size_area=round(sample_size*Declarations_summary_join$arearatio)

    sample_size_area_over=ifelse(sample_size_area>sample_size_area_overunder,sample_size_area,sample_size_area_overunder)
    sample_size_area_over[setdiff(index2,index)]=Declarations_summary_join$count.calib[setdiff(index2,index)]

    sample_size_area_over=ifelse(sample_size_area_over>Declarations_summary_join$count.calib,Declarations_summary_join$count.calib,sample_size_area_over)

    ###Test for undersampling
    #sample_size_area_over=ifelse(sample_size_area_over>count_thresh*4,count_thresh*4,sample_size_area_over)
    #sample_size_area_over=ifelse(Declarations_summary_join$count.calib>3*count_thresh,3*count_thresh,Declarations_summary_join$count.calib)
    #print(sample_size_area_over)


        Sample_areaweighted_originals=NA
    for (i in 1:nrow(Declarations_summary_join)){
      set.seed(42)
      temp=sample_n(Smoted_data[which(Smoted_data$TARGET==Declarations_summary_join$TARGET[i]& Smoted_data$SMOTE==1),],sample_size_area_over[i])
      Sample_areaweighted_originals=rbind(Sample_areaweighted_originals,temp)
    }



    sample_size_diff=ifelse(sample_size_area_over<count_thresh,count_thresh,sample_size_area_over)-sample_size_area_over
    #sample_size_diff[setdiff(index2,index)]=sample_size_area[setdiff(index2,index)]-Declarations_summary_join$count.calib[setdiff(index2,index)]
    print(sample_size_diff)

    Sample_areaweighted_smoted=matrix(data=NA,nrow=0,ncol=ncol(Smoted_data))
    colnames(Sample_areaweighted_smoted) <-names(Smoted_data)
    for (i in 1:nrow(Declarations_summary_join)){
      set.seed(42)
      temp=sample_n(Smoted_data[which(Smoted_data$TARGET==Declarations_summary_join$TARGET[i]& Smoted_data$SMOTE==0),],sample_size_diff[i])
      Sample_areaweighted_smoted=rbind(Sample_areaweighted_smoted,temp)
    }

    Sample_SMOTE=rbind( na.omit(Sample_areaweighted_originals), na.omit(Sample_areaweighted_smoted))
    Sample_SMOTE$TARGET=as.factor(Sample_SMOTE$TARGET)
    rm(Sample_areaweighted_originals, Sample_areaweighted_smoted, Smoted_data)

    ind=which(round(Sample_SMOTE$NewID)==Sample_SMOTE$NewID)
    NewIDs=Sample_SMOTE$NewID[ind]
    write.csv(NewIDs,paste0(workdir,paste("Calib_NewIDs",sample_size,format(Sys.time(),"%m%d-%H%M"),sep="_"),".csv"))
    data_calib_red=Sample_SMOTE
    rm(Sample_SMOTE)
  } else if ( samplingmethod=="Fixunder") {
    print("Starting Fixed oversampling")

    grp_data=group_by(data_joined,TARGET)
    nrow=nrow(data_joined)
    areatot=sum(as.numeric(data_joined$AREA),na.rm=TRUE)
    Declarations_summary = summarise(grp_data, count=n(),area=sum(as.numeric(AREA)),arearatio=sum(as.numeric(AREA))/areatot, countratio=n()/nrow)

    grp_data=group_by(data_calib,TARGET)
    nrow=nrow(data_calib)
    areatot=sum(as.numeric(data_calib$AREA),na.rm=TRUE)
    Declarations_summary_calib = summarise(grp_data, count=n(),area=sum(as.numeric(AREA)),arearatio=sum(as.numeric(AREA))/areatot, countratio=n()/nrow)

    Declarations_summary_join=left_join(Declarations_summary_calib,Declarations_summary,by="TARGET",suffix = c(".calib",""))


    Smoted_data=NA
    index=which(Declarations_summary_join$count.calib>count_min)
    Declarations_summary_join=Declarations_summary_join[index,]

    for (i in 1:nrow(Declarations_summary_join)){

      SMOTEd=NA
      Sample_areaweighted=NA
      print(paste("Iteration",i,"on",nrow(Declarations_summary_join)))
      data_calib_red$test=ifelse(data_calib$TARGET==Declarations_summary_join$TARGET[i],1,0)
      dupsize=smotesize/length(which(data_calib$TARGET==Declarations_summary_join$TARGET[i]))
      #dupsize=ifelse(dupsize<1,1,dupsize)
      #nums <- sapply(data_calib_red, is.numeric)
      # data_smote=data_calib_red[,which(nums)]
      #data_smote <- data_calib_red %>% dplyr:: select(union(starts_with("X"),starts_with("test")))
      #data_smote = cbind(TARGET=data_calib[,"TARGET"],data_smote)
      #data_smote = na.omit(data_calib_red)
      #data_smote[,c("TARGET")]=as.numeric(as.character(data_smote[,c("TARGET")]))
      #data_smote[,!(names(data_smote) %in% c("TARGET"))]
      data_smote <- data_calib_red

      if ( dupsize>1) {

        SMOTEd=SMOTE(data_smote[,!(names(data_smote) %in% c("test","TARGET"))],as.numeric(data_smote[,c("test")]),K=k,dup_size=dupsize)

        originals=SMOTEd$orig_P
        synthetics=SMOTEd$syn_data
        rm(SMOTEd)
        originals$SMOTE=1
        synthetics$SMOTE=0

        synthetics$TARGET=Declarations_summary_join$TARGET[i]
        originals$TARGET=Declarations_summary_join$TARGET[i]
        Smoted_data=rbind(Smoted_data,originals,synthetics)
      } else

        originals=data_smote[which(data_smote$TARGET==Declarations_summary_join$TARGET[i]),]
      colnames(originals)[which(names(originals) == "test")] <- "class"
      #originals$class=NA
      #synthetics=NA
      originals$SMOTE=1
      #synthetics$SMOTE=0
      #synthetics$TARGET=originals$TARGET[1]
      Smoted_data=rbind(Smoted_data,originals)
    }
    rm(data_smote)


    grp_data=group_by(data_joined,TARGET)
    nrow=nrow(data_joined)
    areatot=sum(as.numeric(data_joined$AREA),na.rm=TRUE)
    Declarations_summary = summarise(grp_data, count=n(),area=sum(as.numeric(AREA)),arearatio=sum(as.numeric(AREA))/areatot, countratio=n()/nrow)

    grp_data=group_by(data_/calib,TARGET)
    nrow=nrow(data_calib)
    areatot=sum(as.numeric(data_calib$AREA),na.rm=TRUE)
    Declarations_summary_calib = summarise(grp_data, count=n(),area=sum(as.numeric(AREA)),arearatio=sum(as.numeric(AREA))/areatot, countratio=n()/nrow)

    Declarations_summary_join=left_join(Declarations_summary_calib,Declarations_summary,by="TARGET",suffix = c(".calib",""))


    index=which(Declarations_summary_join$count.calib>count_min)
    Declarations_summary_join=Declarations_summary_join[index,]

    index_sample=which((Declarations_summary_join$arearatio*sample_size)<count_thresh)
    index_count=which(Declarations_summary_join$count.calib<count_thresh)

    index=union(index_sample,index_count)

    index2=which((Declarations_summary_join$arearatio*sample_size)>Declarations_summary_join$count.calib)

    temp_area=sum(as.numeric(Declarations_summary_join$area[-index]))
    temp_count=ifelse(Declarations_summary_join$count.calib[index]>count_thresh,count_thresh,Declarations_summary_join$count.calib[index])
    temp_size=sample_size-sum(temp_count)


    factor=sum(as.numeric(Declarations_summary_join$area)/temp_area)
    sample_size_area_overunder=floor(temp_size*Declarations_summary_join$arearatio*factor)
    sample_size_area_overunder[index]=temp_count
    sample_size_area_overunder[setdiff(index2,index)]=Declarations_summary_join$count.calib[setdiff(index2,index)]

    sample_size_area=round(sample_size*Declarations_summary_join$arearatio)

    sample_size_area_over=ifelse(sample_size_area>sample_size_area_overunder,sample_size_area,sample_size_area_overunder)
    sample_size_area_over[setdiff(index2,index)]=Declarations_summary_join$count.calib[setdiff(index2,index)]

    sample_size_area_over=ifelse(sample_size_area_over>Declarations_summary_join$count.calib,Declarations_summary_join$count.calib,sample_size_area_over)

    ##undersampling
    sample_size_area_over=ifelse(sample_size_area_over>count_thresh,count_thresh,sample_size_area_over)
    print(sample_size_area_over)


    Sample_areaweighted_originals=NA
    for (i in 1:nrow(Declarations_summary_join)){
      set.seed(42)
      temp=sample_n(Smoted_data[which(Smoted_data$TARGET==Declarations_summary_join$TARGET[i]& Smoted_data$SMOTE==1),],sample_size_area_over[i])
      Sample_areaweighted_originals=rbind(Sample_areaweighted_originals,temp)
    }



    sample_size_diff=ifelse(sample_size_area_over<count_thresh,count_thresh,sample_size_area_over)-sample_size_area_over
    #sample_size_diff[setdiff(index2,index)]=sample_size_area[setdiff(index2,index)]-Declarations_summary_join$count.calib[setdiff(index2,index)]
    print(sample_size_diff)

    Sample_areaweighted_smoted=matrix(data=NA,nrow=0,ncol=ncol(Smoted_data))
    colnames(Sample_areaweighted_smoted) <-names(Smoted_data)
    for (i in 1:nrow(Declarations_summary_join)){
      set.seed(42)
      temp=sample_n(Smoted_data[which(Smoted_data$TARGET==Declarations_summary_join$TARGET[i]& Smoted_data$SMOTE==0),],sample_size_diff[i])
      Sample_areaweighted_smoted=rbind(Sample_areaweighted_smoted,temp)
    }

    Sample_SMOTE=rbind( na.omit(Sample_areaweighted_originals), na.omit(Sample_areaweighted_smoted))
    rm(Sample_areaweighted_originals, Sample_areaweighted_smoted, Smoted_data)
    Sample_SMOTE$TARGET=as.factor(Sample_SMOTE$TARGET)


    ind=which(round(Sample_SMOTE$NewID)==Sample_SMOTE$NewID)
    NewIDs=Sample_SMOTE$NewID[ind]
    write.csv(NewIDs,paste0(workdir,paste("Calib_NewIDs",sample_size,format(Sys.time(), "%m%d-%H%M"),sep="_"),".csv"))
    data_calib_red=Sample_SMOTE
    rm(Sample_SMOTE)

  } else
  print("Sampling not properly defined")

rm(data_calib)
gc()

parcels_predict = data.frame(data_joined$NewID, data_joined$AREA, data_joined$TARGET)
colnames(parcels_predict)=c("NewID", "AREA", "TARGET")
parcels_predict_name = paste("Parcels_predict", format(Sys.time(), "%m%d-%H%M"), sep = "_")
saveRDS(parcels_predict, paste0(workdir, parcels_predict_name, ".rds"))
rm(parcels_predict)

# remove non-features columns
data_predict_red <- data_joined %>% dplyr:: select(starts_with("XX"))
data_predict_red_name = paste("Data_predict", sample_size, format(Sys.time(), "%m%d-%H%M"), sep="_")
saveRDS(data_predict_red, paste0(workdir,data_predict_red_name,".rds"))
rm(data_joined, data_predict_red)
gc()

# remove NULL and NA values (see above)
#data_calib_red=na.omit(data_calib_red)
#data_red=data_red[,-2]

#validation set
#data_valid_red <- data_valid[c(-1, -2, -4)]
data_valid_red <- data_valid %>% dplyr:: select(union(starts_with("TARGET"),starts_with("XX")))

if(samplingmethod!="Random"){
  grp_data=group_by(data_valid_red,TARGET)
  Declarations_summary = summarise(grp_data, count=n())
  index=Declarations_summary$TARGET[which(Declarations_summary$count>ceiling(count_min/3))]
  data_valid_red=data_valid_red[which(data_valid_red$TARGET %in% index),]
}

###classification
#data_model <- data_calib_red[c(ncol(data_calib_red), 2:(ncol(data_calib_red) - 3))]
data_model <- data_calib_red %>% dplyr:: select(union(starts_with("TARGET"),starts_with("XX")))
rm(data_calib_red)

#print(mem_used())
grp_data=group_by(data_valid,TARGET)
areatot=sum(as.numeric(data_valid$AREA))
Declarations_summary = summarise(grp_data, count=n(),arearatio=sum(as.numeric(AREA))/areatot)
rm(data_valid, grp_data)

#print(mem_used())
print(paste('Dimensions sample for training:',dim(data_model)))
print(paste('Dimensions sample for validation:',dim(data_valid_red)))

data_valid_red_name = paste("Data_valid", sample_size, format(Sys.time(), "%m%d-%H%M"), sep="_")
saveRDS(data_valid_red, paste0(workdir, data_valid_red_name, ".rds"))
rm(data_valid_red)
gc()

# grp_data=group_by(data_model,TARGET)
# Declarations_summary = summarise(grp_data, count=n())
# countmax=max(Declarations_summary$count)
# Declarations_summary$countweight=countmax/Declarations_summary$count
#
# weights=matrix(data=NA,nrow=nrow(data_model),ncol=1)
# for (i in 1:nrow(Declarations_summary)){
#   set.seed(42)
#   weights[which(data_model$TARGET==Declarations_summary$TARGET[i])]=countmax/Declarations_summary$count[i]
# }

##Launch model
print("... Start training using Ranger script")

start.time <- Sys.time()
Ranger_trees = ranger(TARGET ~ ., data = droplevels(data_model), write.forest = TRUE,probability = TRUE,num.trees = numtrees,mtry = NULL,importance = "impurity",min.node.size = 10,seed = 42)
#Ranger_trees = ranger(TARGET ~ ., data = droplevels(data_model), case.weights=weights, write.forest = TRUE,probability = TRUE,num.trees = numtrees,mtry = NULL,importance = "impurity",min.node.size = 10,seed = 42)

end.time <- Sys.time()
time.taken <- end.time - start.time
time.taken
rm(data_model)
gc()
Ranger_trees$time.taken=time.taken
Ranger_trees$inputs=list(variable)
print(Ranger_trees)


#Save model
modelname=paste("Ranger",sample_size,format(Sys.time(), "%m%d-%H%M"),sep="_")
saveRDS(Ranger_trees, paste(workdir,modelname,".rds",sep=""))

###Prediction
data_valid_red <- readRDS(paste0(workdir, data_valid_red_name, ".rds"))

start.time <- Sys.time()
predict_Ranger_trees=predict(Ranger_trees,data_valid_red)
end.time <- Sys.time()
time.taken <- end.time - start.time
time.taken

#save predictions
predictname=paste("Predict_Ranger",sample_size,format(Sys.time(), "%m%d-%H%M"),sep="_")
print(paste(workdir,predictname,".rds",sep=""))
saveRDS(predict_Ranger_trees, paste(workdir,predictname,".rds",sep=""))


###Validation and confusion matrix
predictions=predict_Ranger_trees$predictions

predict.max=apply(predictions, 1, max)
predict.whichmax=apply(predictions, 1, which.max)

predict.class=factor(colnames(predictions)[predict.whichmax])
data_ref=factor(data_valid_red$TARGET)
lvl = union(levels(predict.class), levels(data_ref))
predict.class = factor(predict.class, levels = lvl)
data_ref = factor(data_ref, levels = lvl)

#data_ref=data_ref[data_ref %in% levels(data_predict)]
#data_predict=data_predict[data_predict %in% levels(data_ref)]
#data_predict=data_predict[1:length(data_ref)]

Results_Ranger=confusionMatrix(predict.class,data_ref,mode="everything")
resultname=paste("Results_Ranger",sample_size,samplingmethod,count_thresh,numtrees,format(Sys.time(), "%m%d-%H%M"),sep="_")
saveRDS(Results_Ranger, paste(workdir,resultname,".rds",sep=""))
print(Results_Ranger$overall)

rm(data_valid_red)
gc()

metrics_name=paste("Metrics",sample_size,samplingmethod,count_thresh,numtrees,format(Sys.time(), "%m%d-%H%M"),sep="_")
write.csv(t(Results_Ranger$overall), file = paste0(workdir, metrics_name, ".csv"), row.names = FALSE, quote = FALSE)

df <- data.frame(x=factor(c("Overall accuracy","Kappa",names(Results_Ranger$byClass[,"F1"])),levels=(c("Overall accuracy","Kappa",names(Results_Ranger$byClass[order(Results_Ranger$byClass[,"Prevalence"],decreasing = TRUE),"F1"])))),y=c(Results_Ranger$overall[1],Results_Ranger$overall[2],Results_Ranger$byClass[,"F1"]),prevalence=c(Results_Ranger$overall[1],Results_Ranger$overall[2],Results_Ranger$byClass[,"Prevalence"]))
index=which(df$prevalence!=0)
df=df[index,]

df$x=as.character(df$x)
df$x[3:nrow(df)]=strapplyc(df$x[3:nrow(df)], "[0-9]{1,4}",simplify=TRUE)

df$area=Declarations_summary$arearatio[match(as.character(df$x), as.character(Declarations_summary$TARGET))]
df$x=factor(df$x,levels=c(df$x[1],df$x[2],df$x[3:length(df$x)][order(df$area[3:length(df$x)],decreasing=TRUE)]))

df$cumarea=0
df$cumarea[3:length(df$area)][order(df$x[3:length(df$area)])]=cumsum(df$area[3:length(df$area)][order(df$x[3:length(df$area)])])
colors=c('Overall accuracy','Kappa',rep('F-score',nrow(df)-2))

p<-ggplot(data=df) +
  geom_bar(aes(x=x, y=y,fill=colors),width=.5,stat="identity", position="dodge") + geom_point(aes(x=x,y=cumarea),size=1) + scale_y_continuous(limits=c(0,1),breaks=seq(0,1,0.1),expand = c(0, 0))
q<- p + theme_light() + theme(axis.text.x = element_text(angle = 90, hjust = 1,vjust=0.5)) + xlab("") + ylab("Performance measure or Relative cumulated area (dots)") + scale_fill_discrete("")
ggsave(paste(workdir,"Plot_",resultname,".png",sep=""),q,width = 13, height = 8)

data_predict_red <- readRDS(paste0(workdir,data_predict_red_name,".rds"))

print("Start prediction...")
start.time <- Sys.time()
predict_Ranger_trees=predict(Ranger_trees,data_predict_red)
end.time <- Sys.time()
time.taken <- end.time - start.time
time.taken

predictions=predict_Ranger_trees$predictions
print("after prediction")
rm(Ranger_trees, data_predict_red)
gc()

predict.max=apply(predictions, 1, max)
predict.whichmax=apply(predictions, 1, which.max)
predict.class=colnames(predictions)[predict.whichmax]

n <- ncol(predictions)
predict.2max=apply(predictions, 1, function(x) sort(x,partial=n-1)[n-1])
predict.which2max=apply(predictions, 1, function(x) which(x==sort(x,partial=n-1)[n-1])[1])

predict.2class=colnames(predictions)[predict.which2max]

predict.2class=ifelse(predict.class==predict.2class,colnames(predictions)[apply(predictions, 1, function(x) which(x==sort(x,partial=n-1)[n-1])[2])],predict.2class)

parcels_predict <- readRDS(paste0(workdir, parcels_predict_name, ".rds"))
Predict_classif=data.frame(parcels_predict$NewID,parcels_predict$AREA,parcels_predict$TARGET,predict.class,round(predict.max,digits=3),predict.2class,round(predict.2max,digits=3))
colnames(Predict_classif)=c("NewID",'Area','CT_decl','CT_pred_1','CT_conf_1','CT_pred_2','CT_conf_2')

#save predictions
predictname=paste("Predict_classif",modelname,sep="_")
write.csv(Predict_classif,paste0(workdir,predictname,".csv"), row.names = FALSE)
