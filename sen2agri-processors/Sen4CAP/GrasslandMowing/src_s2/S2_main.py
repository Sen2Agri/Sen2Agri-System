import os, glob
import sys

import re
import configparser
import ast
import pandas as pd

import numpy as np
import scipy
import scipy.ndimage as ndimage

import model_lib
import fusion
import compliancy
import S2_gmd

import dateutil.parser
import datetime

import gdal
from osgeo import ogr, osr
gdal.UseExceptions()
osr.UseExceptions()
ogr.UseExceptions()

import argparse
try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser
    
import psycopg2
from psycopg2.sql import SQL, Literal
import psycopg2.extras
import fnmatch


def run_proc(tile_number, S2DataGlob, re_compile, segmentsFile,
             new_acq_date, older_acq_date=None,
             outputDir=None, modelDir=None, outputShapeFile=None,
             seg_parcel_id_attribute=None,
             options_layer_burning=['ALL_TOUCHED=False'],
             erode_pixels = 0,
             prod_type_list = ['SNDVI'],
             sc_fact = None,
             corrupted_th = None,
             invalid_data = None,
             decreasing_abs_th = None,
             decreasing_rate_th = None,
             increasing_rate_th = None,
             high_abs_th = None,
             low_abs_th = None,
             apply_model = True,
             model_dict = None,
             no_mowing_after_det=30,
             non_overlap_interval_days=45,
             locAcqTime='10:30:00',
             S2_time_interval=5,
             do_cmpl=False,
             stat_smpl_n=0,
             compliancy_conf_file=None,
             test=False):

    print('File list generation from data dir')

    # setting file name parsing tools
    keys = ['file_name', 'data_type', 'acq_date', 'acq_time', 'tile_code']
    get_par_from_file = re.compile(re_compile)

    # file list generation from data dir
    print("Search paths -->", S2DataGlob)
    file_list = []
    for pth in S2DataGlob:
        print("path:", pth)
        file_list += glob.glob(pth)

    # remove NOTV directories
    file_list = [f for f in file_list if not '_NOTV/' in f]

    # extract data file names and dates from file list and for specific orbits
    par_list = S2_gmd.read_file_list(file_list, get_par_from_file, keys, tile_number, orbit_field_label='tile_code')
    print("size par_list", len(par_list))

    # verify if there are data for specific tile
    if len(par_list) == 0:
        print("There are NO data for the specific tile", tile_number)
        return 2, None

    # put filename and data parameters in a pandas df
    print("fill pandas structure")
    df = pd.DataFrame.from_records(par_list, columns=keys).drop_duplicates(keys[0])
#    print(df)

    # date and time conversion from str
    df['acq_date_time' ] = pd.to_datetime(df.apply(lambda x: x['acq_date']+'T'+x['acq_time'], axis=1), yearfirst=True, dayfirst=False)

    # remove all indexes not required
    valid_prod_type_mask = False
    for pt in prod_type_list:
        valid_prod_type_mask = np.logical_or(valid_prod_type_mask, df['data_type'] == pt)
    df = df.loc[valid_prod_type_mask]

    print('File selection based on dates')
    # date selection in the validity date/time range
    if older_acq_date:
        validity_temporal_range_str = [older_acq_date+"T000000", new_acq_date+"T235959"]
    else:
        validity_temporal_range_str = [new_acq_date+"T000000", new_acq_date+"T235959"]

    validity_temporal_range_str = [dateutil.parser.parse(date_str, yearfirst=True, dayfirst=False) for date_str in validity_temporal_range_str]
    select_date_interval = [validity_temporal_range_str[0] - datetime.timedelta(days=S2_time_interval*stat_smpl_n), validity_temporal_range_str[1]]
    print("validity_temporal_range_str", validity_temporal_range_str)
    print("select_date_interval",select_date_interval)
    valid_date_mask = (df['acq_date_time'] >= select_date_interval[0]) & (df['acq_date_time'] <= select_date_interval[1])
    print(np.sum(valid_date_mask))
    df = df.loc[valid_date_mask]

    print('Filtered file list')
