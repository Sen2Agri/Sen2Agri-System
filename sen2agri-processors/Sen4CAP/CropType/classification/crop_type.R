#!/usr/bin/env Rscript
#
# requiredPackages = c("ranger","dplyr","e1071","caret","smotefamily","readr","gsubfn","caTools","tidyverse","data.table")
#
# for(p in requiredPackages){
#   if(!require(p,character.only = TRUE)) install.packages(p)
#   library(p,character.only = TRUE)
# }
#

library(ranger)
library(dplyr)
library(e1071)
library(caret)
library(smotefamily)
library(readr)
library(gsubfn)
library(caTools)
library(tidyverse)
library(data.table)

## Implement variables

variable <- commandArgs(trailingOnly=TRUE)
print(variable)
workdir <- variable[1]  #transform to numeric !!
InputSAR <- variable[2]
InputOpt <- variable[3]
InputOptRe <- variable[4]
InputSAR_tempStats <- variable[5]
Shape_filename <- variable[6]
CTnumL4A <- variable[7]
LC_monitored <- variable[8]
AreaDeclared <- variable[9]
S2pixMIN <- as.numeric(variable[10])
S1pixMIN <- as.numeric(variable[11])
PaMIN <- as.numeric(variable[12])
S2pixBEST <- as.numeric(variable[13])
PaCalibH <- as.numeric(variable[14])
PaCalibL <- as.numeric(variable[15])
Sample_ratioH <- as.numeric(variable[16])
Sample_ratioL <- as.numeric(variable[17])
samplingmethod <-variable[18]
smotesize <- as.numeric(variable[19])
k <- as.numeric(variable[20])
numtrees <- as.numeric(variable[21])
min_node_size <- as.numeric(variable[22])
LUT <-variable[23]

## Check if parameteres correctly defined: LC and sampling method

# LC

if (!(LC_monitored %in% c(1,2,3,4,5,0,"All","12","123","1234"))) {
  stop("Wrong argument: choose out of (1,2,3,4,5,0,'All','12','123','1234')", call.=FALSE)
}

if(LC_monitored=="All") landcover=c(0,1,2,3,4,5)
if(LC_monitored=="1234") landcover=c(1,2,3,4)
if(LC_monitored=="123") landcover=c(1,2,3)
if(LC_monitored=="12") landcover=c(1,2)

print(paste0("Monitored land cover classes are:",landcover))

# Sampling method

if (!(samplingmethod %in% c("Random","Areaweighted","Overareaweighted","Overunderareaweighted","Smote","Fixunder"))) {
  stop('Wrong argument: choose out of "Random","Areaweighted","Overareaweighted","Overunderareaweighted","Smote","Fixunder"', call.=FALSE)
}

## Import declaration dataset and LUT as csv + JOIN

print("Importing declaration dataset")

Shapefile<-read_csv(Shape_filename,col_names = TRUE,col_types= cols(NewID = 'i',Area_meters = 'd',CTnumL4A = 'i',CTL4A = 'c', LC = 'i',S1Pix = 'i',S2Pix = 'i'))
LUT<-read_csv(LUT,col_types=cols(CTL4A = 'c', CTnumL4A = 'i'))

## Trajectory and Purpose flags (old but could be reactivated)
#
# Shapefile$Trajectory <-0
# Shapefile$Purpose <-0

names(Shapefile)[which(names(Shapefile)==CTnumL4A)]="CTnumL4A"
names(Shapefile)[which(names(Shapefile)==AreaDeclared)]="AreaDeclared"

## Join the optical and SAR data to declaration dataset

data_joined = Shapefile

if (InputOpt != 0) {
  print("Importing optical features...")
  ncol_Optcsv=system(paste("head -1",InputOpt,"| sed 's/[^,]//g' | wc -c"),intern=TRUE)
  Opt_features=read_csv(InputOpt,col_types=paste0("i",paste(rep("d",as.numeric(ncol_Optcsv)-1),collapse="")))

  data_joined = inner_join(data_joined, Opt_features, by="NewID")
  rm(Opt_features)
}

if (InputOptRe != 0) {
  print("Importing red-edge optical features...")
  ncol_OptRecsv=system(paste("head -1",InputOptRe,"| sed 's/[^,]//g' | wc -c"),intern=TRUE)
  Opt_Re_features=read_csv(InputOptRe,col_types=paste0("i",paste(rep("d",as.numeric(ncol_OptRecsv)-1),collapse="")))

  data_joined = inner_join(data_joined, Opt_Re_features, by="NewID")
  rm(Opt_Re_features)
}

if (InputSAR != 0) {
  print("Importing SAR features...")
  ncol_SARcsv=system(paste("head -1",InputSAR,"| sed 's/[^,]//g' | wc -c"),intern=TRUE)
  SAR_features=read_csv(InputSAR,col_types=paste0("i",paste(rep("d",as.numeric(ncol_SARcsv)-1),collapse="")))

  data_joined = inner_join(data_joined, SAR_features, by="NewID")
  rm(SAR_features)
}

