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
from osgeo import ogr
gdal.UseExceptions()
ogr.UseExceptions()


def run_proc(tile_number, S2DataGlob, re_compile, segmentsFile,
             new_acq_date, older_acq_date=None,
             truthsFile=None, outputDir=None, outputShapeFile=None,
             seg_parcel_id_attribute=None, tr_parcel_id_attribute=None,
             options_layer_burning=['ALL_TOUCHED=False'],
             erode_pixels = 0,
             prod_type_list = ['NDVI'],
             sc_fact = None,
             corrupted_th = None,
             invalid_data = None,
             decreasing_abs_th = None,
             model_dict = None,
             no_mowing_after_det=30,
             non_overlap_interval_days=45,
             locAcqTime='10:30:00',
             S2_time_interval=5,
             do_cmpl=False,
             stat_smpl_n=0,
             compliancy_conf_file=None):

    print('File list generation from data dir')


    # setting file name parsing tools
    keys = ['file_name', 'data_type', 'acq_date', 'acq_time', 'tile_code']
    get_par_from_file = re.compile(re_compile)

    # file list generation from data dir
    file_list = glob.glob(S2DataGlob)
    print("Searched files:", S2DataGlob)

    # remove NOTV directories
    file_list = [f for f in file_list if not '_NOTV/' in f]

    # extract data file names and dates from file list and for specific orbits 
    par_list = S2_gmd.read_file_list(file_list, get_par_from_file, keys, tile_number, orbit_field_label='tile_code')
    print("size par_list", len(par_list))

    # verify if there are data for specific tile
    if len(par_list) == 0:
        print("There are NO data for the specific tile", tile_number)
        return 2

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
        print("Intesection extent between shapefile and VRT is empty")
        return 1

    # make vrt over the intersection extent
    print('Make virtual raster of input data (considering intersection between extensions of i- the shape file, ii- the virtual raster)')
    output_vrt = os.path.join(output_tmp_dir, "data_cube.vrt")
    images_n = S2_gmd.make_vrt(data_list, dst_srs, output_tmp_dir, output_vrt, outputBounds=outputBounds,
                                  srcNodata=invalid_data, resampling=resampling, error_threshold=error_threshold)
    if images_n > 0:
        vrt_data = gdal.Open(output_vrt)
    else:
        print('Empty VRT. Does tile intersect segment layer?')
        return 1

    print("Images in VRT:", images_n)


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
    burned_pixels, seg_attributes = S2_gmd.layer2mask(segmentsFile, output_vrt, segmentsOutRaster, layer_type='segments', options=options_layer_burning)
    print('segments burned_pixels',burned_pixels)

    if truthsFile:
        # Generate truths raster map from input shapefile
        print("layer2mask for truths: ", truthsFile, output_vrt, truthsOutRaster)
        burned_pixels, tr_attributes = S2_gmd.layer2mask(truthsFile, output_vrt, truthsOutRaster, layer_type='segments', options=options_layer_burning)
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

    print("eroded_segs", eroded_segs)
    print("min, max:", np.min(eroded_segs), np.max(eroded_segs))
    
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
    print("unique_segments.shape =", unique_segments.shape)
    seg_pixels_num = np.array(scipy.ndimage.sum(eroded_segs>0, eroded_segs, index=unique_segments))

    if truthsFile:
        #truths
        unique_truths = np.unique(eroded_truths)
        if unique_truths[0] == 0: # 0 label corresponds to not valid segments and it will be removed
            unique_truths = unique_truths[1:]
        print("unique_truths.shape =", unique_truths.shape)

    # extract data parameters and dates from file_list in the vrt data
    print('Extract data parameters and dates from file_list in the vrt data')
    par_list = S2_gmd.read_file_list(vrt_data.GetFileList()[1:], get_par_from_file, keys, tile_number, orbit_field_label='tile_code')

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
                             columns=['data_type', 'acq_date'], dropna=False)
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

    parcel = data_df.index.values

    # generate array with temporal series                                                                                                                                                                        
    VI_seg = []
    for pt, sf, ct in zip(prod_type_list, sc_fact, corrupted_th):
        VI_data = data_df.xs(key=('mean', pt), level=(0, 'data_type'), axis=1).values
        dates_str = data_df.xs(key=('mean', pt), level=(0, 'data_type'), axis=1).columns.values

        VIDateList = [dateutil.parser.parse(d+" "+locAcqTime, yearfirst=True, dayfirst=False) for d in dates_str]
        VI_seg.append({'data_type': pt, 'date_list': VIDateList, 'VI': VI_data, 'sc_fact': sf, 'corrupted_th': ct})

    # apply scale factors and remove corrupted VI
    for d in VI_seg:
        d['VI'] /= d['sc_fact']
        d['VI'][ d['VI'] < d['corrupted_th'] ] = np.nan


    # Detection
    # -----------------

    print("Detection")

    NDVI_seg = np.array(VI_seg[0]['VI'])
    NDVIDateList = VI_seg[0]['date_list']

    # Fit of the model to the available DOYs
    x = model_dict[prod_type_list[0]][:,0]
    model_dbl_l = model_dict[prod_type_list[0]][:,1]

    # final no-mowing model
    NDVI_nomow_model_ = np.interp([d.timetuple().tm_yday for d in NDVIDateList], x, model_dbl_l)


    # diff
    diff_seg = np.zeros_like(NDVI_seg)*np.nan
    for i in range(len(NDVI_seg)):
        ind_mas = np.where(np.isfinite(NDVI_seg[i,:]))
        if np.size(ind_mas) > 0:
            val = (NDVI_seg[i, ind_mas])[0]
            diff = (val - np.roll(val, 1))
            diff[0] = -10000
            diff_seg[i,ind_mas] = diff
    


    # inverti dizionari
    print("inverti dizionari")
    inv_seg_dct = {v[seg_parcel_id_attribute]:k for k,v in seg_attributes.items()}
    if truthsFile:
        inv_tr_dct = {v[tr_parcel_id_attribute]:k for k,v in tr_attributes.items()}


    # Adapt/initialize no-mowing model to each parcel
    print("Adapt/initialize no-mowing model to each parcel")
    NDVI_nomow_model = np.zeros_like(NDVI_seg)
    for i in range(len(NDVI_seg)):
        indx = np.where(np.isfinite(NDVI_seg[i,:]))[0]
        if np.size(indx) > 0:
            indx = indx[0]
            NDVI_nomow_model[i,:] = NDVI_nomow_model_*(NDVI_seg[i,indx]/NDVI_nomow_model_[indx])


    # Detection on model

    NDVI_det_cube = np.zeros_like(NDVI_seg)

    for t in np.arange(1, NDVI_det_cube.shape[1]):
        # detection at t
        NDVI_det_cube[:,t] = np.maximum((NDVI_nomow_model[:,t] - NDVI_seg[:,t] - decreasing_abs_th)/NDVI_nomow_model[:,t-1], 0)  # C1: decremento assoluto wrt model    

    # update model only for the parcel with detected mowing
        to_update_parc = list(np.where(NDVI_det_cube[:,t] > 0)[0])
        increased_NDVI = list(np.where(diff_seg[:,t] > 0)[0])
        to_update_parc = np.array(to_update_parc + increased_NDVI)
        if len(to_update_parc) > 0:
            NDVI_nomow_model[to_update_parc, t:] *= (NDVI_seg[to_update_parc, t]/NDVI_nomow_model[to_update_parc, t])[:,None]
    
    det_cube = NDVI_det_cube
    det_cube[np.isnan(det_cube)] = 0

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

    return 0


