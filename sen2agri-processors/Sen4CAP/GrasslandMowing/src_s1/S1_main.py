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

def run_proc(orbit_list, orbit_type, sarDataGlob, re_compile, segmentsFile,
             new_acq_date, older_acq_date=None,
             truthsFile=None, outputDir=None, outputShapeFile=None,
             seg_parcel_id_attribute=None, tr_parcel_id_attribute=None,
             options_layer_burning=['ALL_TOUCHED=False'],
             invalid_data = 0,
             saturate_sigma=False,
             pfa=3e-5,
             stat_smpl_n=5,
             min_cohe_var=0.024,
             erode_pixels=0,
             no_mowing_after_det=30,
             locAcqTimeASC='18:00:00',
             locAcqTimeDESC='06:00:00',
             dataType=['COHE'],
             polType=['VH'],
             S1_time_interval=6,
             do_cmpl=False,
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
        return 2

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
        return 1

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
        return 1

    # make vrt over the intersection extent
    print('Make virtual raster of input data (considering intersection between extensions of i- the shape file, ii- the virtual raster)')
    output_vrt = os.path.join(output_tmp_dir, "data_cube.vrt")
    images_n = S1_gmd.make_vrt(data_list, dst_srs, output_tmp_dir, output_vrt, outputBounds=outputBounds,
                                  srcNodata=invalid_data, resampling=resampling, error_threshold=error_threshold)
    if images_n > 0:
        vrt_data = gdal.Open(output_vrt)
    else:
        print('Empty VRT. Does orbit intersect segment layer?')
        return 1

    # Generate segmentation map from input shapefiles
    # -----------------------------------------------

    print('Generate segmentation map from input shapefiles')

    segmentsOutputDir=os.path.join(outputDir, "segments")
    print("segmentsOutputDir", segmentsOutputDir)
    if truthsFile:
        truthsOutputDir=os.path.join(outputDir, "truths")
        print("truthsOutputDir", truthsOutputDir)
    else:
        truthsOutputDir = None

    try:
        os.makedirs(segmentsOutputDir)
    except(OSError):
        print('directory ',segmentsOutputDir,' exists...moving on')

    if truthsFile:
        try:
            os.makedirs(truthsOutputDir)
        except(OSError):
            print('directory ',truthsOutputDir,' exists...moving on')

    segmentsOutRaster = os.path.join(segmentsOutputDir, os.path.basename(segmentsFile)[:-4]+'_raster.tif')
    print("segmentsOutRaster", segmentsOutRaster)

    if truthsFile:
        truthsOutRaster = os.path.join(truthsOutputDir, os.path.basename(truthsFile)[:-4]+'_raster.tif')
        print("truthsOutRaster", truthsOutRaster)

    # Generate segmentation raster map from input shapefile
    print("layer2mask for segments: ", segmentsFile, output_vrt, segmentsOutRaster)
    burned_pixels, seg_attributes = S1_gmd.layer2mask(segmentsFile, output_vrt, segmentsOutRaster, layer_type='segments', options=options_layer_burning)
    print('segments burned_pixels',burned_pixels)

    if truthsFile:
        # Generate truths raster map from input shapefile
        print("layer2mask for truths: ", truthsFile, output_vrt, truthsOutRaster)
        burned_pixels, tr_attributes = S1_gmd.layer2mask(truthsFile, output_vrt, truthsOutRaster, layer_type='segments', options=options_layer_burning)
        print('truths burned_pixels',burned_pixels)

    # Segment Analysis
    # ----------------

    # load segments and truths
    gdal_data =  gdal.Open(segmentsOutRaster)
    segments = gdal_data.ReadAsArray()
    segments_geo_transform = gdal_data.GetGeoTransform()
    segments_projection = gdal_data.GetProjection()

    if truthsFile:
        gdal_data =  gdal.Open(truthsOutRaster)
        truths = gdal_data.ReadAsArray()
        truths_geo_transform = gdal_data.GetGeoTransform()
        truths_projection = gdal_data.GetProjection()

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

    ## truths
    if truthsFile:
        # erode segments

        eroded_truths = np.copy(truths)
        if erode_pixels > 0:
            # first step: separate segments
            cross = np.array([[0,1,0],[1,1,1],[0,1,0]])
            min_truths = scipy.ndimage.minimum_filter(truths,footprint=cross)
            max_truths = scipy.ndimage.maximum_filter(truths,footprint=cross)
            truths_borders = (max_truths-min_truths)>0
            eroded_truths[truths_borders] = 0

            # second setp: erode
            if erode_pixels > 1:
                erosion_mask = scipy.ndimage.morphology.binary_erosion(eroded_truths>0, structure=cross, iterations=(erode_pixels-1))
                eroded_truths = eroded_truths*erosion_mask

    # Fill segments
    # -------------

    print('Fill segments')

    # segments
    unique_segments = np.unique(eroded_segs)
    if unique_segments[0] == 0: # 0 label corresponds to not valid segments and it will be removed
        unique_segments = unique_segments[1:]
    print(unique_segments.shape)
    seg_pixels_num = np.array(ndimage.sum(eroded_segs>0, eroded_segs, index=unique_segments))

    if truthsFile:
        #truths
        unique_truths = np.unique(eroded_truths)
        if unique_truths[0] == 0: # 0 label corresponds to not valid segments and it will be removed
            unique_truths = unique_truths[1:]
        print(unique_truths.shape)

    
    # extract data parameters and dates from file_list in the vrt data
    print('Extract data parameters and dates from file_list in the vrt data')
    par_list = S1_gmd.read_file_list(vrt_data.GetFileList()[1:], get_par_from_file, keys, orbit_list, polType, dataType)

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
    parcel_fid = (data_df.reset_index()[['parcel_id', 'fid']]
                         .set_index('parcel_id')
                         .reset_index()
                         .drop_duplicates()
                         .set_index('parcel_id')
                 )
    data_df = pd.pivot_table(data_df, values=['mean','count'], index=[seg_parcel_id_attribute],
                             columns=['orbit', 'pol', 'data_type', 'acq_date'], dropna=False)
    data_df['fid'] = parcel_fid['fid']

    # Move fid to columns and sort by it to match unique_segments
    data_df = data_df.reset_index().set_index(seg_parcel_id_attribute)
    data_df.sort_values('fid', inplace=True)

    print("STAMPA DATA_DF")
    print(data_df)

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

    # make dictionary segment --> seg_parcel_id_attribute
    seg_attributes = {i: {seg_parcel_id_attribute: p} for i, p in enumerate(parcel)}

    # inverti dizionari
    inv_seg_dct = {v[seg_parcel_id_attribute]:k for k,v in seg_attributes.items()}
    if truthsFile:
        inv_tr_dct = {v[tr_parcel_id_attribute]:k for k,v in tr_attributes.items()}

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
                              mission_id='S1', max_dates=4, minimum_interval_days=no_mowing_after_det)

    if do_cmpl:
        print("Run compliancy calculation")
        compliancy.do_compliancy(outputShapeFile, compliance_config_filename=compliancy_conf_file)


    return 0