if (InputSAR_tempStats != 0) {
  print("Importing temporal SAR features...")
  ncol_SARcsv=system(paste("head -1",InputSAR_tempStats,"| sed 's/[^,]//g' | wc -c"),intern=TRUE)
  SAR_tempStats=read_csv(InputSAR_tempStats,col_types=paste0("i",paste(rep("d",as.numeric(ncol_SARcsv)-1),collapse="")))

  data_joined = inner_join(data_joined, SAR_tempStats, by="NewID")
  rm(SAR_tempStats)
}

print("Features imported successfully")

data_joined=as.data.frame(data_joined)
print(paste('Dimensions before filtering:',dim(data_joined)))

## Filtering steps

# Filter out data with NAs

data_joined <- data_joined %>% drop_na(CTnumL4A,starts_with("XX"))
print(paste('Dimensions after removing NAs:',dim(data_joined)))

# Filter out not monitored LC classes

data_joined <- subset(data_joined, LC %in% landcover)
print(paste('Dimensions after filtering by LC:',dim(data_joined)))

# Filter out small parcels

data_joined <- subset(data_joined, S2Pix >= S2pixMIN & S1Pix >= S1pixMIN)
print(paste('Dimensions after filtering out small parcels:',dim(data_joined)))

## Filter out crop types with few parcels

# Selection of best parcels for calibration (for the filtering)

data_calib <- subset(data_joined, S2Pix >= S2pixBEST)
print(paste('Dimensions after selecting best parcels (S2Pix >= S2pixBEST) for calibration:',dim(data_calib)))

# Filter out crop types with few parcels

dj2 <- count(data_calib,CTnumL4A)
dj3 <- subset(dj2, n >= PaMIN)
ctlist <- dj3[, "CTnumL4A", drop=TRUE]
print(paste('Crop types after filtering out crop types with few parcels:',ctlist))
data_joined <- subset(data_joined, CTnumL4A %in% ctlist)
print(paste('Dimensions after filtering out crop types with few parcels:',dim(data_joined)))
ctlist2 <- data_joined[, "NewID", drop=TRUE]

## Selection of best parcels for calibration

data_valid1 <- subset(data_joined,S2Pix < S2pixBEST)
print(paste('Dimensions of the first part of the validation dataset with parcels with S2Pix < S2pixBEST:',dim(data_valid1)))

data_calib <- subset(data_joined, S2Pix >= S2pixBEST)
print(paste('Dimensions after selecting best parcels (S2Pix >= S2pixBEST) for calibration:',dim(data_calib)))

## Crop type strategy lists

strategy1 <- subset(dj3, n >= PaCalibH)
strategy1list <- strategy1[, "CTnumL4A", drop=TRUE]
print(paste("Crop types belonging to strategy 1:",strategy1list))

strategy2 <- subset(dj3, n < PaCalibH & n >= PaCalibL )
strategy2list <- strategy2[, "CTnumL4A", drop=TRUE]
print(paste("Crop types belonging to strategy 2:",strategy2list))

strategy3 <- subset(dj3, n < PaCalibL )
strategy3list <- strategy3[, "CTnumL4A", drop=TRUE]
print(paste("Crop types belonging to strategy 3:",strategy3list))

# Save list

crop_type_list=group_by(Shapefile,CTnumL4A)
nrow=nrow(Shapefile)
areatot=sum(as.numeric(Shapefile$AreaDeclared),na.rm=TRUE)
crop_type_list_summary = summarise(crop_type_list,Count=n(),Area=sum(as.numeric(AreaDeclared)),Arearatio=sum(as.numeric(AreaDeclared))/areatot, Countratio=n()/nrow)
rm(crop_type_list)
crop_type_list_summary$Classified <- FALSE
crop_type_list_summary$Strategy <- as.integer(0)
crop_type_list_summary$Classified[crop_type_list_summary$CTnumL4A %in% c(strategy1list,strategy2list,strategy3list)] <-TRUE
crop_type_list_summary$Strategy[crop_type_list_summary$CTnumL4A %in% strategy1list] <-1
crop_type_list_summary$Strategy[crop_type_list_summary$CTnumL4A %in% strategy2list] <-2
crop_type_list_summary$Strategy[crop_type_list_summary$CTnumL4A %in% strategy3list] <-3
join<-Shapefile %>% distinct(CTnumL4A, .keep_all = TRUE)
join<-join %>% select (CTnumL4A,CTL4A)
crop_type_list_summary=left_join(crop_type_list_summary,join,"CTnumL4A")
crop_type_list_summary<-crop_type_list_summary %>% select(CTnumL4A,CTL4A,Count,Countratio,Area,Arearatio,Classified,Strategy)
crop_type_list_summary<-crop_type_list_summary[order(-crop_type_list_summary$Area),]