#    print(df)

    # generate lists of files and dates
    data_list = df['file_name'].values

    # remove corrupted files
    data_list = S2_gmd.remove_corrupted_files(list(data_list))

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
    print(segmentsFile)

    # Get projection from shape file
    ogr_data = ogr.Open(segmentsFile)
    Layer = ogr_data.GetLayer(0)
    spatialRef = Layer.GetSpatialRef()
    dst_srs = spatialRef.ExportToWkt()

    # Some gdalwarp parameters
    resampling = gdal.GRA_Bilinear
    error_threshold = 0.125  # error threshold --> use same value as in gdalwarp

    # make vrt of all data without considering extension of the shape file
    print('Make virtual raster of input data (without considering extension of the shape file)')
    output_vrt_tmp = os.path.join(output_tmp_dir, "data_cube_tmp.vrt")
    images_n = S2_gmd.make_vrt(data_list, dst_srs, output_tmp_dir, output_vrt_tmp, outputBounds=None,
                                  srcNodata=invalid_data, resampling=resampling, error_threshold=error_threshold)

    if images_n > 0:
        vrt_data = gdal.Open(output_vrt_tmp)
    else:
        print('Empty VRT. Does tile intersect segment layer?')
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
        print("Intesection extent between shapefile and VRT is empty")
        return 1, None

    # make vrt over the intersection extent
    print('Make virtual raster of input data (considering intersection between extensions of i- the shape file, ii- the virtual raster)')
    output_vrt = os.path.join(output_tmp_dir, "data_cube.vrt")
    images_n = S2_gmd.make_vrt(data_list, dst_srs, output_tmp_dir, output_vrt, outputBounds=outputBounds,
                                  srcNodata=invalid_data, resampling=resampling, error_threshold=error_threshold)
    if images_n > 0:
        vrt_data = gdal.Open(output_vrt)
    else:
        print('Empty VRT. Does tile intersect segment layer?')
        return 1, None

    print("Images in VRT:", images_n)

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
    burned_pixels, seg_attributes = S2_gmd.layer2mask(segmentsFile, output_vrt, segmentsOutRaster, layer_type='segments', options=options_layer_burning)
    print('segments burned_pixels',burned_pixels)
    if burned_pixels == 0:
        print('No parcels burned in the segmentation')
        return 1, None

    # check on duplication of the parcel IDs
    list_parcel_id_attributes = [v[seg_parcel_id_attribute] for k, v in seg_attributes.items()]
    if len(list_parcel_id_attributes) != len(set(list_parcel_id_attributes)):
        print("There are parcel IDs duplicated!")
        return 4, None

    # inverti dizionari
    print("inverti dizionari")
    inv_seg_dct = {v[seg_parcel_id_attribute]:k for k,v in seg_attributes.items()}

##################################################################################################

    # Set/Load model_map
    # ------------------

    # model empty with parcel_id from shape file, that is filled with NAN
#    last_vi_df = pd.DataFrame(columns=['SNDVI', 'SLAI', 'SFAPAR', 'SNDVI_doy', 'SLAI_doy', 'SFAPAR_doy'], index=[v[seg_parcel_id_attribute] for k, v in seg_attributes.items()], dtype='float32')
    last_vi_df = pd.DataFrame(columns=['SNDVI0', 'SNDVI1', 'SLAI0', 'SLAI1', 'SFAPAR0', 'SFAPAR1', 'SNDVI0_doy', 'SNDVI1_doy', 'SLAI0_doy', 'SLAI1_doy', 'SFAPAR0_doy', 'SFAPAR1_doy'], index=[v[seg_parcel_id_attribute] for k, v in seg_attributes.items()], dtype='float32')

    # Load model in the last_vi_df structure
    # The loaded model could have parcel not included in area we are processing or even it could not have some parcels.
    # For these reasons, we consider all parcels extracted from the segmentation, filling them with model values extracted from the loaded model

    # load last_vi_df
    last_vi_file = model_lib.hist_data_file_name_gen([tile_number], segmentsFile, prod_type_list)
    last_vi_file_abs = os.path.join(os.path.join(outputDir, modelDir), last_vi_file)
    print("Last VI data file name:", last_vi_file_abs)
    if os.path.exists(last_vi_file_abs):
        print("Last VI data is available")
        loaded_last_vi_df = pd.read_pickle(last_vi_file_abs)
    else:
        print("Last VI data must be initialized")
        try:
            os.makedirs(os.path.join(os.path.join(outputDir, modelDir)))
        except (OSError):
            print("output dir exixts...moving on")
        loaded_last_vi_df = last_vi_df.copy()
        loaded_last_vi_df.to_pickle(last_vi_file_abs)

    # find the parcel_id's which are both in the loaded model and in the segmentation
    common_index = loaded_last_vi_df.index.intersection(last_vi_df.index)

    # copy all model values from loaded model to the model structure derived from segmentation
    last_vi_df.loc[common_index] = loaded_last_vi_df.loc[common_index]

##############################################################################################


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

    print("eroded_segs", eroded_segs)
    print("min, max:", np.min(eroded_segs), np.max(eroded_segs))


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

    # extract data parameters and dates from file_list in the vrt data
    print('Extract data parameters and dates from file_list in the vrt data')
    file_list = [vrt_data.GetRasterBand(i).GetDescription() for i in range(1, vrt_data.RasterCount+1)]
    par_list = S2_gmd.read_file_list(file_list, get_par_from_file, keys, tile_number, orbit_field_label='tile_code')

    # put filename and data parameters in a pandas df
    print('Put data parameters in a pandas structure')
    vrt_df = pd.DataFrame.from_records(par_list, columns=keys)

    # generation of the column acq_date_time in string format
    vrt_df['acq_date_time'] = vrt_df.apply(lambda x: x['acq_date']+'T'+x['acq_time'], axis=1)

    print('Extraction of stats')
    #selection of the average method
    stat_p = scipy.ndimage.mean

    # Write list of df
    data_df = S2_gmd.load_stats(vrt_data, vrt_df, eroded_segs, unique_segments, seg_attributes, seg_parcel_id_attribute, stat_p, invalid_data)

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
                             columns=['data_type', 'acq_date'], dropna=False)
    data_df['fid'] = parcel_fid['fid']

    # Move fid to columns and sort by it to match unique_segments
    data_df = data_df.reset_index().set_index(seg_parcel_id_attribute)
    data_df.sort_values('fid', inplace=True)

    # Sort with respect the dates (from early to late)
    print("Sort by Date")
    print(datetime.datetime.now())
    data_df = data_df.sort_values(['acq_date'], axis=1, ascending=True)