def main_run(configFile, segmentsFile, outputDir, new_acq_date, older_acq_date=None,
             truthsFile=None, orbit_list=[], orbit_type_list=[],
             seg_parcel_id_attribute=None, tr_parcel_id_attribute=None, outputShapeFile=None, do_cmpl=False):

    config = configparser.ConfigParser()
    config.read(configFile)

    # [S1_input_data]
    re_compile = config['S1_input_data']['re_compile']
    data_x_detection = list(map(str.strip, config['S1_input_data']['data_x_detection'].split(',')))
    print("re_compile", re_compile)
    print("data_x_detection", data_x_detection)

    # [S1_constants]
    S1_time_interval = np.int(config['S1_constants']['S1_time_interval'])
    SAR_spacing = np.float(config['S1_constants']['SAR_spacing'])
    cohe_ENL = np.float(config['S1_constants']['cohe_ENL'])
    min_cohe_var = np.float(config['S1_constants']['min_cohe_var'])
    locAcqTimeASC = config['S1_constants']['locAcqTimeASC']
    locAcqTimeDESC = config['S1_constants']['locAcqTimeASC']

    # [S1_processing]
    invalid_data = np.float(config['S1_processing']['invalid_data'])
    saturate_sigma = np.bool(config['S1_processing']['saturate_sigma'])
    pfa = np.float(config['S1_processing']['pfa'])
    stat_smpl_n = np.int(config['S1_processing']['stat_smpl_n'])
    no_mowing_after_det = np.int(config['S1_processing']['no_mowing_after_det'])
    options_layer_burning = list(map(str.strip, config['S1_processing']['options_layer_burning'].split(',')))
    erode_pixels = np.int(config['S1_processing']['erode_pixels'])
    dataType = list(map(str.strip, config['S1_processing']['data_types'].split(',')))
    polType = list(map(str.strip, config['S1_processing']['pol_types'].split(',')))
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
    for orbit_number, orbit_type in zip(orbit_list, orbit_type_list):
        print("Run: orbit %s, type %s"%(orbit_number,orbit_type))
        ret = run_proc([orbit_number], [orbit_type], data_x_detection, re_compile, segmentsFile,
                       new_acq_date, older_acq_date=older_acq_date,
                       truthsFile=truthsFile, outputDir=outputDir, outputShapeFile=outputShapeFile,
                       seg_parcel_id_attribute = seg_parcel_id_attribute,
                       tr_parcel_id_attribute = tr_parcel_id_attribute,
                       options_layer_burning = options_layer_burning,
                       invalid_data = invalid_data,
                       saturate_sigma = saturate_sigma,
                       pfa = pfa,
                       stat_smpl_n = stat_smpl_n,
                       min_cohe_var = min_cohe_var,
                       erode_pixels = erode_pixels,
                       no_mowing_after_det = no_mowing_after_det,
                       locAcqTimeASC = locAcqTimeASC,
                       locAcqTimeDESC = locAcqTimeDESC,
                       dataType = dataType,
                       polType = polType,
                       S1_time_interval = S1_time_interval,
                       do_cmpl = do_cmpl,
                       compliancy_conf_file=configFile)

        print("Orbit done: ", orbit_number)
        print("Processing state (0: update shp file; 1: no parcels intersecting orbit; 2: no data available for that orbit) -->", ret)
        done_orbits.append(orbit_number)
        state_orbits.append(ret)

    print("orbit list:  ", orbit_list)
    print("orbit done:  ", done_orbits)
    print("orbit state: ", state_orbits)

    return 0