write.csv(crop_type_list_summary,paste0(workdir,paste("Crop_types_summary",format(Sys.time(),"%m%d_%H%M"),sep="_"), ".csv"), row.names = FALSE)

## Strategy 1

data_strategy1 <- subset(data_calib, CTnumL4A %in% strategy1list)

if (nrow(data_strategy1)==0) {
  data_strategy1_valid=data_strategy1
  data_strategy1_calib=data_strategy1

  print('No crop type in strategy1')

  print(paste('Dimensions data_strategy1_valid:',dim(data_strategy1_valid)))
  print(paste('Dimensions data_strategy1_calib:',dim(data_strategy1_calib)))

} else {

set.seed(42)

trainindex1= sample.split(data_strategy1$CTnumL4A,SplitRatio=Sample_ratioH)
data_strategy1_valid= data_strategy1[!trainindex1,]
data_strategy1_calib= data_strategy1[trainindex1,]

print(paste('Dimensions data_strategy1_valid:',dim(data_strategy1_valid)))
print(paste('Dimensions data_strategy1_calib:',dim(data_strategy1_calib)))

}

## Strategy 2

data_strategy2 <- subset(data_calib, CTnumL4A %in% strategy2list)

if (nrow(data_strategy2)==0) {
  data_strategy2_valid=data_strategy2
  data_strategy2_calib=data_strategy2

  print('No crop type in strategy2')

  print(paste('Dimensions data_strategy2_valid:',dim(data_strategy2_valid)))
  print(paste('Dimensions data_strategy2_calib:',dim(data_strategy2_calib)))

} else {

set.seed(42)

sample_temp <- data_strategy2 %>%
  group_by(CTnumL4A) %>%
  sample_n(smotesize)

sample_ID=as.numeric(sample_temp$NewID)
data_strategy2_calib=data_strategy2[which(data_strategy2$NewID %in% sample_ID),]
data_strategy2_valid=data_strategy2[which(!(data_strategy2$NewID %in% sample_ID)),]

rm(sample_temp)
print(paste('Dimensions data_strategy2_valid:',dim(data_strategy2_valid)))
print(paste('Dimensions data_strategy2_calib:',dim(data_strategy2_calib)))

}

## Strategy 3

data_strategy3 <- subset(data_calib, CTnumL4A %in% strategy3list)

if (nrow(data_strategy3)==0) {
  data_strategy3_valid=data_strategy3
  data_strategy3_calib=data_strategy3

  print('No crop type in strategy3')

  print(paste('Dimensions data_strategy3_valid:',dim(data_strategy3_valid)))
  print(paste('Dimensions data_strategy3_calib:',dim(data_strategy3_calib)))

} else {

set.seed(42)

trainindex3= sample.split(data_strategy3$CTnumL4A,SplitRatio=Sample_ratioL)
data_strategy3_valid= data_strategy3[!trainindex3,]
data_strategy3_calib= data_strategy3[trainindex3,]

print(paste('Dimensions data_strategy3_valid:',dim(data_strategy3_valid)))
print(paste('Dimensions data_strategy3_calib:',dim(data_strategy3_calib)))

}

rm(data_calib,data_strategy1,data_strategy2,data_strategy3)

## Compilation of the calibration and validation datasets

data_valid_final=rbind(data_valid1,data_strategy1_valid,data_strategy2_valid,data_strategy3_valid)
data_calib_final=rbind(data_strategy1_calib,data_strategy2_calib,data_strategy3_calib)
data_joined=rbind(data_valid_final,data_calib_final)

rm(data_valid1,data_strategy1_valid,data_strategy2_valid,data_strategy3_valid,data_strategy1_calib,data_strategy2_calib,data_strategy3_calib)

print(paste('Dimensions data_valid_final:',dim(data_valid_final)))
write.csv(data_valid_final,paste0(workdir,paste("Data_validation_final",format(Sys.time(),"%m%d_%H%M"),sep="_"),".csv"))
print(paste('Dimensions data_calib_final:',dim(data_calib_final)))
write.csv(data_calib_final,paste0(workdir,paste("Data_calibration_final_before_smote",format(Sys.time(),"%m%d_%H%M"),sep="_"),".csv"))
print(paste('Dimensions data_joined:',dim(data_joined)))

# ## Trajectory and purpose flags (old but could be reactivated)
#
# classif_ID=c(as.numeric(data_joined$NewID))
# valid_ID=c(as.numeric(data_valid_final$NewID))
# calib_ID=c(as.numeric(data_calib_final$NewID))
#
# for (row in 1:nrow(Shapefile)) {
#   if('NewID' %in% classif_ID) {
#     Shapefile$Trajectory=1
#     if('NewID' %in% valid_ID) {
#       Shapefile$Purpose=2
#     } else if('NewID' %in% calib_ID) {
#       Shapefile$Purpose=1
#     }
#   } else
#   Shapefile$Trajectory=0
#   Shapefile$Purpose=0
# }
#
# # Save declaration dataset with Trajectory and Purpose flags
#
# write.csv(Shapefile,paste0(workdir,"Parcels_all_Trajectory_Purpose_flags.csv"), row.names = FALSE)