######################################################################

    # Sort the last_vi_df rows to match with data_df rows
    last_vi_df = last_vi_df.loc[data_df.index.values]

######################################################################


    # Read temporal series
    # --------------------
    print('Extraction of temporal series from pandas')

    parcel = data_df.index.values

    # generate array with temporal series
    VI_seg = []
    for pt, sf, ct in zip(prod_type_list, sc_fact, corrupted_th):
        VI_data = data_df.xs(key=('mean', pt), level=(0, 'data_type'), axis=1).values
        dates_str = data_df.xs(key=('mean', pt), level=(0, 'data_type'), axis=1).columns.values

        VIDateList = [dateutil.parser.parse(d+" "+locAcqTime, yearfirst=True, dayfirst=False) for d in dates_str]

#        VI_seg.append({'data_type': pt, 'date_list': VIDateList, 'VI': VI_data, 'sc_fact': sf, 'corrupted_th': ct})
#####################################################################################################################################################
        VI_seg.append({'data_type': pt, 'date_list': VIDateList, 'VI': VI_data, 'sc_fact': sf, 'corrupted_th': ct,
                       'last_VI': np.hstack((last_vi_df[pt+'0'].values[:,None], last_vi_df[pt+'1'].values[:,None])),
                       'last_VI_doy': np.hstack((last_vi_df[pt+'0'+'_doy'].values[:,None], last_vi_df[pt+'1'+'_doy'].values[:,None]))})
#####################################################################################################################################################

    # apply scale factors and remove corrupted VI
    for d in VI_seg:
        d['VI'] /= d['sc_fact']
        d['VI'][ d['VI'] < d['corrupted_th'] ] = np.nan


    # Detection
    # -----------------

    print("Detection")

    # assume that the processing is done with only one VI and therefore len(VI_seg) == 1
    VI_index = 0
    print("Detection applied to VI:", VI_seg[VI_index]['data_type'])
    NDVI_seg = np.array(VI_seg[VI_index]['VI'])
    NDVIDateList = VI_seg[VI_index]['date_list']
    DOYList = [d.timetuple().tm_yday for d in NDVIDateList]
    NDVI_last = np.array(VI_seg[VI_index]['last_VI'])
    NDVI_last_doy = np.round(np.array(VI_seg[VI_index]['last_VI_doy'])).astype("uint32")
    NDVI_corrupted_th = VI_seg[VI_index]['corrupted_th']
    NDVI_last_hist = np.zeros_like(NDVI_seg)

#    # Fit of the model to the available DOYs
#    x = model_dict[VI_seg[VI_index]['data_type']][:,0]
#    model_dbl_l = model_dict[VI_seg[VI_index]['data_type']][:,1]
################################################################################################################

    if apply_model:
        # Fit of the model to the available DOYs
        x = model_dict[VI_seg[VI_index]['data_type']][:,0]
        model_dbl_l = model_dict[VI_seg[VI_index]['data_type']][:,1]

        # final no-mowing model
        NDVI_nomow_model_ = np.interp(DOYList, x, model_dbl_l)
    else:
        x = np.arange(1, 366+1)
        model_dbl_l = np.ones_like(x).astype('int16')
        NDVI_nomow_model_ = np.ones((len(NDVIDateList),), dtype='float32')

    # Initalize model, force the minimum of the model to pass close to 0 (0.001)
    #  - for parcel with NDVI_last  = NAN, initalize model by forcing the minimum of the model to pass close to 0 (0.001)
    #  - for parcel with NDVI_last != NAN, initalize model by forcing the model to pass through the NDVI_last at the NDVI_last_doy

    # - for all parcels
    NDVI_nomow_model = np.repeat((NDVI_nomow_model_[None,:]/np.min(NDVI_nomow_model_)*0.001),NDVI_seg.shape[0], axis=0)

    # - just for parcel with NDVI_before_last != NAN
    no_nan_idx = list(np.where(np.isfinite(NDVI_last[:,0]))[0])
    if len(no_nan_idx) > 0:
        NDVI_nomow_model[no_nan_idx] = NDVI_nomow_model_[None,:] / model_dbl_l[NDVI_last_doy[no_nan_idx,0]][:,None] * NDVI_last[no_nan_idx,0][:,None]

    # - just for parcel with NDVI_last != NAN
    print("NDVI_last shape::::::::::", NDVI_last.shape)
    no_nan_idx = list(np.where(np.isfinite(NDVI_last[:,1]))[0])
    if len(no_nan_idx) > 0:
        NDVI_nomow_model[no_nan_idx] = NDVI_nomow_model_[None,:] / model_dbl_l[NDVI_last_doy[no_nan_idx,1]][:,None] * NDVI_last[no_nan_idx,1][:,None]

