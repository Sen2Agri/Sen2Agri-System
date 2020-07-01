import os, glob
import sys

import re
import configparser
import ast
import pandas as pd

import numpy as np
import scipy
import scipy.ndimage as ndimage

import fusion
import compliancy
import S1_gmd

import dateutil.parser
import datetime

import gdal
from osgeo import ogr
gdal.UseExceptions()
ogr.UseExceptions()


import argparse
try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser
    
import psycopg2
from psycopg2.sql import SQL, Literal
import psycopg2.extras

  
def run_proc(orbit_list, orbit_type, sarDataGlob, re_compile, segmentsFile,
             new_acq_date, older_acq_date=None,
             outputDir=None, outputShapeFile=None,
             seg_parcel_id_attribute=None, tr_parcel_id_attribute=None,
             options_layer_burning=['ALL_TOUCHED=False'],
             invalid_data = 0,
             saturate_sigma=False,
             pfa=3e-5,
             stat_smpl_n=5,
             min_cohe_var=0.024,
             erode_pixels=0,
             non_overlap_interval_days=30,
             locAcqTimeASC='18:00:00',
             locAcqTimeDESC='06:00:00',
             dataType=['COHE'],
             polType=['VH'],
             S1_time_interval=6,
             do_cmpl=False,
             test=False,
             compliancy_conf_file=None):


    print('File list generation from data dir')

    # setting file name parsing tools
    keys = ['file_name', 'satellite', 'master_date', 'master_time', 'slave_date', 'slave_time', 'pol', 'orbit', 'data_type']
    get_par_from_file = re.compile(re_compile)

    # file list generation from data dir
    print("Search paths -->", sarDataGlob)
    file_list = []
    for pth in sarDataGlob:
        print("path:", pth)
        file_list += glob.glob(pth)

    # extract data file names and dates from file list and for specific orbits, polarization and data type
    par_list = S1_gmd.read_file_list(file_list, get_par_from_file, keys, orbit_list, polType, dataType)
    print("par_list", par_list)
    print("orbit_list", get_par_from_file, keys, orbit_list)

    # verify if there are data for specific orbits
    if len(par_list) == 0:
        print("There are NO data for the specific orbit", orbit_list)
        return 2, None

    # put filename and data parameters in a pandas df
    print("Fill pandas structure with file names")
    df = pd.DataFrame.from_records(par_list, columns=keys).drop_duplicates(keys[0])
    print("df ------->", df)

    # add the orbit type column
    orbit_type_dict = {orbit: orbit_t for orbit, orbit_t in zip(orbit_list, orbit_type)}
    df['orbit_type'] = df.apply(lambda x: orbit_type_dict[x['orbit']], axis=1)

    # date and time conversion from str
    df['master_date_time'] = pd.to_datetime(df.apply(lambda x: x['master_date']+'T'+x['master_time'], axis=1), yearfirst=True, dayfirst=False)
    df['slave_date_time'] = pd.to_datetime(df.apply(lambda x: x['slave_date']+'T'+x['slave_time'], axis=1), yearfirst=True, dayfirst=False)
    df.drop(['master_date', 'master_time', 'slave_date', 'slave_time'], axis=1, inplace=True)

    # add acq_time1 acq_time_old columns
    df['old_acq_time'] = df.apply(lambda x: min(x['master_date_time'], x['slave_date_time']), axis=1)
    df['acq_time'] = df.apply(lambda x: max(x['master_date_time'], x['slave_date_time']), axis=1)

    print('File selection based on dates')
    # date selection in the validity date/time range
    if older_acq_date:
        validity_temporal_range_str = [older_acq_date+"T000000", new_acq_date+"T235959"]
    else:
        validity_temporal_range_str = [new_acq_date+"T000000", new_acq_date+"T235959"]

    validity_temporal_range_str = [dateutil.parser.parse(date_str, yearfirst=True, dayfirst=False) for date_str in validity_temporal_range_str]
    select_date_interval = [validity_temporal_range_str[0] - datetime.timedelta(days=S1_time_interval*stat_smpl_n), validity_temporal_range_str[1]]
    print("validity_temporal_range_str", validity_temporal_range_str)
    print("select_date_interval",select_date_interval)
    valid_date_mask = (df['acq_time'] >= select_date_interval[0]) & (df['acq_time'] <= select_date_interval[1])
    print(np.sum(valid_date_mask))
    df = df.loc[valid_date_mask]

    print('Filtered file list')
    print(df)

    # generate lists of files and dates
    data_list = df['file_name'].values

    # remove corrupted files
    data_list = S1_gmd.remove_corrupted_files(list(data_list))

    print(np.array(data_list))

    # make output and tmp directories
    try:
        os.makedirs(outputDir)
    except(OSError):
        print('directory ',outputDir,' exists...moving on')

    output_tmp_dir = os.path.join(outputDir, "tmp")
    try:
        os.makedirs(output_tmp_dir)
    except(OSError):
        print('directory ',output_tmp_dir,' exists...moving on')

    # Get projection from shape file
    ogr_data = ogr.Open(segmentsFile)
    Layer = ogr_data.GetLayer(0)
    spatialRef = Layer.GetSpatialRef()
    dst_srs = spatialRef.ExportToWkt()

    # Some gdalwarp parameters
    resampling = gdal.GRA_Bilinear
    error_threshold = 0.125  # error threshold --> use same value as in gdalwarp

    # make vrt of all data without considering extent of the shape file
    print('Make virtual raster of input data (without considering extent of the shape file)')
    output_vrt_tmp = os.path.join(output_tmp_dir, "data_cube_tmp.vrt")
    images_n = S1_gmd.make_vrt(data_list, dst_srs, output_tmp_dir, output_vrt_tmp, outputBounds=None,
                                  srcNodata=invalid_data, resampling=resampling, error_threshold=error_threshold)

    if images_n > 0:
        vrt_data = gdal.Open(output_vrt_tmp)
    else:
        print('Empty VRT. Does orbit intersect segment layer?')
        return 1, None

    # Get virtual raster extent
    geoTr = vrt_data.GetGeoTransform()
    r_min_x = geoTr[0]
    r_max_y = geoTr[3]
    r_max_x = r_min_x + geoTr[1]*vrt_data.RasterXSize
    r_min_y = r_max_y + geoTr[5]*vrt_data.RasterYSize
    r_extent = (r_min_x, r_min_y, r_max_x, r_max_y)
    del vrt_data

    print("virtual raster extent", (r_min_x, r_min_y, r_max_x, r_max_y))

    # Get shape file extent
    (min_x, max_x, min_y, max_y) = Layer.GetExtent()
    print("shape file extent", (min_x, min_y, max_x, max_y))

    # Calculate extent intersection between virtual raster and shape file
    outputBounds = (np.maximum(min_x, r_min_x),
                    np.maximum(min_y, r_min_y),
                    np.minimum(max_x, r_max_x),
                    np.minimum(max_y, r_max_y))

    print("intesection extent", outputBounds)

    if (outputBounds[0] >= outputBounds[2]) or (outputBounds[1] >= outputBounds[3]):
        print('Empty VRT. Does orbit intersect segment layer?')
        return 1, None

    # make vrt over the intersection extent
    print('Make virtual raster of input data (considering intersection between extensions of i- the shape file, ii- the virtual raster)')
    output_vrt = os.path.join(output_tmp_dir, "data_cube.vrt")
    images_n = S1_gmd.make_vrt(data_list, dst_srs, output_tmp_dir, output_vrt, outputBounds=outputBounds,
                                  srcNodata=invalid_data, resampling=resampling, error_threshold=error_threshold)
    if images_n > 0:
        vrt_data = gdal.Open(output_vrt)
    else:
        print('Empty VRT. Does orbit intersect segment layer?')
        return 1, None

    # Generate segmentation map from input shapefiles
    # -----------------------------------------------

    print('Generate segmentation map from input shapefiles')

    segmentsOutputDir=os.path.join(outputDir, "segments")
    print("segmentsOutputDir", segmentsOutputDir)

    try:
        os.makedirs(segmentsOutputDir)
    except(OSError):
        print('directory ',segmentsOutputDir,' exists...moving on')

    segmentsOutRaster = os.path.join(segmentsOutputDir, os.path.basename(segmentsFile)[:-4]+'_raster.tif')
    print("segmentsOutRaster", segmentsOutRaster)

    # Generate segmentation raster map from input shapefile
    print("layer2mask for segments: ", segmentsFile, output_vrt, segmentsOutRaster)
    burned_pixels, seg_attributes = S1_gmd.layer2mask(segmentsFile, output_vrt, segmentsOutRaster, layer_type='segments', options=options_layer_burning)
    print('segments burned_pixels',burned_pixels)

    # Invert dictionary of se_attribute
    inv_seg_dct = {v[seg_parcel_id_attribute]:k for k,v in seg_attributes.items()}

    # Segment Analysis
    # ----------------

    # load segments
    gdal_data =  gdal.Open(segmentsOutRaster)
    segments = gdal_data.ReadAsArray()
    segments_geo_transform = gdal_data.GetGeoTransform()
    segments_projection = gdal_data.GetProjection()

    # Apply erosion
    # -------------

    ## Segments
    # erode segments

    eroded_segs = np.copy(segments)
    if erode_pixels > 0:
        print('Apply erosion')
        # first step: separate segments
        cross = np.array([[0,1,0],[1,1,1],[0,1,0]])
        min_seg = scipy.ndimage.minimum_filter(segments,footprint=cross)
        max_seg = scipy.ndimage.maximum_filter(segments,footprint=cross)
        segs_borders = (max_seg-min_seg)>0
        eroded_segs[segs_borders] = 0

        # second setp: erode
        if erode_pixels > 1:
            erosion_mask = scipy.ndimage.morphology.binary_erosion(eroded_segs>0, structure=cross,iterations=(erode_pixels-1))
            eroded_segs = eroded_segs*erosion_mask

    # Fill segments
    # -------------

    print('Fill segments')

    # segments
    unique_segments = np.unique(eroded_segs)
    if unique_segments[0] == -1: # the value -1 is used for the background
        unique_segments = unique_segments[1:] # remove the first label which is associated to the background
    print("unique_segments.shape =", unique_segments.shape)
    if np.size(unique_segments) == 0:
        print('No parcels remains after segmentation and erosion')
        return 1, None

    seg_pixels_num = np.array(ndimage.sum(eroded_segs>0, eroded_segs, index=unique_segments))

    # extract data parameters and dates from file_list in the vrt data
    print('Extract data parameters and dates from file_list in the vrt data')
    file_list = [vrt_data.GetRasterBand(i).GetDescription() for i in range(1, vrt_data.RasterCount+1)]
    par_list = S1_gmd.read_file_list(file_list, get_par_from_file, keys, orbit_list, polType, dataType)

    print('Put data parameters in a pandas structure')

    # put data parameters in a pandas df
    print("fill pandas structure")
    vrt_df = pd.DataFrame.from_records(par_list, columns=keys)

    # add the orbit type column
    orbit_type_dict = {orbit: orbit_type for orbit, orbit_type in zip(orbit_list, orbit_type)}
    vrt_df['orbit_type'] = vrt_df.apply(lambda x: orbit_type_dict[x['orbit']], axis=1)

    # date and time conversion from str
    vrt_df['master_date_time'] = pd.to_datetime(vrt_df.apply(lambda x: x['master_date']+'T'+x['master_time'], axis=1), yearfirst=True, dayfirst=False)
    vrt_df['slave_date_time'] = pd.to_datetime(vrt_df.apply(lambda x: x['slave_date']+'T'+x['slave_time'], axis=1), yearfirst=True, dayfirst=False)
    vrt_df.drop(['master_date', 'master_time', 'slave_date', 'slave_time'], axis=1, inplace=True)

    # add acq_time1 old_acq_time
    vrt_df['old_acq_time'] = vrt_df.apply(lambda x: min(x['master_date_time'], x['slave_date_time']), axis=1)
    vrt_df['acq_time'] = vrt_df.apply(lambda x: max(x['master_date_time'], x['slave_date_time']), axis=1)
    vrt_df['old_acq_date'] = vrt_df.apply(lambda x: x['old_acq_time'].date(), axis=1)
    vrt_df['acq_date'] = vrt_df.apply(lambda x: str(x['acq_time'].date()), axis=1)


    # Extraction features
    # -------------------

    print('Extraction of stats')

    #selection of the average method
    stat_p = scipy.ndimage.mean

    # Write list of df and concatenate into a data frame
    data_df = S1_gmd.load_stats(vrt_data, vrt_df, eroded_segs, unique_segments, seg_attributes, seg_parcel_id_attribute, stat_p, invalid_data)

    if len(data_df) == 0:
        print('No valid data collected')
        return 1, None

    # Sort wrt count
    print("Sort wrt count")
    print(datetime.datetime.now())
    data_df.sort_values(['count'], ascending=[False], inplace=True)

    # Aggregate with respect the same date and same parcel_id by keeping the stats associated with the highest count
    print("Aggregate")
    print(datetime.datetime.now())
    #    data_df = data_df.loc[data_df.groupby(['acq_date', 'data_type', seg_parcel_id_attribute])['count'].aggregate('idxmax')]
    data_df = data_df.groupby(['acq_date', 'data_type', seg_parcel_id_attribute]).aggregate('first')

    # Rearrange structure to put dates and data_types as columns
    print("Pivoting")
    print(datetime.datetime.now())
    parcel_fid = (data_df.reset_index()
                         [[seg_parcel_id_attribute, 'fid']]
                         .groupby(seg_parcel_id_attribute)
                         .max() #assume features are burnt in FID asceding order,
                                #thus features with same parcel_id and same geometry
                                #will be rasterised with the greatest FID
                 )
    data_df = pd.pivot_table(data_df, values=['mean','count'], index=[seg_parcel_id_attribute],
                             columns=['orbit', 'pol', 'data_type', 'acq_date'], dropna=False)
    data_df['fid'] = parcel_fid['fid']

    # Move fid to columns and sort by it to match unique_segments
    data_df = data_df.reset_index().set_index(seg_parcel_id_attribute)
    data_df.sort_values('fid', inplace=True)

    # Sort with respect the dates (from early to late)
    print("Sort by Date")
    print(datetime.datetime.now())
    data_df = data_df.sort_values(['acq_date'], axis=1, ascending=True)


    # Read temporal series
    # --------------------

    print('Extraction of temporal series from pandas')

    typical_acq_time = {'asc': locAcqTimeASC, 'desc': locAcqTimeDESC, 'ASC': locAcqTimeASC, 'DESC': locAcqTimeDESC}
    orbit_type_dict = {orbit: orbit_type for orbit, orbit_type in zip(orbit_list,  orbit_type)}

    parcel = data_df.index.values

    # generate array with temporal series
    ampVV_seg = {}
    ampVH_seg = {}
    coheVV_seg = {}
    coheVH_seg = {}

    orbit_list_aux = {x[1] for x in data_df.columns if len(x[1])>0}
    for o in orbit_list_aux:
        if ('VV' in polType) and ('AMP' in dataType):
            ampVV_seg[o] = data_df.xs(key=('mean', 'VV', 'AMP', o), level=(0, 'pol', 'data_type', 'orbit'), axis=1).values
            dates_str = data_df.xs(key=('mean', 'VV', 'AMP', o), level=(0, 'pol', 'data_type', 'orbit'), axis=1).columns.values
            ampVVDateList = [datetime.datetime.combine(pd.Timestamp(d).to_pydatetime(), datetime.time(hour=int(typical_acq_time[orbit_type_dict[o]][:2]), minute=int(typical_acq_time[orbit_type_dict[o]][3:5]))) for d in dates_str]

        if ('VH' in polType) and ('AMP' in dataType):
            ampVH_seg[o] = data_df.xs(key=('mean', 'VH', 'AMP', o), level=(0, 'pol', 'data_type', 'orbit'), axis=1).values
            dates_str = data_df.xs(key=('mean', 'VH', 'AMP', o), level=(0, 'pol', 'data_type', 'orbit'), axis=1).columns.values
            ampVHDateList = [datetime.datetime.combine(pd.Timestamp(d).to_pydatetime(), datetime.time(hour=int(typical_acq_time[orbit_type_dict[o]][:2]), minute=int(typical_acq_time[orbit_type_dict[o]][3:5]))) for d in dates_str]

        if ('VV' in polType) and ('COHE' in dataType):
            coheVV_seg[o] = data_df.xs(key=('mean', 'VV', 'COHE', o), level=(0, 'pol', 'data_type', 'orbit'), axis=1).values
            dates_str = data_df.xs(key=('mean', 'VV', 'COHE', o), level=(0, 'pol', 'data_type', 'orbit'), axis=1).columns.values
            coheVVDateList2 = [datetime.datetime.combine(pd.Timestamp(d).to_pydatetime(), datetime.time(hour=int(typical_acq_time[orbit_type_dict[o]][:2]), minute=int(typical_acq_time[orbit_type_dict[o]][3:5]))) for d in dates_str]
            coheVVDateList1 = [d - datetime.timedelta(days=S1_time_interval) for d in coheVVDateList2]

        if ('VH' in polType) and ('COHE' in dataType):
            coheVH_seg[o] = data_df.xs(key=('mean', 'VH', 'COHE', o), level=(0, 'pol', 'data_type', 'orbit'), axis=1).values
            dates_str = data_df.xs(key=('mean', 'VH', 'COHE', o), level=(0, 'pol', 'data_type', 'orbit'), axis=1).columns.values
            coheVHDateList2 = [datetime.datetime.combine(pd.Timestamp(d).to_pydatetime(), datetime.time(hour=int(typical_acq_time[orbit_type_dict[o]][:2]), minute=int(typical_acq_time[orbit_type_dict[o]][3:5]))) for d in dates_str]
            coheVHDateList1 = [d - datetime.timedelta(days=S1_time_interval) for d in coheVHDateList2]

    if ('VV' in polType) and ('AMP' in dataType):
        ampVV_seg = ampVV_seg[o]
    if ('VH' in polType) and ('AMP' in dataType):
        ampVH_seg = ampVH_seg[o]
    if ('VV' in polType) and ('COHE' in dataType):
        coheVV_seg = coheVV_seg[o]
    if ('VH' in polType) and ('COHE' in dataType):
        coheVH_seg = coheVH_seg[o]

    # make new dictionary seg_attribute including only the parcel analysed {FID: parcel_id}
    seg_attributes = {inv_seg_dct[p]:p for p in parcel}
    # inverti il nuovo dizionario
    inv_seg_dct = {v:k for k,v in seg_attributes.items()}

    # Detection
    # ---------

    print('Mowing Detection')

    # calcolo k_fact associato a pfa
    k_fact = np.sqrt(2)*scipy.special.erfinv(1. - 2.*pfa)
    alpha = 1.0
    print("PFA =", pfa, "k_fact =", k_fact, "alpha =", alpha)

    if ('VV' in polType):
        # Detection on cohe VV
        print('Detection on cohe VV')

        # prepare data: fit
        print('fit')
        data_seg_pred, data_seg_std = S1_gmd.temporal_linear_fit(coheVV_seg, coheVVDateList1, stat_smpl_n, linear_fit=True)

        print('detection')
        # Calculate detection and confidences
        if saturate_sigma:
            # calculate cohestd within segments
            sigma_th = np.sqrt(min_cohe_var/seg_pixels_num[:])
        else:
            sigma_th = None
        coheVV_det_cube_fit = S1_gmd.CFAR_detection(coheVV_seg, k_fact, data_seg_pred, data_seg_std, saturate_sigma_seg=sigma_th)

        # detection on coherences have 1-acquisition delay. Remove this delay
        coheVV_det_cube_fit = np.roll(coheVV_det_cube_fit, -1, axis=1)
        coheVV_det_cube_fit[:,-1] = 0

    if ('VH' in polType):
        # Detection on cohe VH
        print('Detection on cohe VH')

        # prepare data: fit
        print('fit')
        data_seg_pred, data_seg_std = S1_gmd.temporal_linear_fit(coheVH_seg, coheVHDateList1, stat_smpl_n, linear_fit=True)

        print('detection')
        # Calculate detection and confidences
        if saturate_sigma:
            # calculate cohestd within segments
            sigma_th = np.sqrt(min_cohe_var/seg_pixels_num[:])
        else:
            sigma_th = None
        coheVH_det_cube_fit = S1_gmd.CFAR_detection(coheVH_seg, k_fact, data_seg_pred, data_seg_std, saturate_sigma_seg=sigma_th)

        # detection on coherences have 1-acquisition delay. Remove this delay
        coheVH_det_cube_fit = np.roll(coheVH_det_cube_fit, -1, axis=1)
        coheVH_det_cube_fit[:,-1] = 0

    if ('VH' in polType):
        det_cube = np.copy(coheVH_det_cube_fit)

    # Calculate detection reliability index as normalized index
    print('Calculate detection reliability index as normalized index')
    det_cube[det_cube>0] = S1_gmd.norm_fun(det_cube[det_cube>0], alpha, bounds=(0.0, 0.5))

    # calculate fused confidences
    if ('VV' in polType):
        print('Calculate fused confidences')
        aux = np.copy(coheVV_det_cube_fit)
        aux[aux>0] = S1_gmd.norm_fun(aux[aux>0], alpha, bounds=(0.0, 0.5))
        det_cube[det_cube>0] = np.maximum(det_cube[det_cube>0], aux[det_cube>0])

    # Remove nan
    print('Remove nan')
    det_cube[np.logical_not(np.isfinite(det_cube))] = 0

    print('Write results on outputShapeFile')
    if not os.path.exists(outputShapeFile):
        print('Make a clone')
        fusion.cloneAndUpdateShapefile(segmentsFile, outputShapeFile)

    print('Write file')
    fusion.writeDetections_S1(outputShapeFile, unique_segments, det_cube, coheVHDateList1, coheVHDateList2,
                              mission_id='S1', max_dates=4, minimum_interval_days=non_overlap_interval_days)

    if do_cmpl:
        print("Run compliancy calculation")
        compliancy.do_compliancy(outputShapeFile, compliance_config_filename=compliancy_conf_file)

    if test:
        ret_test = {"data_df":data_df, "coheVHDateList1":coheVHDateList1, "coheVHDateList2":coheVHDateList2, "coheVH_seg":coheVH_seg, "det_cube":det_cube, "unique_segments":unique_segments, "inv_seg_dct":inv_seg_dct}
    else:
        ret_test = None

    return 0, ret_test