def main_run(configFile, segmentsFile, outputDir, new_acq_date, 
             older_acq_date=None, truthsFile=None, tile_list=[],
             seg_parcel_id_attribute=None, tr_parcel_id_attribute=None, outputShapeFile=None,
             do_cmpl=False):

    config = configparser.ConfigParser()
    config.read(configFile)

    # [S2_input_data]
    re_compile = config['S2_input_data']['re_compile']
    data_x_detection = config['S2_input_data']['data_x_detection']
    data_x_model = config['S2_input_data']['data_x_model']
    print("data_x_detection", data_x_detection)
    print("data_x_model", data_x_model)

    # [constants]
    S2_time_interval = np.int(config['S2_constants']['S2_time_interval'])
    locAcqTime = config['S2_constants']['locAcqTime']
    print("S2_time_interval", S2_time_interval)
    print("locAcqTime", locAcqTime)

    # [S2_model]
    modelDir = config['S2_model']['modelDir']
    NDVI_nomow_model_perc = np.float(config['S2_model']['NDVI_nomow_model_perc'])  # 95.
    sampling_days = np.float(config['S2_model']['sampling_days'])  # 1
    p_n_th = np.int(config['S2_model']['minimum_parcels_th'])  # 20
    model_temporal_range_str = list(ast.literal_eval(config['S2_model']['model_temporal_range_str']))
