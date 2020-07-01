import numpy as np
import dateutil.parser
import datetime

import glob, os, shutil
from pathlib import Path
import ntpath

from osgeo import osr, ogr
osr.UseExceptions()
ogr.UseExceptions()

def fuse_conf(array_conf):
# Method to calculate the composite confidence level from an array of confidence levels
# Inputs:
#   array_conf: array of confidence values
#   fused confidence

    return 1 - (np.sum((1-array_conf)**-1.))**-1.

def intersection_date(d1, d2, pd1, pd2):
# Method to calculate the intersection between two time intervals
# Inputs:
#   d1: first time of the first interval (datetime)
#   d2: last time of the first interval (datetime)
#   pd1: first time of the second interval (datetime)
#   pd2: last time of the second interval (datetime)
# Outputs:
#   ret: list of first and last date of the intersection interval
#   valid_inter: True for valid interval, False otherwise

    ret = np.array([max(d1, pd1), min(d2, pd2)])
    valid_inter = ret[1] > ret[0]
    
    return valid_inter, ret

def fuse_dets(date1, date2, conf, mission, pr_date1, pr_date2, pr_conf, pr_mis):
# This function claculate the intersection of detection intervals taking into consideration:
#   - the confidence levels
#   - temporal intervals among detections
# Inputs
#   date1: list of starting dates of the first set of intervals
#   date2: list of ending dates of the first set of the intervals
#   conf: list of the confidence levels of the first set of the intervals
#   pr_date1: list of starting dates of the second set of intervals
#   pr_date2: list of ending dates of the second set of intervals intervals
#   pr_conf: list of the confidence levels of the second set of the intervals
#   pr_mis: list of the missions strings of the second set of the intervals
# Outputs
#   list of three elements: [list of the confidence levels, list of starting dates, list of ending dates]

    lend = len(date1)
    pr_lend = len(pr_date1)

    if lend == 0 and pr_lend == 0:
        return None, None, None, None
    
    if lend==0:
        list_interv = list(zip(*[pr_conf, pr_date1, pr_date2, pr_mis]))
    elif pr_lend==0:
        list_interv = list(zip(*[conf, date1, date2, mission]))
    else:
        list_interv = []
        banned_i, banned_j = [], []
        for i in range(lend):
            for j in range(pr_lend):
                valid_inter, interv = intersection_date(date1[i], date2[i], pr_date1[j], pr_date2[j])
                if valid_inter: #conferma di detection precedente
###
                    if conf[i] > pr_conf[j]:
                        interv = np.array((date1[i], date2[i]))
                    else:
                        interv = np.array((pr_date1[j], pr_date2[j]))
###

#### Inizio -------- aggiunta da LAURA
#                    interv = np.array((date1[i], date2[i]))
#### Fine   -------- aggiunta da LAURA
                    mis_str = pr_mis[j]
                    if mission[i] not in pr_mis[j]:
                        mis_str = pr_mis[j] + '/' + mission[i]
                    # calculate fused confidence
                    fused_confidence = fuse_conf(np.array([conf[i], pr_conf[j]]))
                    if mis_str == "S1": # only for the fusion of two S1 detection confidences, it is ensures that the S1 confidences are not higher than 0.5
#                        print("mis_str == S1", mis_str)
                        interv_to_add = (np.minimum(fused_confidence, 0.5), interv[0], interv[1], mis_str)
                    else:
#                        print("mis_str != S1", mis_str)
                        interv_to_add = (fused_confidence, interv[0], interv[1], mis_str)
                    list_interv.append(interv_to_add)
                    banned_i.append(i)
                    banned_j.append(j)
        for i in range(lend):
            if i not in banned_i:
                list_interv.append((conf[i], date1[i], date2[i], mission[i]))
        for j in range(pr_lend):
            if j not in banned_j:
                list_interv.append((pr_conf[j], pr_date1[j], pr_date2[j], pr_mis[j]))

    list_interv = sorted(list_interv)[::-1]  # ordino rispetto al livello di confidenza

    return list(zip(*list_interv))

