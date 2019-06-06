#!/usr/bin/env Rscript
#
# requiredPackages = c("ranger","dplyr","e1071","caret","smotefamily","readr","gsubfn")
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

## Implement variables

variable <- commandArgs(trailingOnly=TRUE)
print(variable)
workdir <- variable[1]  #transform to numeric !!
InputSAR <- variable[2]
InputOpt <- variable[3]
InputSAR_tempStats <- variable[4]
Shape_filename <- variable[5]
TARGET <- variable[6]
LC_monitored <- variable[7]
AreaDeclared <- variable[8]
S2pixMIN <- as.numeric(variable[9])
S1pixMIN <- as.numeric(variable[10])
PaMIN <- as.numeric(variable[11])
S2pixBEST <- as.numeric(variable[12])
PaCalibH <- as.numeric(variable[13])
PaCalibL <- as.numeric(variable[14])
Sample_ratioH <- as.numeric(variable[15])
Sample_ratioL <- as.numeric(variable[16])
samplingmethod <-variable[17]
smotesize <- as.numeric(variable[18])
k <- as.numeric(variable[19])
numtrees <- as.numeric(variable[20])
min_node_size <- as.numeric(variable[21])

## Check if parameteres correctly defined: LC and samplingmethod

if (!(LC_monitored %in% c(1,2,3,4,5,0,"All","123","1234"))) {
  stop("Wrong argument: choose out of (1,2,3,4,5,0,'All','123','1234')", call.=FALSE)
}

if(LC_monitored=="All") landcover=c(1,2,3,4,5)
if(LC_monitored=="1234") landcover=c(1,2,3,4)
if(LC_monitored=="123") landcover=c(1,2,3)
print(paste0("Monitored land cover classes are:",landcover))

if (!(samplingmethod %in% c("Random","Areaweighted","Overareaweighted","Overunderareaweighted","Smote","Fixunder"))) {
  stop('Wrong argument: choose out of "Random","Areaweighted","Overareaweighted","Overunderareaweighted","Smote","Fixunder"', call.=FALSE)
}

## Import declaration dataset as csv

print("Importing declaration dataset")

Shapefile=read_csv(Shape_filename,col_types= cols_only(NewID = 'i',HoldID = 'i',GeomValid = 'l',Duplic = 'l',Overlap = 'l',Area_meters = 'd',ShapeInd = 'd',CTnum = 'i',CT = 'c',LC = 'i',S1Pix = 'i',S2Pix = 'i',CTnumL4A = 'i',CTL4A = 'c',CTnumDIV = 'i',CTDIV = 'c',EAA = 'l',AL = 'l',PGrass = 'l',TGrass = 'l',Fallow = 'l',Cwater = 'l'))

Shapefile$Trajectory <-0
Shapefile$Purpose <-0

names(Shapefile)[which(names(Shapefile)==TARGET)]="TARGET"
names(Shapefile)[which(names(Shapefile)==AreaDeclared)]="AreaDeclared"

## Join the optical and SAR data to declaration dataset

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
  print("Optical, SAR and SAR temporal features")

  print("Importing Opt_Features...")
  ncol_Optcsv=system(paste("head -1",InputOpt,"| sed 's/[^,]//g' | wc -c"),intern=TRUE)
  Opt_features=read_csv(InputOpt,col_types=paste0("i",paste(rep("d",as.numeric(ncol_Optcsv)-1),collapse="")))

  print("Importing SAR_Features...")
  ncol_SARcsv=system(paste("head -1",InputSAR,"| sed 's/[^,]//g' | wc -c"),intern=TRUE)
  SAR_features=read_csv(InputSAR,col_types=paste0("i",paste(rep("d",as.numeric(ncol_SARcsv)-1),collapse="")))

  print("Importing SAR_Temporal_Features...")
  ncol_SARcsv=system(paste("head -1",InputSAR_tempStats,"| sed 's/[^,]//g' | wc -c"),intern=TRUE)
  SAR_tempStats=read_csv(InputSAR_tempStats,col_types=paste0("i",paste(rep("d",as.numeric(ncol_SARcsv)-1),collapse="")))

  print("SAR, Optical and SAR temporal features imported successfully")

data_joined=inner_join(Shapefile,SAR_features,by="NewID")

print("join by 1 ok")

data_joined=inner_join(data_joined,Opt_features,by="NewID")

print("join by 2 ok")

data_joined=inner_join(data_joined,SAR_tempStats,by="NewID")