################################################################################################################

    # diff
    diff_seg = np.zeros_like(NDVI_seg)*np.nan
    for i in range(len(NDVI_seg)):
        ind_mas = np.where(np.isfinite(NDVI_seg[i,:]))
        if np.size(ind_mas) > 0:
            val = (NDVI_seg[i, ind_mas])[0]
            diff = (val - np.roll(val, 1))
            diff[0] = -10000
            diff_seg[i,ind_mas] = diff

    # Detection on model

    NDVI_det_cube = np.zeros_like(NDVI_seg)

    if apply_model:
        for t in range(NDVI_det_cube.shape[1]):

            # Detection at t
            # Detection of a mowing if there are 1) NDVI decrement wrt adaptive model AND 2) wrt the 95 percentile model
            NDVI_det_cube[:,t] = np.minimum( np.maximum((NDVI_nomow_model[:,t] - NDVI_seg[:,t] - decreasing_abs_th)/NDVI_nomow_model[:,t], 0),  # C1: decremento wrt adaptive mode
                                             np.maximum((NDVI_nomow_model_[t]     - NDVI_seg[:,t] - decreasing_abs_th)/NDVI_nomow_model_[t], 0) )   # C1: decremento wrt the 95 percentile model

            # update model only for the parcel with detected mowing
            to_update_parc = list(np.where(NDVI_det_cube[:,t] > 0)[0])
            increased_NDVI = list(np.where(diff_seg[:,t] > 0)[0])
            to_update_parc = np.array(to_update_parc + increased_NDVI)
            if len(to_update_parc) > 0:
                NDVI_nomow_model[to_update_parc, t:] *= (NDVI_seg[to_update_parc, t]/NDVI_nomow_model[to_update_parc, t])[:,None]

            # update NDVI_last and NDVI_doy and NDVI_last_hist
            if t > 0:
                NDVI_last_hist[:,t] = NDVI_last_hist[:,t-1]
            valid_vi_indexes = list(np.where(np.isfinite(NDVI_seg[:,t]))[0])
            if len(valid_vi_indexes) > 0:
                NDVI_last[valid_vi_indexes,0] = NDVI_last[valid_vi_indexes,1]
                NDVI_last_doy[valid_vi_indexes,0] = NDVI_last_doy[valid_vi_indexes,1]
                NDVI_last[valid_vi_indexes,1] = NDVI_seg[valid_vi_indexes,t]
                NDVI_last_doy[valid_vi_indexes,1] = DOYList[t]
                NDVI_last_hist[valid_vi_indexes, t] = NDVI_last[valid_vi_indexes,1]
    else:
        for t in range(NDVI_det_cube.shape[1]):

            # Detection at the time t of mowing occurred between NDVI_before_last = NDVI_last[0,:] and NDVI_last = NDVI_last[1,:]
            # Detection of a mowing if the decreasing angle (alpha = atan(delta_VI/delta_doy)) is enough small, that is lower than decreasing_rate_th. (VI decreasing enough fast.)
            NDVI_det_cube[:,t] = np.maximum((NDVI_last[:,0] - NDVI_last[:,1] - decreasing_abs_th)/NDVI_last[:,1], 0)             # last VI decreasing is at least decreasing_abs_th
            NDVI_det_cube[:,t] *= ((NDVI_last[:,1] - NDVI_last[:,0]) / (NDVI_last_doy[:,1] - NDVI_last_doy[:,0]) < decreasing_rate_th)   # detection only if the decreasing is enough fast
            NDVI_det_cube[:,t] *= ((NDVI_seg[:,t] - NDVI_last[:,1]) / (DOYList[t] - NDVI_last_doy[:,1]) < increasing_rate_th)   # detection only if the successive VI increasing is not too fast
            NDVI_det_cube[:,t] *= (NDVI_last[:,1] < high_abs_th)
            NDVI_det_cube[:,t] *= (NDVI_last[:,0] > low_abs_th)

            # update always NDVI_last and NDVI_doy and NDVI_last_hist
            if t > 0:
                NDVI_last_hist[:,t] = NDVI_last_hist[:,t-1]
            valid_vi_indexes = list(np.where(np.isfinite(NDVI_seg[:,t]))[0])
            if len(valid_vi_indexes) > 0:
                NDVI_last[valid_vi_indexes,0] = NDVI_last[valid_vi_indexes,1]
                NDVI_last_doy[valid_vi_indexes,0] = NDVI_last_doy[valid_vi_indexes,1]
                NDVI_last[valid_vi_indexes,1] = NDVI_seg[valid_vi_indexes,t]
                NDVI_last_doy[valid_vi_indexes,1] = DOYList[t]
                NDVI_last_hist[valid_vi_indexes, t] = NDVI_last[valid_vi_indexes,1]

    # save last_VI and last_VI_doy
    # NDVI_last and NDVI_last_doy contains the last VI and doy values to be saved
    last_vi_df[prod_type_list[VI_index]+'0'] = NDVI_last[:,0]
    last_vi_df[prod_type_list[VI_index]+'1'] = NDVI_last[:,1]
    last_vi_df[prod_type_list[VI_index]+'0'+'_doy'] = NDVI_last_doy[:,0]
    last_vi_df[prod_type_list[VI_index]+'1'+'_doy'] = NDVI_last_doy[:,1]

    # updated "last_vi_df" have higher priority than "loaded_last_vi_df"
    loaded_last_vi_df['prty'] = 1
    last_vi_df['prty'] = 2

    # generate merged last VI and save them
    ret_last_vi_df = pd.concat([loaded_last_vi_df, last_vi_df], axis='rows').reset_index().sort_values('prty', ascending=False).groupby(['index']).aggregate('first').drop('prty', axis=1)
    ret_last_vi_df.to_pickle(last_vi_file_abs)

    # detection have 1-(cloud-free)-acquisition delay. Remove this delay
    det_cube = np.zeros_like(NDVI_det_cube)
    for i in range(NDVI_seg.shape[0]):
        val_ind=np.where(np.isfinite(NDVI_seg[i,:]))[0]
        if len(val_ind)>0:
            det_cube[i,val_ind[:-1]] = NDVI_det_cube[i,val_ind[1:]]
    del NDVI_det_cube

    # calculate detection reliability index as normalized index
    print("Confidence normalization")
    alpha = 1.0
    det_cube[det_cube>0] =  S2_gmd.norm_fun(det_cube[det_cube>0], alpha, bounds=(0.5,1.0))

    if not os.path.exists(outputShapeFile):
        print("Clone shape file")
        fusion.cloneAndUpdateShapefile(segmentsFile, outputShapeFile)

    print("Write detection shape file results")
    fusion.writeDetections_S2(outputShapeFile, unique_segments, det_cube, NDVIDateList, np.isfinite(NDVI_seg), "S2",
                               minimum_interval_days=no_mowing_after_det, non_overlap_interval_days=non_overlap_interval_days)

    print("Compliance")
    if do_cmpl:
        print("Run compliancy calculation")
        compliancy.do_compliancy(outputShapeFile, compliance_config_filename=compliancy_conf_file)


    if test:
        ret_test = {"NDVI_seg":NDVI_seg, "NDVI_nomow_model":NDVI_nomow_model, "NDVI_nomow_model_":NDVI_nomow_model_, "NDVI_last_hist":NDVI_last_hist, "NDVIDateList":NDVIDateList, "det_cube":det_cube, "unique_segments":unique_segments, "inv_seg_dct":inv_seg_dct}
    else:
        ret_test = None

    return 0, ret_test