#    start_params = [float(s) for s in config['S2_model']['start_params'].split(',')]
    start_params = ast.literal_eval(config['S2_model']['start_params'])
    bounds = list(ast.literal_eval(config['S2_model']['bounds']))
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
    for tile_number in tile_list:
        print("Run tile:", tile_number)
        model_file = model_lib.model_file_name_gen([tile_number], model_temporal_range_str, prod_type_list)
        model_file_abs = os.path.join(os.path.join(outputDir, modelDir), model_file)
        print(model_file_abs)

        if os.path.exists(model_file_abs):
            print("Model is already calculated")
            model_dict = model_lib.load_model(model_file_abs)
        else:
            print("Model must be calculated")
            try:
                os.makedirs(os.path.join(os.path.join(outputDir, modelDir)))
            except (OSError):
                print("output dir exixts...moving on")
   
            model_dbl_l, x_mod, y, x, flag = model_lib.make_model_pandas(model_temporal_range_str, data_x_model, segmentsFile,
                                                                         seg_parcel_id_attribute, outputDir,
                                                                         prod_type_list, tile_number, sc_fact, corrupted_th,
                                                                         model_file_abs,
                                                                         options_layer_burning=options_layer_burning,
                                                                         erode_pixels=erode_pixels, 
                                                                         invalid_val=invalid_data[0],
                                                                         NDVI_nomow_model_perc=NDVI_nomow_model_perc, sampling_days=sampling_days,
                                                                         p_n_th=p_n_th,
                                                                         start_params=start_params, bounds=bounds)
            if flag == 0:
                print("x:", x)
                print("y:", y)
                model_dict = model_lib.load_model(model_file_abs)
            else:
                print("No model extracted")
                model_dict = None

        if model_dict: 
            ret = run_proc(tile_number, data_x_detection, re_compile, segmentsFile, 
                           new_acq_date, older_acq_date=older_acq_date,
                           truthsFile=truthsFile, outputDir=outputDir, outputShapeFile=outputShapeFile,
                           seg_parcel_id_attribute = seg_parcel_id_attribute,
                           tr_parcel_id_attribute = tr_parcel_id_attribute,
                           options_layer_burning = options_layer_burning,
                           erode_pixels = erode_pixels,
                           prod_type_list = prod_type_list,
                           sc_fact = sc_fact,
                           corrupted_th = corrupted_th,
                           invalid_data = invalid_data[0],
                           decreasing_abs_th = decreasing_abs_th,
                           model_dict = model_dict,
                           no_mowing_after_det = no_mowing_after_det,
                           non_overlap_interval_days = non_overlap_interval_days,
                           locAcqTime = locAcqTime,
                           S2_time_interval = S2_time_interval,
                           stat_smpl_n = stat_smpl_n,
                           do_cmpl = do_cmpl,
                           compliancy_conf_file=configFile)
        else:
            ret = 3

        print("Tiles done: ", tile_number)
        print("Processing state (0: update shp file; 1: no parcels intersecting tile; 2: no data available for that tile; 3: no model) -->", ret)
        done_tiles.append(tile_number)
        state_tiles.append(ret)

    print("tile list:  ", tile_list)
    print("tile done:  ", done_tiles)
    print("tile state: ", state_tiles)

    return 0

if __name__== "__main__":
    if len(sys.argv)==10:
        config_file = sys.argv[1]
        input_shape_file = sys.argv[2]
        output_data_dir =  sys.argv[3]
        new_acq_date =   sys.argv[4]
        older_acq_date =  sys.argv[5]
        tile_list_file = sys.argv[6]
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
        print("6) tile_list_file")
        print("7) seg_parcel_id_attribute")
        print("8) outputShapeFile")
        print("9) do_cmpl")
        raise Exception("Just 9 arguments are expected.", len(sys.argv)-1, "arguments are passed.")

    config = configparser.ConfigParser()
    config.read(tile_list_file)
    tile_list = list(map(str.strip, config['tiles']['tile_list'].split(',')))
    print("tile_list:", tile_list)

    ret = main_run(config_file, input_shape_file, output_data_dir, new_acq_date,
                   older_acq_date = older_acq_date, tile_list=tile_list,
                   seg_parcel_id_attribute = seg_parcel_id_attribute, outputShapeFile=outputShapeFile,
                   do_cmpl=do_cmpl)