def cloneAndUpdateShapefile(source, dest):
# It makes a copy of the source shape file if not already existing
# Input:
#   source: file name of the input shape file
#   dest:   file name of the destination shape file
# Output:
#   None: write a copy of the file with dest file name

    print("Cloning source...")
    # drv = ogr.GetDriverByName('ESRI Shapefile')
    # ds = drv.CopyDataSource(ogr.Open(source), dest)
    # layer = ds.GetLayerByIndex(0)
    parentSrcDir = Path(source).parent
    destDir = Path(dest).parent
    fileName = ntpath.basename(source)
    destFileName = ntpath.basename(dest)
    fileNameNoExt = os.path.splitext(fileName)[0]
    destFileNameNoExt = os.path.splitext(destFileName)[0]

    pathToCheck = os.path.join(parentSrcDir, "{}.*".format(fileNameNoExt))
    files = glob.iglob(pathToCheck)
    for file in files:
        if os.path.isfile(file):
            srcFileExt = os.path.splitext(file)[1]
            destFilePath = os.path.join(destDir, "{}{}".format(destFileNameNoExt, srcFileExt))
            print ("Copying source file {} to dest file {}".format(file, destFilePath))
            shutil.copyfile(file, destFilePath)
    
#    for i in range(1, max_dates+1):
#        layer.CreateField(ogr.FieldDefn('m%d_date' % i, ogr.OFTString))
#        layer.CreateField(ogr.FieldDefn('m%d_conf' % i, ogr.OFTReal))
#        layer.CreateField(ogr.FieldDefn('m%d_mis' % i, ogr.OFTString))

def writeDetections_S2(dest, segment_ids, confidence, dateList, valid_date_list_mask, mission_id='S2', max_dates=4, minimum_interval_days=30, non_overlap_interval_days=45):
# It write detection in the output shape file
# Input:
#   dest: file name of the destination shape file
#   segment_ids: list of the segments ids
#   confidence: list of the confidences (a confidence level>0 is a detection)
#   dateList1: list of the starting dates
#   dateList2: list of the ending dates
#   mission_id: mission id (only one string)
#   max_dates: maximum number of detection intervals
# Output:
#   None, write results on the destination shape file

    dateList = np.array(dateList)
    detections = confidence > 0
    
    print("Reading features...")
    ds = ogr.Open(dest, 1)
    layer = ds.GetLayerByIndex(0)
    #fids, features = list(zip(*[(feature['OBJECTID'], feature) for feature in layer]))
    fids, features = list(zip(*[(feature.GetFID(), feature) for feature in layer]))
    
    print("Sorting features...")
    sort_idx = np.argsort(fids)
    features = np.array(features)[sort_idx]
    fids = np.array(fids)[sort_idx]
    idx = np.searchsorted(fids, segment_ids)
    
    print("Updating features...")
    count = 0
    for i, segment_id, detection, conf, valid_dates in zip(idx, segment_ids, detections, confidence, valid_date_list_mask):
        if i >= len(features):
            count += 1
            continue

        feature = features[i]

#        assert feature['OBJECTID'] == segment_id
        assert feature.GetFID() == segment_id
        
        date2 = dateList[detection]
        date1 = []
        
        # calculation of date1
        valid_ndvi_ind = np.where(valid_dates>0)[0]
        det_ind = np.where(detection>0)[0]
        for di in det_ind:
            det_index1 = valid_ndvi_ind[max(np.searchsorted(valid_ndvi_ind, di)-1,0)]
            det_index2 = di
            det_date1 = dateList[det_index1]
            det_date2 = dateList[di]
#            print("Prima -> ", "DetD1:", det_date1,"DetD2:", det_date2)

            if (det_date2 - det_date1).days > minimum_interval_days:
                det_date1 = det_date2 - datetime.timedelta(days=minimum_interval_days)