def main_run(configFile, segmentsFile, data_x_detection, outputDir, new_acq_date, older_acq_date=None,
             orbit_list=[], orbit_type_list=[],
             seg_parcel_id_attribute=None, tr_parcel_id_attribute=None, outputShapeFile=None,
             do_cmpl=False, test=False):

    config = configparser.ConfigParser()
    config.read(configFile)

    re_compile = "(SEN4CAP_L2A_(S[0-9]{1,2})_V([0-9]{8})T([0-9]{6})_([0-9]{8})T([0-9]{6})_([VH]{2})_([0-9]{3})_(?:.+)?(AMP|COHE)\.)"
    # [S1_input_data]
    # re_compile = config['S1_input_data']['re_compile']
    # data_x_detection = list(map(str.strip, config['S1_input_data']['data_x_detection'].split(',')))
    print("re_compile", re_compile)
    print("data_x_detection", data_x_detection)

    # [S1_constants]
    S1_time_interval = np.int(config['S1_constants']['S1_time_interval'])
    SAR_spacing = np.float(config['S1_constants']['SAR_spacing'])
    cohe_ENL = np.float(config['S1_constants']['cohe_ENL'])
    min_cohe_var = np.float(config['S1_constants']['min_cohe_var'])
    locAcqTimeASC = config['S1_constants']['locAcqTimeASC']
    locAcqTimeDESC = config['S1_constants']['locAcqTimeASC']
    print("S1_time_interval", S1_time_interval)
    print("SAR_spacing", SAR_spacing)
    print("cohe_ENL", cohe_ENL)
    print("min_cohe_var", min_cohe_var)
    print("locAcqTimeASC", locAcqTimeASC)
    print("locAcqTimeDESC", locAcqTimeDESC)

    # [S1_processing]
    invalid_data = np.float(config['S1_processing']['invalid_data'])
    saturate_sigma_str = config['S1_processing']['saturate_sigma']
    if saturate_sigma_str == "True":
        saturate_sigma = True
    elif saturate_sigma_str == "False":
        saturate_sigma = False
    else:
        print("saturate_sigma invalid")
        return 0, test_d
    pfa = np.float(config['S1_processing']['pfa'])
    stat_smpl_n = np.int(config['S1_processing']['stat_smpl_n'])
    non_overlap_interval_days = np.int(config['S1_processing']['non_overlap_interval_days'])
    options_layer_burning = list(map(str.strip, config['S1_processing']['options_layer_burning'].split(',')))
    erode_pixels = np.int(config['S1_processing']['erode_pixels'])
    dataType = list(map(str.strip, config['S1_processing']['data_types'].split(',')))
    polType = list(map(str.strip, config['S1_processing']['pol_types'].split(',')))
    print("invalid_data", invalid_data)
    print("saturate_sigma", saturate_sigma)
    print("pfa", pfa)
    print("stat_smpl_n", stat_smpl_n)
    print("non_overlap_interval_days", non_overlap_interval_days)
    print("options_layer_burning", options_layer_burning)
    print("erode_pixels", erode_pixels)
    print('dataType', dataType)
    print('polType', polType)

    # [compliancy]
    if do_cmpl:
        cnt_crop_code = list(map(str.strip, config['compliancy']['crop_codes'].split(',')))
        cnt_crop_TR = list(ast.literal_eval(config['compliancy']['crop_time_intervals']))
        cnt_crop_rule = [np.int(s) for s in config['compliancy']['crop_rule'].split(',')]
    else:
        cnt_crop_code = None
        cnt_crop_TR = None
        cnt_crop_rule = None
    print('cnt_crop_TR:', cnt_crop_TR)
    print("cnt_crop_code", cnt_crop_code)
    print("cnt_crop_rule", cnt_crop_rule)

    print(orbit_list, orbit_type_list)
    done_orbits=[]
    state_orbits=[]
    test_d = []
    for orbit_number, orbit_type in zip(orbit_list, orbit_type_list):
        print("Run: orbit %s, type %s"%(orbit_number,orbit_type))
        ret, test_data = run_proc([orbit_number], [orbit_type], data_x_detection, re_compile, segmentsFile,
                                  new_acq_date, older_acq_date=older_acq_date,
                                  outputDir=outputDir, outputShapeFile=outputShapeFile,
                                  seg_parcel_id_attribute = seg_parcel_id_attribute,
                                  tr_parcel_id_attribute = tr_parcel_id_attribute,
                                  options_layer_burning = options_layer_burning,
                                  invalid_data = invalid_data,
                                  saturate_sigma = saturate_sigma,
                                  pfa = pfa,
                                  stat_smpl_n = stat_smpl_n,
                                  min_cohe_var = min_cohe_var,
                                  erode_pixels = erode_pixels,
                                  non_overlap_interval_days = non_overlap_interval_days,
                                  locAcqTimeASC = locAcqTimeASC,
                                  locAcqTimeDESC = locAcqTimeDESC,
                                  dataType = dataType,
                                  polType = polType,
                                  S1_time_interval = S1_time_interval,
                                  do_cmpl = do_cmpl,
                                  test = test,
                                  compliancy_conf_file=configFile)

        print("Orbit done: ", orbit_number)
        print("Processing state (0: update shp file; 1: no parcels intersecting orbit; 2: no data available for that orbit) -->", ret)
        done_orbits.append(orbit_number)
        state_orbits.append(ret)
        test_d.append(test_data)

    print("orbit list:  ", orbit_list)
    print("orbit done:  ", done_orbits)
    print("orbit state: ", state_orbits)

    return 0, test_d