def main_run(configFile, segmentsFile, data_x_detection, data_x_model, outputDir, new_acq_date,
             older_acq_date=None, tile_list=[],
             seg_parcel_id_attribute=None, outputShapeFile=None,
             do_cmpl=False, test=False):

    config = configparser.ConfigParser()
    config.read(configFile)

    re_compile = "(S2AGRI_L3B_([A-Z]{5,11})_A([0-9]{8})T([0-9]{6})_T([0-9]{2}[A-Z]{3})\.)"
    # [S2_input_data]
    # re_compile = config['S2_input_data']['re_compile']
    # data_x_detection = list(map(str.strip, config['S2_input_data']['data_x_detection'].split(',')))
    # data_x_model = list(map(str.strip, config['S2_input_data']['data_x_model'].split(',')))
    print("data_x_detection", data_x_detection)
    print("data_x_model", data_x_model)

    # [constants]
    S2_time_interval = np.int(config['S2_constants']['S2_time_interval'])
    locAcqTime = config['S2_constants']['locAcqTime']
    print("S2_time_interval", S2_time_interval)
    print("locAcqTime", locAcqTime)

    # [S2_model]
    apply_model = config['S2_model']['apply_model']
    if apply_model == "True":
        apply_model = True
    elif apply_model == "False":
        apply_model = False
    else:
        print("apply_model par invalid")
    modelDir = config['S2_model']['modelDir']
    NDVI_nomow_model_perc = np.float(config['S2_model']['NDVI_nomow_model_perc'])  # 95.
    sampling_days = np.float(config['S2_model']['sampling_days'])  # 1
    p_n_th = np.int(config['S2_model']['minimum_parcels_th'])  # 20
    model_temporal_range_str = list(ast.literal_eval(config['S2_model']['model_temporal_range_str']))