if __name__== "__main__":
    if len(sys.argv)==10:
        config_file = sys.argv[1]
        input_shape_file = sys.argv[2]
        output_data_dir =  sys.argv[3]
        new_acq_date =   sys.argv[4]
        older_acq_date =  sys.argv[5]
        orbit_list_file = sys.argv[6]
        seg_parcel_id_attribute = sys.argv[7]
        outputShapeFile = sys.argv[8]
        do_cmpl = np.bool(sys.argv[9])
    else:
        print("Expected input:")
        print("1) config_file")
        print("2) input_shape_file")
        print("3) output_data_dir")
        print("4) new_acq_date")
        print("5) older_acq_date")
        print("6) orbit_list_file")
        print("7) seg_parcel_id_attribute")
        print("8) outputShapeFile")
        print("9) do_cmpl")
        raise Exception("Just 11 arguments are expected.", len(sys.argv)-1, "arguments are passed.")

    config = configparser.ConfigParser()
    config.read(orbit_list_file)
    
    orbit_list = list(map(str.strip, config['orbits']['orbit_list'].split(',')))
    orbit_type_list = list(map(str.strip, config['orbits']['orbit_type_list'].split(',')))
    print("orbit_list:", orbit_list)
    print("orbit_type_list", orbit_type_list)

    ret = main_run(config_file, input_shape_file, output_data_dir, new_acq_date, 
                   older_acq_date = older_acq_date, orbit_list=orbit_list, orbit_type_list=orbit_type_list,
                   seg_parcel_id_attribute = seg_parcel_id_attribute, outputShapeFile=outputShapeFile,
                   do_cmpl=do_cmpl)