## Preparation of data_calib_final_red

data_calib_final_red <- data_calib_final %>% dplyr::select(starts_with("NewID"),starts_with("CTnumL4A"),starts_with("XX"))

## Preparation of data_valid_final_red

data_valid_final_red <- data_valid_final %>% dplyr::select(starts_with("NewID"),starts_with("CTnumL4A"),starts_with("XX"))

## SMOTE process

if (samplingmethod=="Smote") {

    print("Starting Smote")

    grp_data=group_by(data_joined,CTnumL4A)
    nrow=nrow(data_joined)
    areatot=sum(as.numeric(data_joined$AreaDeclared),na.rm=TRUE)
    Declarations_summary = summarise(grp_data, count=n(),area=sum(as.numeric(AreaDeclared)),arearatio=sum(as.numeric(AreaDeclared))/areatot, countratio=n()/nrow)

    grp_data=group_by(data_calib_final,CTnumL4A)
    nrow=nrow(data_calib_final)
    areatot=sum(as.numeric(data_calib_final$AreaDeclared),na.rm=TRUE)
    Declarations_summary_calib = summarise(grp_data, count=n(),area=sum(as.numeric(AreaDeclared)),arearatio=sum(as.numeric(AreaDeclared))/areatot, countratio=n()/nrow)

    Declarations_summary_join=left_join(Declarations_summary_calib,Declarations_summary,by="CTnumL4A",suffix = c(".calib",""))

    Declarations_summary_join$ratio=Declarations_summary_join$count.calib/Declarations_summary_join$count

    rm(Declarations_summary,Declarations_summary_calib)

    Smoted_data=NA

    for (i in 1:nrow(Declarations_summary_join)){

      SMOTEd=NA
      Sample_areaweighted=NA

      print(paste("Iteration",i,"on",nrow(Declarations_summary_join)))
      data_calib_final_red$test=ifelse(data_calib_final$CTnumL4A==Declarations_summary_join$CTnumL4A[i],1,0)
      nparcels=length(which(data_calib_final$CTnumL4A==Declarations_summary_join$CTnumL4A[i]))
      dupsize=smotesize/length(which(data_calib_final$CTnumL4A==Declarations_summary_join$CTnumL4A[i]))
      data_smote <- data_calib_final_red

      if ( dupsize>1) {

      SMOTEd=SMOTE(data_smote[,!(names(data_smote) %in% c("test","CTnumL4A"))],as.numeric(data_smote[,c("test")]),K=k,dup_size=dupsize)

      synthetics=SMOTEd$syn_data
      synthetics2=sample_n(synthetics,(smotesize-nparcels))

      synthetics2$SMOTE=1

      synthetics2$CTnumL4A=Declarations_summary_join$CTnumL4A[i]

      Smoted_data=rbind(Smoted_data,synthetics2)

      }

      originals=data_smote[which(data_smote$CTnumL4A==Declarations_summary_join$CTnumL4A[i]),]
      colnames(originals)[which(names(originals) == "test")] <- "class"
      originals$SMOTE=0
      Smoted_data=rbind(Smoted_data,originals)

    }

    print(paste('Dimensions Smoted_data before removing NAs:',dim(Smoted_data)))

    Smoted_data=na.omit(Smoted_data)
    print(paste('Dimensions Smoted_data after removing NAs:',dim(Smoted_data)))

    data_calib_final_red=Smoted_data

    rm(Smoted_data,data_smote,originals,synthetics,synthetics2)

  } else

  print("Sampling not properly defined")

# Prepare data_joined after smote

data_calib_final_red <- data_calib_final_red %>% dplyr::select(starts_with("NewID"),starts_with("CTnumL4A"),starts_with("SMOTE"),starts_with("XX"))
write.csv(data_calib_final_red,paste0(workdir,paste("Data_calibration_final_after_smote",format(Sys.time(),"%m%d_%H%M"),sep="_"),".csv"))
data_calib_final_red <- data_calib_final_red %>% dplyr::select(starts_with("NewID"),starts_with("CTnumL4A"),starts_with("XX"))
data_joined_smote=rbind(data_calib_final_red,data_valid_final_red)

print(paste('Dimensions data_joined_smote:',dim(data_joined_smote)))

rm(data_joined_smote)

## Random Forest model creation

# Preparation of the classified parcels

parcels_predict <- data_joined %>% dplyr::select(-starts_with("XX")) #-starts_with("Trajectory"),-starts_with("Purpose"))