print("join by 3 ok")

} else if (InputSAR==0 & InputOpt==0){
  print("Define SAR or Optical parameters")
}

data_joined=as.data.frame(data_joined)
print(paste('Dimensions before filtering:',dim(data_joined)))

## Filtering steps

# Filter out data with NAs

data_joined=na.omit(data_joined)
print(paste('Dimensions after removing NAs:',dim(data_joined)))

# Filter out not monitored LC classes

data_joined <- subset(data_joined, LC %in% landcover)
print(paste('Dimensions after filtering by LC:',dim(data_joined)))

# Filter out small parcels

data_joined <- subset(data_joined, S2Pix >= S2pixMIN & S1Pix >= S1pixMIN)
print(paste('Dimensions after filtering out small parcels:',dim(data_joined)))

# Filter out crop types with few parcels

dj2 <- count(data_joined,TARGET)
dj3 <- subset(dj2, n >= PaMIN)
ctlist <- dj3[, "TARGET", drop=TRUE]
print(paste('Crop types after filtering out crop types with few parcels:',ctlist))
data_joined <- subset(data_joined, TARGET %in% ctlist)
print(paste('Dimensions after filtering out crop types with few parcels:',dim(data_joined)))
ctlist2 <- data_joined[, "NewID", drop=TRUE]

## Selection of best parcels for calibration

data_valid1 <- subset(data_joined, S2Pix < S2pixBEST)
print(paste('Dimensions of the first part of the validation dataset with parcels with S2Pix < S2pixBEST:',dim(data_valid1)))

data_calib <- subset(data_joined, S2Pix >= S2pixBEST)
print(paste('Dimensions after selecting best parcels (S2Pix >= S2pixBEST) for calibration:',dim(data_calib)))

# Crop type strategy lists

dc1 <- count(data_calib,TARGET)
print(dc1)

strategy1 <- subset(dc1, n >= PaCalibH)
strategy1list <- strategy1[, "TARGET", drop=TRUE]
print(paste("Crop types belonging to strategy 1:",strategy1list))

strategy2 <- subset(dc1, n < PaCalibH & n >= PaCalibL )
strategy2list <- strategy2[, "TARGET", drop=TRUE]
print(paste("Crop types belonging to strategy 2:",strategy2list))

strategy3 <- subset(dc1, n < PaCalibL )
strategy3list <- strategy3[, "TARGET", drop=TRUE]
print(paste("Crop types belonging to strategy 3:",strategy3list))

# Strategy 1

data_strategy1 <- subset(data_calib, TARGET %in% strategy1list)

set.seed(42)
trainindex1=createDataPartition(data_strategy1$TARGET, times = 1, p = Sample_ratioH, list = FALSE)
data_strategy1_valid=data_strategy1[-trainindex1,]
data_strategy1_calib=data_strategy1[trainindex1,]

print(paste('Dimensions data_strategy1_valid:',dim(data_strategy1_valid)))
print(paste('Dimensions data_strategy1_calib:',dim(data_strategy1_calib)))

# Strategy 2

data_strategy2 <- subset(data_calib, TARGET %in% strategy2list)

set.seed(42)
sample_temp <- data_strategy2 %>%
  group_by(TARGET) %>%
  sample_n(smotesize)

# sample_temp=sample(data_strategy2$TARGET,smotesize)
sample_ID=as.numeric(sample_temp$NewID)
data_strategy2_calib=data_strategy2[which(data_strategy2$NewID %in% sample_ID),]
data_strategy2_valid=data_strategy2[which(!(data_strategy2$NewID %in% sample_ID)),]

print(paste('Dimensions data_strategy2_valid:',dim(data_strategy2_valid)))
print(paste('Dimensions data_strategy2_calib:',dim(data_strategy2_calib)))

# Strategy 3

data_strategy3 <- subset(data_calib, TARGET %in% strategy3list)

set.seed(42)
trainindex3=createDataPartition(data_strategy3$TARGET, times = 1, p = Sample_ratioL, list = FALSE)
data_strategy3_valid=data_strategy3[-trainindex3,]
data_strategy3_calib=data_strategy3[trainindex3,]

print(paste('Dimensions data_strategy3_valid:',dim(data_strategy3_valid)))
print(paste('Dimensions data_strategy3_calib:',dim(data_strategy3_calib)))

## Compilation of the calibration and validation datasets

data_valid_final=rbind(data_valid1,data_strategy1_valid,data_strategy2_valid,data_strategy3_valid)
data_calib_final=rbind(data_strategy1_calib,data_strategy2_calib,data_strategy3_calib)
data_joined=rbind(data_valid_final,data_calib_final)