#            print("Dopo -> ", "DetD1:", det_date1,"DetD2:", det_date2)
            date1.append(det_date1)
        date1 = np.array(date1)
        conf = conf[detection]
        mis = np.size(date1)*[mission_id]
        if int(feature['mow_n']): # if there are mowing events already detected do FUSION
            # read values from file
            pr_date1, pr_date2, pr_conf, pr_mis = \
            zip(*[(dateutil.parser.parse(feature['m%d_dstart' % j], yearfirst=True, dayfirst=False),
                   dateutil.parser.parse(feature['m%d_dend' % j], yearfirst=True, dayfirst=False),
                   feature['m%d_conf' % j], feature['m%d_mis' % j]) for j in range(1, 1+int(feature['mow_n']))])
            # fuse the previous detections with the current one

            conf, date1, date2, mis = fuse_dets(date1, date2, conf, mis, pr_date1, pr_date2, pr_conf, pr_mis)

        # seleziono i più confidenti e tali che la distanza temporale tra i due intervalli
        # (misurata come differenza tra l'estremo sinistro del primo intervallo e l'estremo destro del secondo)
        # sia inferiore a non_overlap_interval_days
        list_interv = [(c, d1, d2, m) for c, d1, d2, m in zip(conf, date1, date2, mis)]
        list_interv = sorted(list_interv)[::-1]  # ordino rispetto al livello di confidenza
        selected_interv = []
        for interv in list_interv:
            add_interv = True
            for aux in selected_interv:
#                if abs(interv[2] - aux[2]) < datetime.timedelta(days=non_overlap_interval_days):
                if np.maximum(interv[2] - aux[1], aux[2] - interv[1]) < datetime.timedelta(days=non_overlap_interval_days):
                    add_interv = False
                    break
            if add_interv:
                selected_interv.append(interv)

        selected_interv = sorted(selected_interv)[::-1]  # ordino per confidenza
        selected_interv = selected_interv[:max_dates]  # in caso siano più di max_dates prendo le piu confidenti
        selected_interv = sorted(selected_interv, key=lambda x:x[1]) # riordino rispetto ai tempi

        # Write Detections
        if len(selected_interv) > 0:  # there is at least 1 detection

            conf, date1, date2, mis = list(zip(*selected_interv))

            # Reset previous detections
            for ind_daca in range(max_dates):
                feature['m%d_dstart' % (ind_daca+1)] = 0
                feature['m%d_dend' % (ind_daca+1)] = 0
                feature['m%d_conf' % (ind_daca+1)] = 0.0
                feature['m%d_mis' % (ind_daca+1)] = "0"

            # Write fused detections
            count_dates = 0
            for d1, d2, c, mis_str in zip(date1, date2, conf, mis):
                if count_dates >= max_dates:
                    break
                count_dates += 1
                feature['m%d_dstart' % count_dates] = "{:%Y-%m-%d %H:%M:%S}".format(d1)
                feature['m%d_dend' % count_dates] = "{:%Y-%m-%d %H:%M:%S}".format(d2)
                feature['m%d_conf' % count_dates] = round(c, 3)
                feature['m%d_mis' % count_dates] = mis_str
            feature['mow_n'] = count_dates
        feature['proc'] = 1
        layer.SetFeature(feature)
        if count:
            print("Cannot find %d segments" % count)