############################ NEW IMPLEMENTATION ####################################
def parse_date(str):
    return datetime.datetime.strptime(str, "%Y-%m-%d").date()

class RadarProduct(object):
    def __init__(self, dt, name, orbit_type_id, polarization, radar_product_type, path, orbit_id):
        self.name = name
        self.orbit_type_id = orbit_type_id
        self.polarization = polarization
        self.radar_product_type = radar_product_type
        self.path = path
        self.orbit_id = f'{orbit_id:03}'

        (year, week, _) = dt.isocalendar()
        self.year = year
        self.week = week

class S4CConfig(object):
    def __init__(self, args):
        parser = ConfigParser()
        parser.read([args.s4c_config_file])

        self.host = parser.get("Database", "HostName")
        self.dbname = parser.get("Database", "DatabaseName")
        self.user = parser.get("Database", "UserName")
        self.password = parser.get("Database", "Password")

        self.site_id = args.site_id
        self.config_file = args.config_file
        self.input_shape_file = args.input_shape_file
        self.output_data_dir = args.output_data_dir
        self.new_acq_date = args.new_acq_date
        self.older_acq_date = args.older_acq_date
        # self.orbit_list_file = args.orbit_list_file
        self.seg_parcel_id_attribute = args.seg_parcel_id_attribute
        self.output_shapefile = args.output_shapefile
        self.do_cmpl = args.do_cmpl
        self.test = args.test
        

        if args.season_start:
            self.season_start = parse_date(args.season_start)
            print("Season_start = ", args.season_start)
        
        if args.season_end:        
            self.season_end = parse_date(args.season_end)
            print("Season_end = ", args.season_end)            