data_predict_red <- data_joined %>% dplyr:: select(starts_with("XX"))

gc()

# Preparation of the validation dataset

data_valid_final_red_plus <- data_valid_final_red %>% dplyr:: select(union(starts_with("CTnumL4A"),starts_with("XX")))

gc()

# Preparation of the calibration dataset

data_calib_final_red_plus <- data_calib_final_red %>% dplyr:: select(union(starts_with("CTnumL4A"),starts_with("XX")))
data_calib_final_red_plus$CTnumL4A <- as.factor(data_calib_final_red_plus$CTnumL4A)

# Model creation

print("Start training using Ranger script")

start.time <- Sys.time()

Ranger_trees = ranger(CTnumL4A ~ ., data = droplevels(data_calib_final_red_plus), write.forest = TRUE,probability = TRUE,num.trees = numtrees,mtry = NULL,importance = "impurity",min.node.size = min_node_size,seed = 42)

end.time <- Sys.time()
time.taken <- end.time - start.time
time.taken
rm(data_calib_final_red_plus)
gc()
Ranger_trees$time.taken=time.taken
Ranger_trees$inputs=list(variable)
print(Ranger_trees)

# Save model

modelname=paste("Random_Forest_Model",format(Sys.time(), "%m%d_%H%M"),sep="_")
saveRDS(Ranger_trees, paste(workdir,modelname,".rds",sep=""))

## Validation

# Apply RF model

print("Start validation and confusion matrix")

start.time <- Sys.time()

predict_Ranger_trees=predict(Ranger_trees,data_valid_final_red_plus)

end.time <- Sys.time()
time.taken <- end.time - start.time
time.taken

# Work on predictions

predictions=predict_Ranger_trees$predictions

predict.max=apply(predictions, 1, max)
predict.whichmax=apply(predictions, 1, which.max)
predict.class=colnames(predictions)[predict.whichmax]

data_ref=factor(data_valid_final_red_plus$CTnumL4A)

lvl = union(levels(predict.class), levels(data_ref))

predict.class = factor(predict.class, levels = lvl)
data_ref = factor(data_ref, levels = lvl)

print(paste("Number of predictions:",length(predict.class)))
print(paste("Number of reference:",length(data_ref)))

# Confusion matrix function

Results_Ranger=confusionMatrix(predict.class,data_ref,mode="everything")

## Accuracy metrics

Accuracy_metrics <- as.data.frame(t(Results_Ranger$overall))
Accuracy_metrics <- Accuracy_metrics %>% dplyr::select("Accuracy","Kappa")

write.csv(Accuracy_metrics, file = paste0(workdir, paste("Accuracy_metrics",format(Sys.time(), "%m%d_%H%M"),sep="_"), ".csv"), row.names = FALSE, quote = FALSE)

## COnfusion matrix

# Data preparation

pred <- as(Shapefile, "data.frame")

matrice <-as.data.frame.matrix(Results_Ranger$table)

LUT_CTnumL4A <- LUT[!duplicated(LUT$CTnumL4A),]

# User accuracy

matrice$sum_class <- rowSums(matrice[,1:nrow(matrice)])
matrice$well_class <- 0

for (i in c(1:nrow(matrice))){
  matrice[i,'well_class']<-matrice[i,i]
}
matrice$user_accuracy <- matrice$well_class/matrice$sum_class

# Producer accuracy

matrice['sum_dec',]<- colSums(matrice[1:ncol(matrice),],na.rm = T)
matrice['well_class',]<-NaN

for (i in c(1:ncol(matrice))){
  matrice['well_class',i]<-matrice[i,i]
}
matrice['producer_accuracy',]<- matrice['well_class',]/matrice['sum_dec',]

# Overall accuracy

matrice['well_class','sum_class']<- rowSums(matrice['well_class',1:(nrow(matrice)-3)],na.rm=T)
matrice['producer_accuracy','sum_class']<- NA
matrice['sum_dec','user_accuracy']<-NA
matrice['producer_accuracy','user_accuracy']<- (matrice['sum_dec','well_class']/matrice['sum_dec','sum_class'])

# Write confusion matrix

write.csv(matrice, file = paste0(workdir, paste("Confusion_matrix",format(Sys.time(), "%m%d_%H%M"),sep="_"), ".csv"), row.names = TRUE)

## Confusion tables

# Add some info

matrice$producer_accuracy <- t(matrice['producer_accuracy',])
matrice[1:(nrow(matrice)-3),'sum_dec'] <- t(matrice['sum_dec',1:(nrow(matrice)-3)])

matrice$Fscore <- 2*(matrice$user_accuracy*matrice$producer_accuracy)/(matrice$user_accuracy+matrice$producer_accuracy)

matrice$CTnumL4A <- as.numeric(rownames(matrice))

# User accuracy