print(paste('Dimensions data_valid_final:',dim(data_valid_final)))
print(paste('Dimensions data_calib_final:',dim(data_calib_final)))
print(paste('Dimensions data_joined:',dim(data_joined)))

# ## Trajectory and pupose flags
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
# write.csv(Shapefile,paste0(workdir,"Declaration_dataset_purpose_trajectory.csv"), row.names = FALSE)

## Preparation of data_calib_final

data_calib_final_red <- data_calib_final %>% dplyr:: select(union(starts_with("NewID"),union(starts_with("XX"),starts_with("TARGET"))))

## SMOTE process

if (samplingmethod=="Smote") {

    print("Starting Smote")

    grp_data=group_by(data_joined,TARGET)
    nrow=nrow(data_joined)
    areatot=sum(as.numeric(data_joined$AreaDeclared),na.rm=TRUE)
    Declarations_summary = summarise(grp_data, count=n(),area=sum(as.numeric(AreaDeclared)),arearatio=sum(as.numeric(AreaDeclared))/areatot, countratio=n()/nrow)

    grp_data=group_by(data_calib_final,TARGET)
    nrow=nrow(data_calib_final)
    areatot=sum(as.numeric(data_calib_final$AreaDeclared),na.rm=TRUE)
    Declarations_summary_calib = summarise(grp_data, count=n(),area=sum(as.numeric(AreaDeclared)),arearatio=sum(as.numeric(AreaDeclared))/areatot, countratio=n()/nrow)

    Declarations_summary_join=left_join(Declarations_summary_calib,Declarations_summary,by="TARGET",suffix = c(".calib",""))

    Smoted_data=NA

    for (i in 1:nrow(Declarations_summary_join)){

      SMOTEd=NA
      Sample_areaweighted=NA

      print(paste("Iteration",i,"on",nrow(Declarations_summary_join)))
      data_calib_final_red$test=ifelse(data_calib_final$TARGET==Declarations_summary_join$TARGET[i],1,0)
      nparcels=length(which(data_calib_final$TARGET==Declarations_summary_join$TARGET[i]))
      dupsize=smotesize/length(which(data_calib_final$TARGET==Declarations_summary_join$TARGET[i]))
      data_smote <- data_calib_final_red

      if ( dupsize>1) {

      SMOTEd=SMOTE(data_smote[,!(names(data_smote) %in% c("test","TARGET"))],as.numeric(data_smote[,c("test")]),K=k,dup_size=dupsize)

      originals=SMOTEd$orig_P
      synthetics=SMOTEd$syn_data
      synthetics2=sample_n(synthetics,(smotesize-nparcels))

      originals$SMOTE=1
      synthetics2$SMOTE=0

      synthetics2$TARGET=Declarations_summary_join$TARGET[i]
      originals$TARGET=Declarations_summary_join$TARGET[i]

      Smoted_data=rbind(Smoted_data,synthetics2)

      } else

      originals=data_smote[which(data_smote$TARGET==Declarations_summary_join$TARGET[i]),]
      colnames(originals)[which(names(originals) == "test")] <- "class"
      originals$SMOTE=1
      Smoted_data=rbind(Smoted_data,originals)

    }

    print(paste('Dimensions Smoted_data before removing NAs:',dim(Smoted_data)))

    Smoted_data=na.omit(Smoted_data)
    print(paste('Dimensions Smoted_data after removing NAs:',dim(Smoted_data)))
    write.csv(Smoted_data,paste0(workdir,"Smoted_data.csv"), row.names = FALSE)

    ind=which(round(Smoted_data$NewID)==Smoted_data$NewID)
    NewIDs=Smoted_data$NewID[ind]
    write.csv(NewIDs,paste0(workdir,paste("Calib_NewIDs",format(Sys.time(),"%m%d-%H%M"),sep="_"),".csv"))

    data_calib_final_red=Smoted_data
    rm(Smoted_data)

  } else

  print("Sampling not properly defined")

## Random Forest model creation

# Preparation of the classified parcels

parcels_predict = data.frame(data_joined$NewID, data_joined$AreaDeclared, data_joined$TARGET)
colnames(parcels_predict)=c("NewID", "AreaDeclared", "TARGET")

parcels_predict_name = paste("Parcels_predict",format(Sys.time(),"%m%d-%H%M"), sep="_")
saveRDS(parcels_predict, paste0(workdir, parcels_predict_name, ".rds"))

