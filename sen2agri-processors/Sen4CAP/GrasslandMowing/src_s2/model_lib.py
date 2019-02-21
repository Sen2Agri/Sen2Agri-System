# Inizialization and Import
import os, glob
import re
import pickle

import numpy as np
import dateutil.parser
import pandas as pd
import scipy

import S2_gmd
import datetime
import time
from datetime import date
import pheno_func

import gdal
from osgeo import ogr
gdal.UseExceptions()
ogr.UseExceptions()


def save_model(data_dict, file_name):
    # save the dictionary by using pickle
    with open(file_name, 'wb') as output:
        pickle.dump(data_dict, output)

def load_model(file_name):
    # load the dictionary 
    with open(file_name, 'rb') as fp:
        data_dict = pickle.load(fp)

    return data_dict

def model_file_name_gen(tile_list, model_temporal_range_str, prod_type_list):
    
    model_data_file_name = "model_"+'_'.join(tile_list)+"_"+ model_temporal_range_str[0][:8] + \
                           "_" + model_temporal_range_str[1][:8] + "_" + '_'.join(prod_type_list) + \
                           ".npy"
    return model_data_file_name

def calculate_model_doy(NDVI_seg, doy, model_temporal_range_str, start_params, bounds, min_val_VI=0., NDVI_nomow_model_perc=95., sampling_days=1, p_n_th=100):
    from scipy.signal import argrelextrema
    model_year = (dateutil.parser.parse(model_temporal_range_str[0], yearfirst=True, dayfirst=False)).year
    first_date_of_model_year = datetime.datetime(year=model_year, month=1, day=1, hour=0, minute=0, second=0)

    # definition of the models                                                                                                                                                                                    
    NDVI_nomow_model = np.nanpercentile(NDVI_seg, NDVI_nomow_model_perc, axis=0).T
    NDVI_mean = np.nanmean(NDVI_seg, axis=0).T
    NDVI_std = np.nanstd(NDVI_seg, axis=0).T
    parcel_n = np.sum(NDVI_seg>0, axis=0).T # number of parcels                                                                                                                                                   
    val_ind = np.where(np.logical_and(NDVI_nomow_model > min_val_VI, (parcel_n > p_n_th)))[0]

    if len(val_ind) < 8:
        print("No enough valid dates to calculate model")
        return None, None, None, None, None, None, None, None, 1

    # valid indexes  
    print("Number of parcels for date:", parcel_n)
    print("Valid indexes for model calculation:", val_ind)

    # raw model                                                                                                                                                                                                  
    x = doy[val_ind]
    y = NDVI_nomow_model[val_ind]
    n = parcel_n[val_ind]

    #exclude first points if they not coincide to relative minima                                                                                                                                                 
    relat_min = argrelextrema(y, np.less)[0]
    first_min = relat_min[0]
    #last_min =  relat_min[-1]   # funziona peggio su CZ tile 2 !!                                                                                                                                                
    last_min = np.size(y)-1
    x = x[first_min: min(last_min+1, np.size(x)-1)]
    y = y[first_min: min(last_min+1, np.size(y)-1)]

    # double_logistic fit: parameters                                                                                                                                                                             
    # The starting point to initialize the constrained minimization and the constrains are set                                                                                                                    
    # start_params = [double_logistic pedestal (min), amplitude, rise speed, rise inflection time, fall speed, fall inflection time]                                                                              
    # bound = ([lower bound list],[upper bound list])                                                                                                                                                            

    start_params = [y.min(), y.max()-y.min()] + list(start_params)
    bounds = ([0., 0.] + list(bounds[0]), [np.inf, np.inf] + list(bounds[1]))

    print('start_params', start_params)
    print('bounds', bounds)

    # model fitting                                                                                                                                                                                               
    res = pheno_func.constrained_fit_phenology_model(
                x, y, pheno_model = "dbl_logistic", params=start_params, bounds=bounds)

    if not res.success:
        print("Model fitting failed")
        return None, None, None, None, None, None, None, None, 1
    else:
        dbl_log_params_fit = res.x
        res_cost = res.cost
        res_opt = res.optimality
        message = res.message

        # double_logistic interpolation on dense regular grid
        x_mod = np.linspace(1., 366, num=int(366/sampling_days))
        model_dbl_l = pheno_func.get_model(x_mod, None, params=dbl_log_params_fit, pheno_model = "dbl_logistic")

        return model_dbl_l, x_mod, dbl_log_params_fit, res_cost, res_opt, message, x, y, 0