def get_s1_products(config, conn, prds_list):
    with conn.cursor() as cursor:
        prds_names_list=[]
        if prds_list is None or len(prds_list) == 0 :
            query = SQL(
                """
                with products as (
                    select product.site_id,
                        product.name,
                        case
                            when substr(split_part(product.name, '_', 4), 2, 8) > substr(split_part(product.name, '_', 5), 1, 8) then substr(split_part(product.name, '_', 4), 2, 8)
                            else substr(split_part(product.name, '_', 5), 1, 8)
                        end :: date as date,
                        coalesce(product.orbit_type_id, 1) as orbit_type_id,
                        split_part(product.name, '_', 6) as polarization,
                        product.processor_id,
                        product.product_type_id,
                        substr(product.name, length(product.name) - strpos(reverse(product.name), '_') + 2) as radar_product_type,
                        product.orbit_id,
                        product.full_path
                    from product where product.satellite_id = 3 and product.site_id = {}
                )
                select products.date,
                        products.name,
                        products.orbit_type_id,
                        products.polarization,
                        products.radar_product_type,
                        products.full_path, 
                        products.orbit_id
                from products
                where date between {} and {}
                order by date;
                """
            )

            site_id_filter = Literal(config.site_id)
            start_date_filter = Literal(config.season_start)
            end_date_filter = Literal(config.season_end)
            query = query.format(site_id_filter, start_date_filter, end_date_filter)
            # print(query.as_string(conn))
        else :
            for prd in prds_list:
                prds_names_list.append(os.path.splitext(os.path.basename(prd))[0])
                
            if len (prds_names_list) > 1:
                prdsSubstr = tuple(prds_names_list)
            else :
                prdsSubstr = "('{}')".format(prds_names_list[0])
        
            query= """
                with products as (
                    select product.site_id,
                        product.name,
                        case
                            when substr(split_part(product.name, '_', 4), 2, 8) > substr(split_part(product.name, '_', 5), 1, 8) then substr(split_part(product.name, '_', 4), 2, 8)
                            else substr(split_part(product.name, '_', 5), 1, 8)
                        end :: date as date,
                        coalesce(product.orbit_type_id, 1) as orbit_type_id,
                        split_part(product.name, '_', 6) as polarization,
                        product.processor_id,
                        product.product_type_id,
                        substr(product.name, length(product.name) - strpos(reverse(product.name), '_') + 2) as radar_product_type,
                        product.orbit_id,
                        product.full_path
                    from product where product.satellite_id = 3 and product.name in {}
                )
                select products.date,
                        products.name,
                        products.orbit_type_id,
                        products.polarization,
                        products.radar_product_type,
                        products.full_path, 
                        products.orbit_id
                from products
                order by date;""".format(prdsSubstr)
            #print(query)
        
        # execute the query
        cursor.execute(query)            
            
        results = cursor.fetchall()
        conn.commit()

        products = []
        # We are performing this search to have a warning on not present products but also to have the same order of products as in the inputs
        if len(prds_names_list) > 0 :
            for i in range(len(prds_names_list)):
                prd_name = prds_names_list[i]
                prd = prds_list[i]
                prdAdded = False
                for (dt, name, orbit_type_id, polarization, radar_product_type, full_path, orbit_id) in results:
                    if os.path.splitext(os.path.basename(prd_name))[0] == name : 
                        products.append(RadarProduct(dt, name, orbit_type_id, polarization, radar_product_type, os.path.normpath(prd), orbit_id))
                        prdAdded = True
                        break
                if prdAdded == False :
                    print ("Product {} was not found in the database!!!".format(prd))
        else :
            for (dt, name, orbit_type_id, polarization, radar_product_type, full_path, orbit_id) in results:
                products.append(RadarProduct(dt, name, orbit_type_id, polarization, radar_product_type, full_path, orbit_id))

        return products