def writeDetections_S1(dest, segment_ids, confidence, dateList1, dateList2, mission_id='S1', max_dates=4, minimum_interval_days=30):
# It write detection in the output shape file
# Input:
#   dest: file name of the destination shape file
#   segment_ids: list of the segments ids
#   confidence: list of the confidences (a confidence level>0 is a detection)
#   dateList1: list of the starting dates
#   dateList2: list of the ending dates
#   mission_id: mission id (only one string)
#   max_dates: maximum number of detection intervals
# Output:
#   None, write results on the destination shape file

    dateList1, dateList2 = np.array(dateList1), np.array(dateList2)
    detections = confidence > 0
    
    print("Reading features...")
    ds = ogr.Open(dest, 1)
    layer = ds.GetLayerByIndex(0)
    fids, features = list(zip(*[(feature.GetFID(), feature) for feature in layer]))
    
    print("Sorting features...")
    sort_idx = np.argsort(fids)
    features = np.array(features)[sort_idx]
    fids = np.array(fids)[sort_idx]
    idx = np.searchsorted(fids, segment_ids)
    
    print("Updating features...")
    count = 0
    for i, segment_id, detection, conf in zip(idx, segment_ids, detections, confidence):
        if i >= len(features):
            count += 1
            continue

        feature = features[i]

        assert feature.GetFID() == segment_id
        
        date1 = dateList1[detection]
        date2 = dateList2[detection]
        conf = conf[detection]
        mis = np.size(date1)*[mission_id]
        if int(feature['mow_n']): # if there are mowing events already detected
            # read values from file
            pr_date1, pr_date2, pr_conf, pr_mis = \
            zip(*[(dateutil.parser.parse(feature['m%d_dstart' % j], yearfirst=True, dayfirst=False),
                   dateutil.parser.parse(feature['m%d_dend' % j], yearfirst=True, dayfirst=False),
                   feature['m%d_conf' % j], feature['m%d_mis' % j]) for j in range(1, 1+int(feature['mow_n']))])

            # fuse the previous detections with the current one
            conf, date1, date2, mis = fuse_dets(date1, date2, conf, mis, pr_date1, pr_date2, pr_conf, pr_mis)

        # seleziono i più confidenti e tali che la distanza temporale tra i due intervalli (misurata come differenza tra gli estremi destri degli intervalli)
        # sia inferiore a non_overlap_interval_days
        list_interv = [(c, d1, d2, m) for c, d1, d2, m in zip(conf, date1, date2, mis)]
        list_interv = sorted(list_interv)[::-1]  # ordino rispetto al livello di confidenza
        selected_interv = []
        for interv in list_interv:
            add_interv = True
            for aux in selected_interv:
                if abs(interv[2] - aux[2]) < datetime.timedelta(days=minimum_interval_days):
                    add_interv = False
                    break
            if add_interv:
                selected_interv.append(interv)

        selected_interv = sorted(selected_interv)[::-1]  # ordino per confidenza
        selected_interv = selected_interv[:max_dates]  # in caso siano più di max_dates prendo le piu confidenti
        selected_interv = sorted(selected_interv, key=lambda x:x[1]) # riordino rispetto ai tempi

        # Write Detections
        if len(selected_interv) > 0:  # there is at least 1 detection

            conf, date1, date2, mis = list(zip(*selected_interv))

            # Reset previous detections
            for ind_daca in range(max_dates):
                feature['m%d_dstart' % (ind_daca+1)] = 0
                feature['m%d_dend' % (ind_daca+1)] = 0
                feature['m%d_conf' % (ind_daca+1)] = 0.0
                feature['m%d_mis' % (ind_daca+1)] = "0"

            count_dates = 0
            for d1, d2, c, mis_str in zip(date1, date2, conf, mis):
                if count_dates >= max_dates:
                    break
                count_dates += 1
                feature['m%d_dstart' % count_dates] = "{:%Y-%m-%d %H:%M:%S}".format(d1)
                feature['m%d_dend' % count_dates] = "{:%Y-%m-%d %H:%M:%S}".format(d2)
                feature['m%d_conf' % count_dates] = round(c, 3)
                feature['m%d_mis' % count_dates] = mis_str

            feature['mow_n'] = count_dates
        feature['proc'] = 1
        layer.SetFeature(feature)
        if count:
            print("Cannot find %d segments" % count)