#    start_params = [float(s) for s in config['S2_model']['start_params'].split(',')]
    start_params = ast.literal_eval(config['S2_model']['start_params'])
    bounds = list(ast.literal_eval(config['S2_model']['bounds']))
    print("apply_model", apply_model)
    print("modelDir", modelDir)
    print("NDVI_nomow_model_perc",NDVI_nomow_model_perc)
    print("sampling_days",sampling_days)
    print("p_n_th",p_n_th)
    print("model_temporal_range_str",model_temporal_range_str)
    print("start_params",start_params)
    print("bounds",bounds)

    # [processing]
    prod_type_list = list(map(str.strip, config['S2_processing']['prod_type_list'].split(','))) # ['SNDVI']  # NDVI
    sc_fact = [np.float(s) for s in config['S2_processing']['sc_fact'].split(',')] # [1000]
    corrupted_th = [np.float(s) for s in config['S2_processing']['corrupted_th'].split(',')] # [1000] [0.1]
    decreasing_abs_th = [np.float(s) for s in config['S2_processing']['decreasing_abs_th'].split(',')]
    decreasing_rate_th = [np.float(s) for s in config['S2_processing']['decreasing_rate_th'].split(',')]
    increasing_rate_th = [np.float(s) for s in config['S2_processing']['increasing_rate_th'].split(',')]
    low_abs_th = [np.float(s) for s in config['S2_processing']['low_abs_th'].split(',')]
    high_abs_th = [np.float(s) for s in config['S2_processing']['high_abs_th'].split(',')]
    invalid_data = [np.float(s) for s in config['S2_processing']['invalid_data'].split(',')]
    no_mowing_after_det = np.int(config['S2_processing']['no_mowing_after_det'])
    non_overlap_interval_days = np.int(config['S2_processing']['non_overlap_interval_days'])
    options_layer_burning = list(map(str.strip, config['S2_processing']['options_layer_burning'].split(',')))
    erode_pixels = np.int(config['S2_processing']['erode_pixels'])
    stat_smpl_n = np.int(config['S2_processing']['stat_smpl_n'])
    print("prod_type_list",prod_type_list)
    print("sc_fact",sc_fact)
    print("corrupted_th",corrupted_th)
    print("decreasing_abs_th",decreasing_abs_th)
    print("decreasing_rate_th",decreasing_rate_th)
    print("increasing_rate_th",increasing_rate_th)
    print("low_abs_th",low_abs_th)
    print("high_abs_th",high_abs_th)
    print("options_layer_burning",options_layer_burning)
    print("invalid_data",invalid_data)
    print("no_mowing_after_det",no_mowing_after_det)
    print("non_overlap_interval_days", non_overlap_interval_days)
    print("erode_pixels",erode_pixels)
    print("stat_smpl_n",stat_smpl_n)

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

    print(tile_list)
    done_tiles=[]
    state_tiles=[]
    test_d=[]
    for tile_number in tile_list:
        print("Run tile:", tile_number)
        model_dict = None
        if apply_model:
            print("Apply adaptive model")
            model_file = model_lib.model_file_name_gen([tile_number], model_temporal_range_str, prod_type_list)
            model_file_abs = os.path.join(os.path.join(outputDir, modelDir), model_file)
            print("Adaptive model file name:", model_file_abs)

            if os.path.exists(model_file_abs):
                print("Model is already calculated")
                model_dict = model_lib.load_model(model_file_abs)
            else:
                print("Model must be calculated")
                try:
                    os.makedirs(os.path.join(os.path.join(outputDir, modelDir)))
                except (OSError):
                    print("output dir exixts...moving on")

                model_dbl_l, x_mod, y, x, flag = model_lib.make_model_pandas(model_temporal_range_str, data_x_model,
                                                                             segmentsFile,
                                                                             seg_parcel_id_attribute, outputDir,
                                                                             prod_type_list, tile_number,
                                                                             sc_fact, corrupted_th,
                                                                             model_file_abs,
                                                                             options_layer_burning=options_layer_burning,
                                                                             erode_pixels=erode_pixels,
                                                                             invalid_val=invalid_data[0],
                                                                             NDVI_nomow_model_perc=NDVI_nomow_model_perc,
                                                                             sampling_days=sampling_days,
                                                                             p_n_th=p_n_th,
                                                                             start_params=start_params, bounds=bounds)
                if flag == 0:
                    print("x:", x)
                    print("y:", y)
                    model_dict = model_lib.load_model(model_file_abs)
                else:
                    print("No model extracted")
                    model_dict = None

        if (apply_model and model_dict) or not apply_model:
            print("The processing can be run")
            ret, test_data = run_proc(tile_number, data_x_detection, re_compile, segmentsFile,
                                      new_acq_date, older_acq_date=older_acq_date,
                                      outputDir=outputDir, modelDir=modelDir, outputShapeFile=outputShapeFile,
                                      seg_parcel_id_attribute = seg_parcel_id_attribute,
                                      options_layer_burning = options_layer_burning,
                                      erode_pixels = erode_pixels,
                                      prod_type_list = prod_type_list,
                                      sc_fact = sc_fact,
                                      corrupted_th = corrupted_th,
                                      invalid_data = invalid_data[0],
                                      decreasing_abs_th = decreasing_abs_th,
                                      decreasing_rate_th = decreasing_rate_th,
                                      increasing_rate_th = increasing_rate_th,
                                      high_abs_th = high_abs_th,
                                      low_abs_th = low_abs_th,
                                      apply_model = apply_model,
                                      model_dict = model_dict,
                                      no_mowing_after_det = no_mowing_after_det,
                                      non_overlap_interval_days = non_overlap_interval_days,
                                      locAcqTime = locAcqTime,
                                      S2_time_interval = S2_time_interval,
                                      stat_smpl_n = stat_smpl_n,
                                      do_cmpl = do_cmpl,
                                      compliancy_conf_file=configFile,
                                      test=test)
        else:
            ret = 3
            test_data = None

        print("Tiles done: ", tile_number)
        print("Processing state (0: update shp file; 1: no parcels intersecting tile; 2: no data available for that tile; 3: no model; 4: duplicated parcel IDs) -->", ret)
        done_tiles.append(tile_number)
        state_tiles.append(ret)
        test_d.append(test_data)

    print("tile list:  ", tile_list)
    print("tile done:  ", done_tiles)
    print("tile state: ", state_tiles)

    return 0, test_d