def getPathsAndOrbits(s4cConfig, conn, input_products_list) :
    products = get_s1_products(s4cConfig, conn, input_products_list)
    
    print ("Config file is {}".format(s4cConfig.config_file))
    
    config = configparser.ConfigParser()
    config.read(s4cConfig.config_file)
    
    orbit_list_filter = None
    orbit_type_list_filter = None
    if config.has_option('GENERAL_CONFIG', 's1_orbit_list_filter') : 
        orbit_list_filter = config['GENERAL_CONFIG']['s1_orbit_list_filter']
    if config.has_option('GENERAL_CONFIG', 's1_orbit_type_list_filter') : 
        orbit_type_list_filter = config['GENERAL_CONFIG']['s1_orbit_type_list_filter']
        
    if orbit_list_filter and orbit_type_list_filter :
        orbit_list_filter = list(map(str.strip, orbit_list_filter.split(',')))
        orbit_type_list_filter = list(map(str.strip, orbit_type_list_filter.split(',')))
    
    if orbit_list_filter : 
        print("Filtering orbit_list :", orbit_list_filter)
        print("Filtering orbit_type_list :", orbit_type_list_filter)
    
    orbit_list_tmp = []
    orbit_type_list_tmp = []
    data_x_detection = []
    for product in products:
        data_x_detection.append(product.path) 
        if product.orbit_id not in orbit_list_tmp:
            orbit_list_tmp.append(product.orbit_id)
            if product.orbit_type_id == 1 : 
                orbit_type_list_tmp.append("ASC")
            else :
                orbit_type_list_tmp.append("DESC")

    print("orbit_list detected:", orbit_list_tmp)
    print("orbit_type_list detected", orbit_type_list_tmp)

    orbit_list = []
    orbit_type_list = []
    
    if (orbit_list_filter and len(orbit_list_filter) > 0) :
        # Check if the orbit filters are valid just to give a warning 
        for i in range(len(orbit_list_filter)):
            found = False
            for j in range(len(orbit_list_tmp)):
                if orbit_list_tmp[j] == orbit_list_filter[i] and orbit_type_list_tmp[j] == orbit_type_list_filter[i]:
                    found = True
                    break  
            if not found :
                print("ATTENTION: Orbit {} {} in the filtering orbits list was not found in the products extracted orbits!".format(orbit_list_filter[i], orbit_type_list_filter[i]))

        orbit_list = orbit_list_filter
        orbit_type_list = orbit_type_list_filter
    else :
        orbit_list = orbit_list_tmp
        orbit_type_list = orbit_type_list_tmp
        
    print("Final orbit_list to use:", orbit_list)
    print("Final orbit_type_list to use", orbit_type_list)

    return data_x_detection, orbit_list, orbit_type_list