matrice_p <- merge.data.frame(matrice,LUT_CTnumL4A,by=c('CTnumL4A'),ALL=F)
matrr <- matrice_p %>%
  distinct(CTnumL4A,.keep_all = T)
LUT_red <- matrr[,c('CTnumL4A','CTL4A')]

main_class <- matrr[order(matrr$sum_dec,decreasing = T),]

conf_user <- main_class[,c('CTnumL4A','CTL4A','sum_class','well_class','user_accuracy')]
conf_user[,c('conf_class1','conf_per1','conf_class2','conf_per2','conf_class3','conf_per3')]<-0

matrice1 <-as.data.frame.matrix(Results_Ranger$table)

tmatrice1<-t(matrice1)

matrice2<-matrice1

for (i in c(1:nrow(matrice2))){
  matrice2[i,i]<-0
}

tmatrice2<-t(matrice2)

for(j in c(rownames(tmatrice2))){
  tmatrice2<-tmatrice2[order(-tmatrice2[,as.character(j)]),]
  sum<-sum(tmatrice1[,as.character(j)])
  per1<-tmatrice2[1,as.character(j)]
  per1_class<-rownames(tmatrice2)[1]
  if (per1==0) {
    per1=NA
    per1_class=NA
  }
  per2<-tmatrice2[2,as.character(j)]
  per2_class<-rownames(tmatrice2)[2]
  if (per2==0) {
    per2=NA
    per2_class=NA
  }
  per3<-tmatrice2[3,as.character(j)]
  per3_class<-rownames(tmatrice2)[3]
  if (per3==0) {
    per3=NA
    per3_class=NA
  }

  conf_user[(1:nrow(conf_user))[conf_user$CTnumL4A == j],c('conf_per1')]<-round((per1/sum*100),digits = 1)
  conf_user[(1:nrow(conf_user))[conf_user$CTnumL4A == j],c('conf_per2')]<-round((per2/sum*100),digits = 1)
  conf_user[(1:nrow(conf_user))[conf_user$CTnumL4A == j],c('conf_per3')]<-round((per3/sum*100),digits = 1)

  conf_user[(1:nrow(conf_user))[conf_user$CTnumL4A == j],c('conf_class1')]<- per1_class
  conf_user[(1:nrow(conf_user))[conf_user$CTnumL4A == j],c('conf_class2')]<- per2_class
  conf_user[(1:nrow(conf_user))[conf_user$CTnumL4A == j],c('conf_class3')]<- per3_class

  conf_user[(1:nrow(conf_user))[conf_user$CTnumL4A == j],c('conf_class1')]<-LUT_red[match(conf_user[(1:nrow(conf_user))[conf_user$CTnumL4A == j],c('conf_class1')],LUT_red$CTnumL4A),"CTL4A"]
  conf_user[(1:nrow(conf_user))[conf_user$CTnumL4A == j],c('conf_class2')]<-LUT_red[match(conf_user[(1:nrow(conf_user))[conf_user$CTnumL4A == j],c('conf_class2')],LUT_red$CTnumL4A),"CTL4A"]
  conf_user[(1:nrow(conf_user))[conf_user$CTnumL4A == j],c('conf_class3')]<-LUT_red[match(conf_user[(1:nrow(conf_user))[conf_user$CTnumL4A == j],c('conf_class3')],LUT_red$CTnumL4A),"CTL4A"]

}

results_pred<-pred
rm(pred)

sum_area_results_pred <- sum(results_pred$AreaDeclared)

results_pred_crop_types <- results_pred %>%
  group_by(CTnumL4A) %>%
  summarise(Nr=n(),Nr_percentage=(n()/nrow(results_pred))*100,
            Area=sum(AreaDeclared),Area_percentage=(sum(AreaDeclared)/sum_area_results_pred)*100)

results_pred_crop_types<-results_pred_crop_types[order(-results_pred_crop_types$Area),]

results_pred_crop_types<-filter(results_pred_crop_types, CTnumL4A %in% ctlist)

conf_user_join=left_join(results_pred_crop_types,conf_user,by="CTnumL4A")

conf_user_join <- conf_user_join %>% select(CTnumL4A,CTL4A,sum_class,well_class,user_accuracy,conf_class1,conf_per1,conf_class2,conf_per2,conf_class3,conf_per3,conf_class3,conf_per3)

# Producer accuracy

matrice_pr <- as.data.frame(t(matrice))
matrice_pr$CTnumL4A <- as.numeric(rownames(matrice_pr))

matrice_pr <- merge.data.frame(matrice_pr,LUT_CTnumL4A,by=c('CTnumL4A'),ALL=F)
matrr_p <- matrice_pr %>%
  distinct(CTnumL4A,.keep_all = T)
matrr_p <- matrr_p[1:(nrow(matrice)-3),]
main_class_p <- matrr_p[order(matrr_p$sum_dec,decreasing = T),]