data_predict_red <- data_joined %>% dplyr:: select(starts_with("XX"))
data_predict_red_name = paste("Data_predict",format(Sys.time(),"%m%d-%H%M"),sep="_")
saveRDS(data_predict_red, paste0(workdir,data_predict_red_name,".rds"))
rm(data_joined, data_predict_red)
gc()

# Preparation of the validation dataset

data_valid_final_red <- data_valid_final %>% dplyr:: select(union(starts_with("TARGET"),starts_with("XX")))
print(paste('Dimensions sample for validation:',dim(data_valid_final_red)))
data_valid_final_red_name = paste("Data_valid_red",format(Sys.time(),"%m%d-%H%M"), sep="_")
saveRDS(data_valid_final_red, paste0(workdir, data_valid_final_red_name, ".rds"))
rm(data_valid_final_red)
gc()

# Preparation of the calibration dataset

data_model <- data_calib_final_red %>% dplyr:: select(union(starts_with("TARGET"),starts_with("XX")))
data_model$TARGET <- as.factor(data_model$TARGET)
write.csv(data_model,paste0(workdir,"data_calib_final.csv"), row.names = FALSE)
rm(data_calib_final_red)
print(paste('Dimensions sample for training:',dim(data_model)))

# Model creation

print("Start training using Ranger script")

start.time <- Sys.time()

Ranger_trees = ranger(TARGET ~ ., data = droplevels(data_model), write.forest = TRUE,probability = TRUE,num.trees = numtrees,mtry = NULL,importance = "impurity",min.node.size = min_node_size,seed = 42)

end.time <- Sys.time()
time.taken <- end.time - start.time
time.taken
rm(data_model)
gc()
Ranger_trees$time.taken=time.taken
Ranger_trees$inputs=list(variable)
print(Ranger_trees)

# Save model

modelname=paste("Ranger",format(Sys.time(), "%m%d-%H%M"),sep="_")
saveRDS(Ranger_trees, paste(workdir,modelname,".rds",sep=""))

## Validation and confusion matrix

print("Start validation and confusion matrix")

# Prediction on validation

data_valid_final_red <- readRDS(paste0(workdir, data_valid_final_red_name, ".rds"))
write.csv(data_valid_final_red,paste0(workdir,"data_valid_final.csv"), row.names = FALSE)

start.time <- Sys.time()

predict_Ranger_trees=predict(Ranger_trees,data_valid_final_red)

end.time <- Sys.time()
time.taken <- end.time - start.time
time.taken

# Save predictions

predictname=paste("Predict_Ranger",format(Sys.time(), "%m%d-%H%M"),sep="_")
print(paste(workdir,predictname,".rds",sep=""))
saveRDS(predict_Ranger_trees, paste(workdir,predictname,".rds",sep=""))

# Work on predictions

predictions=predict_Ranger_trees$predictions

predict.max=apply(predictions, 1, max)
predict.whichmax=apply(predictions, 1, which.max)
predict.class=colnames(predictions)[predict.whichmax]

data_ref=factor(data_valid_final_red$TARGET)

lvl = union(levels(predict.class), levels(data_ref))

predict.class = factor(predict.class, levels = lvl)
data_ref = factor(data_ref, levels = lvl)

print(paste("Number of predictions:",length(predict.class)))
print(paste("Number of reference:",length(data_ref)))

# Confusion matrix and metrics

Results_Ranger=confusionMatrix(predict.class,data_ref,mode="everything")
resultname=paste("Results_Ranger",samplingmethod,numtrees,format(Sys.time(), "%m%d-%H%M"),sep="_")
saveRDS(Results_Ranger, paste(workdir,resultname,".rds",sep=""))
print(Results_Ranger$overall)

metrics_name=paste("Metrics",samplingmethod,numtrees,format(Sys.time(), "%m%d-%H%M"),sep="_")
write.csv(t(Results_Ranger$overall), file = paste0(workdir, metrics_name, ".csv"), row.names = FALSE, quote = FALSE)

# Plot

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

Predict_classif=data.frame(parcels_predict$NewID,parcels_predict$AreaDeclared,parcels_predict$TARGET,predict.class,round(predict.max,digits=3),predict.2class,round(predict.2max,digits=3))
colnames(Predict_classif)=c("NewID",'Area','CT_decl','CT_pred_1','CT_conf_1','CT_pred_2','CT_conf_2')

# Save predictions

predictname=paste("Predict_classif",modelname,sep="_")
write.csv(Predict_classif,paste0(workdir,predictname,".csv"), row.names = FALSE)