def make_model_pandas(model_temporal_range_str, S2DataGlob, segmentsFile,
                      seg_parcel_id_attribute, outputDir,
                      prod_type_list, tile_list, sc_fact, corrupted_th,
                      model_data_file_name,
                      options_layer_burning=['ALL_TOUCHED=False'],
                      erode_pixels=0, invalid_val=-10000,
                      NDVI_nomow_model_perc=95., sampling_days=1, p_n_th=100,
                      start_params=[0.05,   50,    0.05,     200.],
                      bounds=      [np.inf, 180.,  np.inf,   360.]):

    # setting file name parsing tools
    keys = ['file_name', 'data_type', 'acq_date', 'acq_time', 'tile_code']
    get_par_from_file = re.compile('(S2AGRI_L3B_([A-Z]{5,11})_A([0-9]{8})T([0-9]{6})_(T[0-9]{2}[A-Z]{3})\.)')

    # file list generation from data dir
    file_list = glob.glob(S2DataGlob)
    print("Searched files:", S2DataGlob)

    # remove NOTV directories
    file_list = [f for f in file_list if not '_NOTV/' in f]

    # extract data file names and dates from file list and for specific orbits 
    par_list = S2_gmd.read_file_list(file_list, get_par_from_file, keys, tile_list, orbit_field_label='tile_code')

    # verify if there are data for specific tile
    if len(par_list) == 0:
        print("There are NO data for the specific tile", tile_list)
        return None, None, None, None, 1

    # put filename and data parameters in a pandas df
    print("fill pandas structure")
    df = pd.DataFrame.from_records(par_list, columns=keys).drop_duplicates(keys[0])

    # date and time conversion from str
    df['acq_date_time' ] = pd.to_datetime(df.apply(lambda x: x['acq_date']+'T'+x['acq_time'], axis=1), yearfirst=True, dayfirst=False)

    # remove all indexes not required
    valid_prod_type_mask = False
    for pt in prod_type_list:
        valid_prod_type_mask = np.logical_or(valid_prod_type_mask, df['data_type'] == pt)
    df = df.loc[valid_prod_type_mask]

    # date selection in the model time range
    validity_temporal_range_str = [dateutil.parser.parse(date_str, yearfirst=True, dayfirst=False) for date_str in model_temporal_range_str]
    select_date_interval = [validity_temporal_range_str[0], model_temporal_range_str[1]]

    valid_date_mask = (df['acq_date_time'] >= select_date_interval[0]) & (df['acq_date_time'] <= select_date_interval[1])
    df = df.loc[valid_date_mask]

    # generate lists of files and dates
    data_list = df['file_name'].values

    # remove corrupted files
    data_list = S2_gmd.remove_corrupted_files(list(data_list))

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
    print('Make virtual raster of input data (without considering extension of the shape file)')
    output_vrt_tmp = os.path.join(output_tmp_dir, "data_cube_tmp.vrt")
    images_n = S2_gmd.make_vrt(data_list, dst_srs, output_tmp_dir, output_vrt_tmp, outputBounds=None,
                                  srcNodata=invalid_val, resampling=resampling, error_threshold=error_threshold)

    if images_n > 0:
        vrt_data = gdal.Open(output_vrt_tmp)
    else:
        print('Empty VRT. Does orbit intersect segment layer?')
        return None, None, None, None, 1

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
        return None, None, None, None, 1

    # make vrt over the intersection extent
    print('Make virtual raster of input data (considering intersection between extensions of i- the shape file, ii- the virtual raster)')
    output_vrt = os.path.join(output_tmp_dir, "data_cube.vrt")
    images_n = S2_gmd.make_vrt(data_list, dst_srs, output_tmp_dir, output_vrt, outputBounds=outputBounds,
                                  srcNodata=invalid_val, resampling=resampling, error_threshold=error_threshold)
    if images_n > 0:
        vrt_data = gdal.Open(output_vrt)
    else:
        print('Empty VRT. Does orbit intersect segment layer?')
        return None, None, None, None, 1

    print("Images to be processed:", images_n)


    # Generate ROI mask and segmentation map from input shapefiles
    # ------------------------------------------------------------

    segmentsOutputDir=os.path.join(outputDir, "segments")
    print("segmentsOutputDir", segmentsOutputDir)

    try:
        os.makedirs(segmentsOutputDir)
    except(OSError):
        print('directory ',segmentsOutputDir,' exists...moving on')

    segmentsOutRaster = os.path.join(segmentsOutputDir, os.path.basename(segmentsFile)[:-4]+'_raster')
    print("segmentsOutRaster", segmentsOutRaster)

    # Generate segmentation raster map from input shapefile
    print("layer2mask for segments: ", segmentsFile, output_vrt, segmentsOutRaster)
    burned_pixels, seg_attributes = S2_gmd.layer2mask(segmentsFile, output_vrt, segmentsOutRaster,
                                                         layer_type='segments', options=options_layer_burning)
    print('segments burned_pixels',burned_pixels)



    # Segment analysis
    # -----------------


    # Load Segmentation
    gdal_data =  gdal.Open(segmentsOutRaster)
    segments = gdal_data.ReadAsArray()
    segments_geo_transform = gdal_data.GetGeoTransform()
    segments_projection = gdal_data.GetProjection()

    ## Erode segments

    eroded_segs = np.copy(segments)
    if erode_pixels > 0:
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

    unique_segments = np.unique(eroded_segs)
    if unique_segments[0] == 0: # 0 label corresponds to not valid segments and it will be removed
        unique_segments = unique_segments[1:]

    # extract data file names and dates from file list and for specific orbits 
    par_list = S2_gmd.read_file_list(vrt_data.GetFileList(), get_par_from_file, keys, tile_list, orbit_field_label='tile_code')

    # put filename and data parameters in a pandas df
    print("fill pandas structure")
    vrt_df = pd.DataFrame.from_records(par_list, columns=keys)

    # date and time conversion from str
    vrt_df['acq_date_time'] = pd.to_datetime(vrt_df.apply(lambda x: x['acq_date']+'T'+x['acq_time'], axis=1), yearfirst=True, dayfirst=False)

    print('Extraction of stats')
    #selection of the average method                                                                                                                                                                             
    stat_p = scipy.ndimage.mean

    # Write list of df                                                                                                                                                                                           
    data_df = S2_gmd.load_stats(vrt_data, vrt_df, eroded_segs, unique_segments, seg_attributes, seg_parcel_id_attribute, stat_p, invalid_val)

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
        VIDateList = [dateutil.parser.parse(d, yearfirst=True, dayfirst=False) for d in dates_str]
        VI_seg.append({'data_type': pt, 'date_list': VIDateList, 'VI': VI_data, 'sc_fact': sf, 'corrupted_th': ct})

    # apply scale factors and remove corrupted VI
    for d in VI_seg:
        d['VI'] /= d['sc_fact']
        d['VI'][ d['VI'] < d['corrupted_th'] ] = np.nan

    # Model Extraction
    # -----------------

    print("Start params:", start_params)
    print("Bounds:", bounds)

    model_dict = {}
    for d in VI_seg:
        print("Date list:", d['date_list'])
        doy = np.array([d.timetuple().tm_yday for d in d['date_list']], dtype=float)
        print("DOY:", doy)
        model_dbl_l, x_mod, dbl_log_params_fit, _, _, message, x, y, flag = \
                        calculate_model_doy(d['VI'], doy, model_temporal_range_str, start_params, bounds, min_val_VI=d['corrupted_th'],
                                            NDVI_nomow_model_perc=NDVI_nomow_model_perc, sampling_days=sampling_days, p_n_th=p_n_th)

        if flag == 1:
            print("Model calculation is failed")
            return None, None, None, None, 1

        print('\n ****** Results of Double Logistic',\
              '\n winter NDVI        ', dbl_log_params_fit[0], \
              '\n max - min NDVI     ', dbl_log_params_fit[1],\
              '\n S infl date spring ', dbl_log_params_fit[3],\
              '\n ms rate of increase ', 1/dbl_log_params_fit[2],\
              '\n A infl date autumn ', dbl_log_params_fit[5],\
              '\n ma rate of decrease', 1/dbl_log_params_fit[4],\
              '\n message:', message)

        model_dict[d['data_type']] = np.vstack((x_mod, model_dbl_l)).T
        model_dict[d['data_type']+'_raw'] = np.vstack((x, y)).T

    # save the model
    save_model(model_dict, model_data_file_name)

    return model_dbl_l, x_mod, y, x, 0