conf_prod <- main_class_p[,c('CTnumL4A','CTL4A','sum_dec','well_class','producer_accuracy')]
conf_prod[,c('conf_class1','conf_per1','conf_class2','conf_per2','conf_class3','conf_per3','conf_class4','conf_per4')]<-0

matrice1 <-as.data.frame.matrix(Results_Ranger$table)

matrice2<-matrice1

for (i in c(1:nrow(matrice2))){
  matrice2[i,i]<-0
}

for(j in c(rownames(matrice2))){
  matrice2<-matrice2[order(-matrice2[,as.character(j)]),]
  sum<-sum(matrice1[,as.character(j)])
  per1<-matrice2[1,as.character(j)]
  per1_class<-rownames(matrice2)[1]
  if (per1==0) {
    per1=NA
    per1_class=NA
  }
  per2<-matrice2[2,as.character(j)]
  per2_class<-rownames(matrice2)[2]
  if (per2==0) {
    per2=NA
    per2_class=NA
  }
  per3<-matrice2[3,as.character(j)]
  per3_class<-rownames(matrice2)[3]
  if (per3==0) {
    per3=NA
    per3_class=NA
  }

  conf_prod[(1:nrow(conf_prod))[conf_prod$CTnumL4A == j],c('conf_per1')]<-round((per1/sum*100),digits = 1)
  conf_prod[(1:nrow(conf_prod))[conf_prod$CTnumL4A == j],c('conf_per2')]<-round((per2/sum*100),digits = 1)
  conf_prod[(1:nrow(conf_prod))[conf_prod$CTnumL4A == j],c('conf_per3')]<-round((per3/sum*100),digits = 1)

  conf_prod[(1:nrow(conf_prod))[conf_prod$CTnumL4A == j],c('conf_class1')]<- per1_class
  conf_prod[(1:nrow(conf_prod))[conf_prod$CTnumL4A == j],c('conf_class2')]<- per2_class
  conf_prod[(1:nrow(conf_prod))[conf_prod$CTnumL4A == j],c('conf_class3')]<- per3_class

  conf_prod[(1:nrow(conf_prod))[conf_prod$CTnumL4A == j],c('conf_class1')]<-LUT_red[match(conf_prod[(1:nrow(conf_prod))[conf_prod$CTnumL4A == j],c('conf_class1')],LUT_red$CTnumL4A),"CTL4A"]
  conf_prod[(1:nrow(conf_prod))[conf_prod$CTnumL4A == j],c('conf_class2')]<-LUT_red[match(conf_prod[(1:nrow(conf_prod))[conf_prod$CTnumL4A == j],c('conf_class2')],LUT_red$CTnumL4A),"CTL4A"]
  conf_prod[(1:nrow(conf_prod))[conf_prod$CTnumL4A == j],c('conf_class3')]<-LUT_red[match(conf_prod[(1:nrow(conf_prod))[conf_prod$CTnumL4A == j],c('conf_class3')],LUT_red$CTnumL4A),"CTL4A"]

}

conf_prod_join=left_join(results_pred_crop_types,conf_prod,by="CTnumL4A")

conf_prod_join <- conf_prod_join %>% select(CTnumL4A,CTL4A,sum_dec,well_class,producer_accuracy,conf_class1,conf_per1,conf_class2,conf_per2,conf_class3,conf_per3,conf_class3,conf_per3)

write.csv(conf_user_join, file = paste0(workdir, paste("Confusion_user",format(Sys.time(), "%m%d_%H%M"),sep="_"), ".csv"), row.names = FALSE)

write.csv(conf_prod_join, file = paste0(workdir, paste("Confusion_producer",format(Sys.time(), "%m%d_%H%M"),sep="_"), ".csv"), row.names = FALSE)

# Plot

F_score <-as.data.frame(Results_Ranger$byClass) %>% select(F1)
F_score <- tibble::rownames_to_column(F_score,"CTnumL4A")
F_score$CTnumL4A=as.numeric(strapplyc(F_score$CTnumL4A, "[0-9]{1,4}",simplify=TRUE))

results_pred_crop_types_filter2<-results_pred_crop_types

# For cumulative area only calculated based on the classified crop types --- begin

filter2nrsum=sum(results_pred_crop_types_filter2$Nr)
filter2areasum=sum(results_pred_crop_types_filter2$Area)

for (i in c(1:nrow(results_pred_crop_types_filter2))) {
  results_pred_crop_types_filter2$Nr_percentage=results_pred_crop_types_filter2$Nr/filter2nrsum*100
  results_pred_crop_types_filter2$Area_percentage=results_pred_crop_types_filter2$Area/filter2areasum*100
}

# For cumulative area only calculated based on the classified crop types --- end

results_pred_crop_types_filter_F_score=left_join(results_pred_crop_types_filter2,F_score,by="CTnumL4A")

LUT_CTnumL4A_CTL4A<-LUT_CTnumL4A %>% select(CTnumL4A,CTL4A)

