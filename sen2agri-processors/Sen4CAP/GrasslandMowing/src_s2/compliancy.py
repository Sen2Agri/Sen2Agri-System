import numpy as np
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


def do_compliancy(shp_file_name, compliance_config_filename=None, cnt_crop_code=None, cnt_crop_TR=None, cnt_crop_rule=None):
    # Update of the shape file with compliancies
    # Input: 
    #   - shp_file_name (string): filename used to read attributes and detections, and to store complinacy results
    #   - compliance_config_filename (string): config file name from which read parameters
    #   - cnt_crop_code (list of string): list of crop codes for wich calculate compliancy
    #   - cnt_crop_TR (list of 2-fields tuples): list of 2-fields tuples, one for each crop code.
    #                  Each tuple contains 2 dates in the  such string format: dd/mm/yyyy
    #   - cnt_crop_rule (list of flags): Flags in (0,1): 0 for standard rule, 1 for lithuania rule

    if compliance_config_filename:
        config = configparser.ConfigParser()
        config.read(compliance_config_filename)
        cnt_crop_code = list(map(str.strip, config['compliancy']['crop_codes'].split(',')))
        cnt_crop_TR = list(ast.literal_eval(config['compliancy']['crop_time_intervals']))
        cnt_crop_rule = [np.int(s) for s in config['compliancy']['crop_rule'].split(',')]

    print('cnt_crop_TR:', cnt_crop_TR)
    print("cnt_crop_code", cnt_crop_code)
    print("cnt_crop_rule", cnt_crop_rule)
    
    # verify that each code has a date range
    assert len(cnt_crop_code) == len(cnt_crop_TR)
    assert len(cnt_crop_code) == len(cnt_crop_rule)

    print("Reading features...")
    ds = ogr.Open(shp_file_name, 1)
    layer = ds.GetLayerByIndex(0)
    fids, features = list(zip(*[(feature.GetFID(), feature) for feature in layer]))
    
    print("Sorting features...")
    sort_idx = np.argsort(fids)
    features = np.array(features)[sort_idx]
    fids = np.array(fids)[sort_idx]

    print("Updating features...")
    # conversion in datetime
    cnt_crop_TR = [(datetime.datetime.strptime(date1, '%d/%m/%Y'), datetime.datetime.strptime(date2, '%d/%m/%Y'))
                   for date1, date2 in cnt_crop_TR]

    rule_dict = {str(crop_c): date_r for crop_c, date_r in zip(cnt_crop_code, cnt_crop_TR)}
    flag_dict = {str(crop_c): flag for crop_c, flag in zip(cnt_crop_code, cnt_crop_rule)}
    
    print(cnt_crop_code, cnt_crop_TR, rule_dict)

    for i, f in enumerate(fids):

        feature = features[f]
#        print(i, f, feature.GetFID(), feature['Ori_crop'])
        
        if str(feature['Ori_crop']) not in cnt_crop_code:
            compliancy = -1  # crop code is not recognized
        else:
            mow_n = int(feature['mow_n'])
            if mow_n: # if there are mowing events
                t_start = rule_dict[str(feature['Ori_crop'])][0]
                t_end = rule_dict[str(feature['Ori_crop'])][1]

                pr_date_start, pr_date_end = \
                zip(*[(dateutil.parser.parse(feature['m%d_dstart' % j], yearfirst=True, dayfirst=False),
                       dateutil.parser.parse(feature['m%d_dend' % j], yearfirst=True, dayfirst=False))
                      for j in range(1, 1+mow_n)])
#                print(feature['Ori_crop'], t_start, t_end)
#                print("pr_date_start=", pr_date_start)
#                print("pr_date_end=", pr_date_end)

                if flag_dict[str(feature['Ori_crop'])] == 0: # standard rule
                    mowing_in_interval = False
                    for h in range(mow_n):
                        mowing_in_TR, _ = intersection_date(pr_date_start[h], pr_date_end[h], t_start, t_end)
                        if mowing_in_TR:
                            mowing_in_interval = True
                            continue

                    if mowing_in_interval:
                        compliancy = 1 # cond3
                    else:
                        compliancy = 2 # cond3
                else:  # lithuania rule
                    mowing_in_interval = 0
                    mowing_out_interval = 0
                    for h in range(mow_n):
                        mowing_in_TR, _ = intersection_date(pr_date_start[h], pr_date_end[h], t_start, t_end)
                        if mowing_in_TR:
                            mowing_in_interval += 1
                        else:
                            mowing_out_interval += 1
                    if (mowing_in_interval > 0) and (mowing_out_interval > 0):
                        compliancy = 3 # cond4  # Assessed and not compliant because a mowing occurred
                                       # in the mandatory period and also outside the mandatory period
                    elif (mowing_in_interval > 0) and (mowing_out_interval == 0):
                        compliancy = 1 # cond5  # Assessed and compliant
                    else:
                        compliancy = 2 # cond6  # Assessed and not compliant because no mowing occurred
                                       # in the mandatory period even if a mowing occurred outside 
                                       # the mandatory period
                        
            else: # there are not mowing events
                if feature['proc'] == 0:
                    compliancy = 0 # cond1
                else:
                    compliancy = 2 # cond2

        feature['compl'] = compliancy   
        layer.SetFeature(feature)

    del ds
    
    return 0