############################ NEW IMPLEMENTATION ####################################
def parse_date(str):
    return datetime.datetime.strptime(str, "%Y-%m-%d").date()

class NdviProduct(object):
    def __init__(self, dt, tile_id, path):
        self.tile_id = tile_id
        self.path = path

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
        
        self.input_products_list = None
        if args.input_products_list:
            self.input_products_list = args.input_products_list

        self.prds_are_tif = int(args.prds_are_tif)
           
        if args.season_start:
            self.season_start = parse_date(args.season_start)
            print("Season_start = ", args.season_start)
        
        if args.season_end:        
            self.season_end = parse_date(args.season_end)
            print("Season_end = ", args.season_end)            

def get_s2_products_from_tiffs(config):
    products = []
    l3b_file_regex = "(S2AGRI_L3B_([A-Z]{5,11})_A([0-9]{8})T([0-9]{6})_T([0-9]{2}[A-Z]{3})\.)"
    regex = re.compile(l3b_file_regex)
    for tifFilePath in config.input_products_list:
        tifFileName = os.path.basename(tifFilePath)
        result = regex.match(tifFileName)
        if result and len(result.groups()) == 5:
            dt_str = "{}T{}".format(result.group(3), result.group(4))
            dt = datetime.datetime.strptime(dt_str, '%Y%m%dT%H%M%S')
            products.append(NdviProduct(dt, result.group(5), tifFilePath))
    
    return products
    