def main() :
    parser = argparse.ArgumentParser(description="Executes grassland mowing S1 detection")
    parser.add_argument('-c', '--s4c-config-file', default='/etc/sen2agri/sen2agri.conf', help="Sen4CAP system configuration file location")
    parser.add_argument('-s', '--site-id', help="The site id")
    parser.add_argument('-f', '--config-file', default = "/usr/share/sen2agri/S4C_L4B_GrasslandMowing/config.ini", help="Grassland mowing parameters configuration file location")
    parser.add_argument('-i', '--input-shape-file', help="The input shapefile GSAA")
    parser.add_argument('-o', '--output-data-dir', help="Output data directory")
    parser.add_argument('-e', '--new-acq-date', help="The new acquisition date")
    parser.add_argument('-b', '--older-acq-date', help="The older acquisition date")
    # parser.add_argument('-l', '--orbit-list-file', help="The orbit list file")    # NOT USED ANYMORE - ORBITS DETECTED FROM THE DATABASE
    parser.add_argument('-l', '--input-products-list', nargs='+', help="The list of S1 L2 products")   
    parser.add_argument('-a', '--seg-parcel-id-attribute', help="The SEQ unique ID attribute name", default="NewID")
    parser.add_argument('-x', '--output-shapefile', help="Output shapefile")
    parser.add_argument('-m', '--do-cmpl', help="Run compliancy")
    parser.add_argument('--season-start', help="Season start")
    parser.add_argument('--season-end', help="Season end")
    parser.add_argument('-t', '--test', help="Run test")
    
    args = parser.parse_args()
    
    s4cConfig = S4CConfig(args)
    
    with psycopg2.connect(host=s4cConfig.host, dbname=s4cConfig.dbname, user=s4cConfig.user, password=s4cConfig.password) as conn:
        data_x_detection, orbit_list, orbit_type_list = getPathsAndOrbits(s4cConfig, conn, args.input_products_list)
        print("orbit_list to use:", orbit_list)
        print("orbit_type_list to use", orbit_type_list)
        
        ret, _ = main_run(s4cConfig.config_file, s4cConfig.input_shape_file, data_x_detection, s4cConfig.output_data_dir, s4cConfig.new_acq_date, 
                       older_acq_date = s4cConfig.older_acq_date, orbit_list = orbit_list, orbit_type_list=orbit_type_list,
                       seg_parcel_id_attribute = s4cConfig.seg_parcel_id_attribute, outputShapeFile=s4cConfig.output_shapefile,
                       do_cmpl=s4cConfig.do_cmpl, test=s4cConfig.test)

