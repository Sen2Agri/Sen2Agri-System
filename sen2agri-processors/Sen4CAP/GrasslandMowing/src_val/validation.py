import numpy as np
import pandas as pd
import dateutil.parser
import datetime
import configparser
import ast
from osgeo import ogr
ogr.UseExceptions()


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


def do_validation(det_shapefile_name, truth_csv_filename, validation_conf_file_name, det_parcel_id_attribute='parcel_id', truth_parcel_id_attribute='parcel_id'):
    
    # load validation parameters
    config = configparser.ConfigParser()
    config.read(validation_conf_file_name)
    validation_interval = list(ast.literal_eval(config['validation']['validation_temporal_range_str']))
    validation_interval = [dateutil.parser.parse(d, yearfirst=True, dayfirst=False) for d in validation_interval]
    fwd_tol_days = np.int(config['validation']['fwd_tol_days'])
    bkw_tol_days = np.int(config['validation']['bkw_tol_days'])
    
    # load truths
    print("Reading truths ...")
    tr_df = pd.read_csv(truth_csv_filename, sep=';')
    # set indexes of data frame
    tr_df['proc'] = 0
    tr_df['FN'] = 1
    tr_df[truth_parcel_id_attribute] = tr_df.apply(lambda x: str(x[truth_parcel_id_attribute]), axis=1)
    tr_df = tr_df.set_index([truth_parcel_id_attribute, 'data'])
    
    # load detections
    print("Reading detections ...")
    ds = ogr.Open(det_shapefile_name, 1)
    layer = ds.GetLayerByIndex(0)
    det_fids, det_features = list(zip(*[(feature.GetFID(), feature) for feature in layer]))
    
    # initialize validation data structures
    TP = 0
    FP = 0
    tr_df[['proc', 'FN']] = [0, 1]

    # validation
    for f in det_fids:
        feature = det_features[f]
        if feature['proc'] == 0:
            continue
        p_id = str(feature[det_parcel_id_attribute])
        try:
            pid_tr = tr_df.loc[p_id]
        except KeyError:
            continue
        tr_df.loc[p_id, 'proc'] = 1
        mow_n = int(feature['mow_n'])
        for i in range(1, mow_n+1):
            det_t_start = dateutil.parser.parse(feature['m%d_dstart' % i], yearfirst=True, dayfirst=False)
            det_t_end = dateutil.parser.parse(feature['m%d_dend' % i], yearfirst=True, dayfirst=False)
            if det_t_start > validation_interval[1]:
                continue
            for j in pid_tr.index:
                true_t_start = dateutil.parser.parse(j+" "+"00:00:00", yearfirst=False, dayfirst=True) - datetime.timedelta(days=bkw_tol_days)
                true_t_end = dateutil.parser.parse(j+" "+"23:59:59", yearfirst=False, dayfirst=True) + datetime.timedelta(days=fwd_tol_days)
                mowing_in_TR, _ = intersection_date(true_t_start, true_t_end, det_t_start, det_t_end)
                if mowing_in_TR:
                    tr_df.loc[(p_id, j), 'FN'] = 0
                    break
            if mowing_in_TR:
                TP += 1
            else:
                FP += 1
    FN = (tr_df['proc']*tr_df['FN']).sum()
    print("truths proc", tr_df['proc'].sum())
    
    print("TP =", TP)
    print("FP =", FP)
    print("FN =", FN)
    if (TP+FN) > 0:
        recall = TP/(TP+FN)
        print("Recall    = TP/(TP+FN)", recall)
    else:
        recall = np.nan
        print("Recall undetermined [TP/(TP+FN)]")
    if (TP+FP) > 0:
        precision = TP/(TP+FP) 
        print("Precision = TP/(TP+FP)", TP/(TP+FP))
    else:
        precision = np.nan
        print("Precision undetermined [TP/(TP+FP)]")

    return recall, precision, TP, FP, FN