def get_s2_products(config, conn):
    with conn.cursor() as cursor:
        if config.input_products_list is None or len(config.input_products_list) == 0 :
            print ("Extracting NDVI infos from the database for the whole season interval!!!")
            query = SQL(
                """
                with products as (
                    select product.site_id,
                        product.name,
                        product.created_timestamp as date,
                        product.processor_id,
                        product.product_type_id,
                        product.full_path,
                        product.tiles
                    from product
                    where product.site_id = {}
                        and product.product_type_id = 3
                )
                select products.date,
                        products.tiles,
                        products.full_path
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
            print ("config.prds_are_tif = {}".format(config.prds_are_tif))
            if config.prds_are_tif == 1 : 
                print ("Extracting NDVI infos from TIF files!!!")            
                return get_s2_products_from_tiffs(config)
            
            print ("Extracting NDVI infos from the database for the list of products!!!")
            # Otherwise, get the products from DB and extract the tiffs
            if len (config.input_products_list) > 1:
                prdsSubstr = tuple(config.input_products_list)
            else :
                prdsSubstr = "('{}')".format(config.input_products_list[0])
        
            query= """
                   with products as (
                    select product.site_id,
                        product.name,
                        product.created_timestamp as date,
                        product.processor_id,
                        product.product_type_id,
                        product.full_path,
                        product.tiles
                    from product
                    where product.site_id = {} and product.product_type_id = 3 and product.full_path in {}
                )
                select products.date,
                        products.tiles,
                        products.full_path
                from products
                order by date;""".format(config.site_id, prdsSubstr)
            #print(query)
        
        # execute the query
        cursor.execute(query)            
            
        results = cursor.fetchall()
        conn.commit()

        products = []
        for (dt, tiles, full_path) in results:
            # print ("Handling product : {} with tiles {} ...".format(full_path, tiles))
            tilesPath = os.path.join(full_path, "TILES")
            try:
                tilesDirs = os.listdir(tilesPath)
            except:
                print("Product {} found in DB but not on disk".format(full_path))
                continue       
                
            for tile in tiles :   
                # print ("Handling tile {} for product : {} ...".format(tile, full_path))
                # print ("TILE DIRS: {} ...".format(tilesDirs))
                
                # Ignore the L8 tiles
                if re.match("\d{6}", tile) :
                    print ("Ignoring L8 tile {}".format(tile))
                    continue
                tilePaths = fnmatch.filter(tilesDirs, "S2AGRI_L3B_A*_T{}".format(tile))
                if len(tilePaths) == 1:
                    subPath = tilePaths[0]
                    fullTilePath = os.path.join(tilesPath, subPath)
                    tileImgDataPath = os.path.join(fullTilePath, "IMG_DATA")
                    try:
                        tileDirFiles = os.listdir(tileImgDataPath)
                    except:
                        print("Expected L3B product structure found but the path {} does not exists".format(tileImgDataPath))
                        continue
                    prdFiles = fnmatch.filter(tileDirFiles, "S2AGRI_L3B_S*_A*_T{}.TIF".format(tile))
                    for prdFile in prdFiles:
                        # print ("Using product tif: {} ...".format(os.path.join(tileImgDataPath, prdFile)))
                        products.append(NdviProduct(dt, tile, os.path.join(tileImgDataPath, prdFile)))
                
                
        return products

def main() :
    parser = argparse.ArgumentParser(description="Executes grassland mowing S2 detection")
    parser.add_argument('-c', '--s4c-config-file', default='/etc/sen2agri/sen2agri.conf', help="Sen4CAP system configuration file location")
    parser.add_argument('-s', '--site-id', help="The site id")
    parser.add_argument('-f', '--config-file', default = "/usr/share/sen2agri/S4C_L4B_GrasslandMowing/config.ini", help="Grassland mowing parameters configuration file location")
    parser.add_argument('-i', '--input-shape-file', help="The input shapefile GSAA")
    parser.add_argument('-o', '--output-data-dir', help="Output data directory")
    parser.add_argument('-e', '--new-acq-date', help="The new acquisition date")
    parser.add_argument('-b', '--older-acq-date', help="The older acquisition date")
    # parser.add_argument('-l', '--orbit-list-file', help="The orbit list file")    # NOT USED ANYMORE - ORBITS DETECTED FROM THE DATABASE
    parser.add_argument('-l', '--input-products-list', nargs='+', help="The list of L3B products")   
    parser.add_argument('--prds-are-tif', help="Specify that the products in the input-product-list are actually the TIF files", type=int, default="0")
    parser.add_argument('-a', '--seg-parcel-id-attribute', help="The SEQ unique ID attribute name", default="NewID")
    parser.add_argument('-x', '--output-shapefile', help="Output shapefile")
    parser.add_argument('-m', '--do-cmpl', help="Run compliancy")
    parser.add_argument('--season-start', help="Season start")
    parser.add_argument('--season-end', help="Season end")
    parser.add_argument('-t', '--test', help="Run test")
    
    args = parser.parse_args()
    
    s4cConfig = S4CConfig(args)
    
    with psycopg2.connect(host=s4cConfig.host, dbname=s4cConfig.dbname, user=s4cConfig.user, password=s4cConfig.password) as conn:
        products = get_s2_products(s4cConfig, conn)
        
        tile_list = []
        data_x_detection = []
        for product in products:
            data_x_detection.append(product.path) 
            if product.tile_id not in tile_list:
                tile_list.append(product.tile_id)

        print("tile_list:", tile_list)
        
        ret, _ = main_run(s4cConfig.config_file, s4cConfig.input_shape_file, data_x_detection, data_x_detection, s4cConfig.output_data_dir, s4cConfig.new_acq_date,
                       older_acq_date = s4cConfig.older_acq_date, tile_list=tile_list,
                       seg_parcel_id_attribute = s4cConfig.seg_parcel_id_attribute, outputShapeFile=s4cConfig.output_shapefile,
                       do_cmpl=s4cConfig.do_cmpl, test=s4cConfig.test)
        
if __name__== "__main__":

    os.environ['SHAPE_ENCODING'] = "utf-8"
    
    main()
    
#    if len(sys.argv)==11:
#        config_file = sys.argv[1]
#        input_shape_file = sys.argv[2]
#        output_data_dir =  sys.argv[3]
#        new_acq_date =   sys.argv[4]
#        older_acq_date =  sys.argv[5]
#        tile_list_file = sys.argv[6]
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
#        print("6) tile_list_file")
#        print("7) seg_parcel_id_attribute")
#        print("8) outputShapeFile")
#        print("9) do_cmpl")
#        print("10) test")
#        raise Exception("Just 10 arguments are expected.", len(sys.argv)-1, "arguments are passed.")
#
#    config = configparser.ConfigParser()
#    config.read(tile_list_file)
#    tile_list = list(map(str.strip, config['tiles']['tile_list'].split(',')))
#    print("tile_list:", tile_list)
#
#    ret, _ = main_run(config_file, input_shape_file, output_data_dir, new_acq_date,
#                   older_acq_date = older_acq_date, tile_list=tile_list,
#                   seg_parcel_id_attribute = seg_parcel_id_attribute, outputShapeFile=outputShapeFile,
#                   do_cmpl=do_cmpl, test=test)