results_pred_crop_types_filter_F_score=left_join(results_pred_crop_types_filter_F_score,LUT_CTnumL4A_CTL4A,by="CTnumL4A")

results_pred_crop_types_filter_F_score$Metrics<-"F-Score"

results_pred_crop_types_filter_F_score<-results_pred_crop_types_filter_F_score %>% rename(Value = F1)

results_pred_crop_types_filter_F_score$Cumulative_area_percentage<-0
results_pred_crop_types_filter_F_score$Cumulative_area_percentage[1:length(results_pred_crop_types_filter_F_score$Area_percentage)]=cumsum(results_pred_crop_types_filter_F_score$Area_percentage)/100

results_pred_crop_types_filter_F_score<-rbind(c(NA,NA,NA,NA,NA,Accuracy_metrics$Kappa,"Kappa","Kappa"),results_pred_crop_types_filter_F_score)
results_pred_crop_types_filter_F_score<-rbind(c(NA,NA,NA,NA,NA,Accuracy_metrics$Accuracy,"Overall accuracy","Overall accuracy"),results_pred_crop_types_filter_F_score)

for (row in nrow(results_pred_crop_types_filter_F_score)) {
  results_pred_crop_types_filter_F_score$Value[is.nan(as.numeric(results_pred_crop_types_filter_F_score$Value))] <-NA
}

results_pred_crop_types_filter_F_score$CTL4A<-as.character(results_pred_crop_types_filter_F_score$CTL4A)
results_pred_crop_types_filter_F_score$Value<-as.numeric(results_pred_crop_types_filter_F_score$Value)
results_pred_crop_types_filter_F_score$Cumulative_area_percentage<-as.numeric(results_pred_crop_types_filter_F_score$Cumulative_area_percentage)

results_pred_crop_types_filter_F_score$CTL4A<- factor(results_pred_crop_types_filter_F_score$CTL4A, levels = results_pred_crop_types_filter_F_score$CTL4A)

colors=c('Overall accuracy','Kappa',rep('F-Score',nrow(results_pred_crop_types_filter_F_score)-2))

p<-ggplot(data=results_pred_crop_types_filter_F_score,x=CTL4A,y=Value) +
  geom_bar(aes(x=CTL4A,y=Value,fill=colors),width=.5,stat="identity", position="dodge") +
  geom_point(aes(x=CTL4A,y=Cumulative_area_percentage),size=1) +
  scale_x_discrete() +
  scale_y_continuous(limits=c(0,1),breaks=seq(0,1,0.1),expand = c(0, 0)) +
  theme_light() +
  theme(text = element_text(size=14,face="bold"),axis.title.x=element_blank(),axis.title.y=element_text(size=14,face="bold"),axis.text.y=element_text(size=12),axis.text.x=element_text(size=12,angle = 60,hjust = 1))+
#  theme(axis.text.x = element_text(angle = 60,hjust = 1))  + #, hjust = 1,vjust=0.5)) +
  xlab("") + ylab("Accuracy metric or Relative cumulated area (dots)") +
  scale_fill_discrete("") +
  theme(plot.margin=unit(c(0.5,0.5,0.5,0.5),"cm"))

ggsave(paste0(workdir, paste("Accuracy_metrics_plot",format(Sys.time(), "%m%d_%H%M"),sep="_"), ".png"),p,width = 13, height = 8)

rm(results_pred,results_pred_crop_types,results_pred_crop_types_filter2,results_pred_crop_types_filter_F_score)

## Predictions

print("Start prediction")

start.time <- Sys.time()

predict_Ranger_trees=predict(Ranger_trees,data_predict_red)

end.time <- Sys.time()
time.taken <- end.time - start.time
time.taken

predictions=predict_Ranger_trees$predictions

print("After prediction")

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

Predict_classif=data.frame(parcels_predict$NewID,parcels_predict$CTnumL4A,predict.class,round(predict.max,digits=3),predict.2class,round(predict.2max,digits=3))
colnames(Predict_classif)=c("NewID",'CT_decl','CT_pred_1','CT_conf_1','CT_pred_2','CT_conf_2')

# Save classified parcels with predictions

Parcels_classified_with_predictions=left_join(parcels_predict,Predict_classif,by="NewID")

write.csv(Parcels_classified_with_predictions,paste0(workdir,paste("Parcels_classified_with_predictions",format(Sys.time(),"%m%d_%H%M"),sep="_"), ".csv"), row.names = FALSE)

# Save all parcels with prediction

Parcels_all_with_predictions=left_join(Shapefile,Predict_classif,by="NewID")

write.csv(Parcels_all_with_predictions,paste0(workdir,paste("Parcels_all_with_predictions",format(Sys.time(),"%m%d_%H%M"),sep="_"), ".csv"), row.names = FALSE)

rm(list=ls())