def do_validation_adv(detShapeFile, truth_csv_filename, validation_conf_file_name,
                      det_parcel_id_attribute='parcel_id',
                      truth_parcel_id_attribute='parcel_id',
                      tr_date1_attr='from', tr_date2_attr='to',
                      tr_type_attr='type', valid_type=['mow'],
                      yearfirst_det = True, dayfirst_det = False,
                      yearfirst_tr = True, dayfirst_tr = False):

    # load validation parameters
    config = configparser.ConfigParser()
    config.read(validation_conf_file_name)
    validation_interval = list(ast.literal_eval(config['validation']['validation_temporal_range_str']))
    validation_interval = [dateutil.parser.parse(d, yearfirst=True, dayfirst=False) for d in validation_interval]
    fwd_tol_days = np.int(config['validation']['fwd_tol_days'])
    bkw_tol_days = np.int(config['validation']['bkw_tol_days'])

    # load truths
    print("Reading truths ...")
    tr_df = pd.read_csv(truth_csv_filename, sep=';')
    # set indexes of data frame
    tr_df['proc'] = 0
    tr_df['FN'] = 1
    tr_df[truth_parcel_id_attribute] = tr_df.apply(lambda x: str(x[truth_parcel_id_attribute]), axis=1)
    tr_df = tr_df.reset_index(drop=True)

#    # select only valid type
#    valid_idx = tr_df[tr_type_attr].isin(valid_type)
#    print(tr_df)
#    tr_df = tr_df[valid_idx]
#    print(tr_df)

    # set indexes
    tr_df = tr_df.set_index([truth_parcel_id_attribute, tr_date1_attr, tr_date2_attr, tr_df.index])

    # load detections
    print("Reading detections ...")
    ds = ogr.Open(detShapeFile, 1)
    layer = ds.GetLayerByIndex(0)
    det_fids, det_features = list(zip(*[(feature.GetFID(), feature) for feature in layer]))
    # initialize validation data structures
    TP = 0
    FP = 0
    tr_df[['proc', 'FN']] = [0, 1]

    # validation
    for f in det_fids:
        feature = det_features[f]
        if feature['proc'] == 0:
            continue
        p_id = str(feature[det_parcel_id_attribute])
        try:
            pid_tr = tr_df.loc[p_id]
        except KeyError:
            continue
        tr_df.loc[p_id, 'proc'] = 1
        mow_n = int(feature['mow_n'])
        for i in range(1, mow_n+1):
            det_t_start = dateutil.parser.parse(feature['m%d_dstart' % i], yearfirst=yearfirst_det, dayfirst=dayfirst_det)
            det_t_end = dateutil.parser.parse(feature['m%d_dend' % i], yearfirst=yearfirst_det, dayfirst=dayfirst_det)
            if det_t_start > validation_interval[1]:
                continue
            for j in pid_tr.index:
                true_t_start = dateutil.parser.parse(j[0]+" "+"00:00:00", yearfirst=yearfirst_tr, dayfirst=dayfirst_tr) - datetime.timedelta(days=bkw_tol_days)
                true_t_end = dateutil.parser.parse(j[1]+" "+"23:59:59", yearfirst=yearfirst_tr, dayfirst=dayfirst_tr) + datetime.timedelta(days=fwd_tol_days)
                mowing_in_TR, _ = intersection_date(true_t_start, true_t_end, det_t_start, det_t_end)
                if mowing_in_TR:
                    tr_df.loc[(p_id, j[0], j[1]), 'FN'] = 0
                    break
            if mowing_in_TR:
                TP += 1
            else:
                FP += 1
    FN = (tr_df['proc']*tr_df['FN']).sum()
    print("truths proc", tr_df['proc'].sum())

    print("TP =", TP)
    print("FP =", FP)
    print("FN =", FN)
    if (TP+FN) > 0:
        recall = TP/(TP+FN)
        print("Recall    = TP/(TP+FN)", recall)
    else:
        recall = np.nan
        print("Recall undetermined [TP/(TP+FN)]")
    if (TP+FP) > 0:
        precision = TP/(TP+FP)
        print("Precision = TP/(TP+FP)", TP/(TP+FP))
    else:
        precision = np.nan
        print("Precision undetermined [TP/(TP+FP)]")

    return recall, precision, TP, FP, FN