if __name__== "__main__":

    os.environ['SHAPE_ENCODING'] = "utf-8"
    
    main()

#if __name__== "__main__":
#    if len(sys.argv)==11:
#        config_file = sys.argv[1]
#        input_shape_file = sys.argv[2]
#        output_data_dir =  sys.argv[3]
#        new_acq_date =   sys.argv[4]
#        older_acq_date =  sys.argv[5]
#        orbit_list_file = sys.argv[6]
#        seg_parcel_id_attribute = sys.argv[7]
#        outputShapeFile = sys.argv[8]
#        do_cmpl_str = sys.argv[9]
#        if do_cmpl_str == "True":
#            do_cmpl = True
#        elif do_cmpl_str == "False":
#            do_cmpl = False
#        else:
#            print("do_cmpl par invalid")
#        test = np.bool(sys.argv[10])
#        if test == "True":
#            test = True
#        elif test == "False":
#            test = False
#        else:
#            print("test par invalid")
#        print("do_cmpl", do_cmpl)
#        print("test", test)
#    else:
#        print("Expected input:")
#        print("1) config_file")
#        print("2) input_shape_file")
#        print("3) output_data_dir")
#        print("4) new_acq_date")
#        print("5) older_acq_date")
#        print("6) orbit_list_file")
#        print("7) seg_parcel_id_attribute")
#        print("8) outputShapeFile")
#        print("9) do_cmpl")
#        print("10) test")
#        raise Exception("Just 10 arguments are expected.", len(sys.argv)-1, "arguments are passed.")
#
#    config = configparser.ConfigParser()
#    config.read(orbit_list_file)
#
#    orbit_list = list(map(str.strip, config['orbits']['orbit_list'].split(',')))
#    orbit_type_list = list(map(str.strip, config['orbits']['orbit_type_list'].split(',')))
#    print("orbit_list:", orbit_list)
#    print("orbit_type_list", orbit_type_list)
#
#    ret, _ = main_run(config_file, input_shape_file, output_data_dir, new_acq_date,
#                      older_acq_date = older_acq_date, orbit_list=orbit_list, orbit_type_list=orbit_type_list,
#                      seg_parcel_id_attribute = seg_parcel_id_attribute, outputShapeFile=outputShapeFile,
#                      do_cmpl=do_cmpl, test=test)
    

