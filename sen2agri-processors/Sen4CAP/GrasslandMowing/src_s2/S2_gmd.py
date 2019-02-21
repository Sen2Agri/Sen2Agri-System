import os
import numpy as np
import pandas as pd

import datetime

import scipy
import scipy.interpolate
import scipy.ndimage as ndimage

import gdal
from osgeo import osr, ogr
gdal.UseExceptions()
ogr.UseExceptions()
osr.UseExceptions()


def fit(x, y):
# This function calculate the N linear regressions with M points having x and y coordinates
# Inputs:
#   x: x coordinates; numpy array NxM, where N is the number of different sets of points and M is the number of (x,y) points
#   y: y coordinates; numpy array NxM, where N is the number of different sets of points and M is the number of (x,y) points
# Outputs:
#   a: intercept (model: y = b x + a); array of N elements
#   b: angular coefficient (model: y = b x + a); array of N elements

    s = x.shape[1]
    sx = x.sum(axis=1)
    sy = y.sum(axis=1)
    sxx = (x**2).sum(axis=1)
    sxy = (x*y).sum(axis=1)
    delta = s * sxx - sx**2
    a = (sxx*sy - sx*sxy) / delta
    b = (s*sxy - sx*sy) / delta

    return a, b

def norm_fun(x, alpha, bounds=(0,1)):
# Normalizzation of a quantitity varying in [0, +inf) to the interval [bound[0], bound[1])
# Input:
#   x: input values to be normalized (array)
#   alpha: parameter of the normalization function (growing rate)
#   bounds: minimum and maximum values of the destination interval
# Output:
#   normalized values (array)

    min_val = bounds[0]
    max_val = bounds[1]

    ret = max_val - (max_val - min_val)*np.exp(-alpha*x)

    return np.minimum(ret, 0.999)

def cohe_varCR(D, cohe_ENL):
# Calculation of the Cramer-Rao variance of the coherence calculation
# Input:
#   D: coherence value
#   cohe_ENL: coherence Equivalent Number of Looks
# Output:
#   Cramer-Rao variance of the coherence

    return ((1-D**2)**2)/(2*cohe_ENL)


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

def fuse_dets(date1, date2, conf, pr_date1, pr_date2, pr_conf, max_detections=4):
# This function claculate the intersection of detection intervals taking into consideration the confidence levels
# Inputs
#   date1: list of starting dates of the first set of intervals
#   date2: list of ending dates of the first set of the intervals
#   conf: list of the confidence levels of the first set of the intervals
#   pr_date1: list of starting dates of the second set of intervals
#   pr_date2: list of ending dates of the second set of intervals intervals
#   pr_conf: list of the confidence levels of the second set of the intervals
# Outputs
#   list of three elements: [list of the confidence levels, list of starting dates, list of ending dates]

    lend = len(date1)
    pr_lend = len(pr_date1)

    if lend == 0 and pr_lend == 0:
        return None, None, None

    if lend==0:
        list_interv = list(zip(*[pr_conf, pr_date1, pr_date2]))
    elif pr_lend==0:
        list_interv = list(zip(*[conf, date1, date2]))
    else:
        list_interv = []
        banned_i, banned_j = [], []
        for i in range(lend):
            for j in range(pr_lend):
                valid_inter, interv = intersection_date(date1[i], date2[i], pr_date1[j], pr_date2[j])
                if valid_inter: #conferma di detection precedente
                    list_interv.append((fuse_conf(np.array([conf[i], pr_conf[j]])), interv[0], interv[1]))
                    banned_i.append(i)
                    banned_j.append(j)
        for i in range(lend):
            if i not in banned_i:
                list_interv.append((conf[i], date1[i], date2[i]))
        for j in range(pr_lend):
            if j not in banned_j:
                list_interv.append((pr_conf[j], pr_date1[j], pr_date2[j]))
    list_interv = sorted(list_interv)[:max_detections]  # prendo le piu confidenti
    list_interv = sorted(list_interv, key=lambda x:x[1]) # riordino rispetto ai tempi
    return list(zip(*list_interv))

def cloneAndUpdateShapefile(source, dest):
# It makes a copy of the source shape file if not already existing
# Input:
#   source: file name of the input shape file
#   dest:   file name of the destination shape file
# Output:
#   None: write a copy of the file with dest file name

    print("Cloning source...")
    drv = ogr.GetDriverByName('ESRI Shapefile')
    ds = drv.CopyDataSource(ogr.Open(source), dest)
    layer = ds.GetLayerByIndex(0)

def load_stats(vrt_data, vrt_df, segments, unique_segments, seg_attributes, seg_parcel_id_attribute, stat_p, invalid_data):

    seg_num = len(unique_segments)
    df = pd.DataFrame()
    for d in vrt_df.itertuples():
        d_d = d._asdict()
        print("Load raster:", d_d['Index']+1, d_d['file_name'])
        print(datetime.datetime.now())
        band = vrt_data.GetRasterBand(d_d['Index']+1).ReadAsArray()

        print("ext_mask")
        print(datetime.datetime.now())
        ext_mask = np.logical_and(np.logical_and(np.isfinite(band), band!=invalid_data), band!=0)

        if not np.any(ext_mask):
            continue

        print("pixel_num")
        print(datetime.datetime.now())
        pixels_num = ndimage.sum(segments[ext_mask]>0, segments[ext_mask], index=unique_segments)

        print("stats")
        print(datetime.datetime.now())
        stats = stat_p(band[ext_mask], segments[ext_mask], index=unique_segments)

        print("make structure")
        print(datetime.datetime.now())
        vals = tuple([d_d[k] for k in d_d.keys()])
        data_struct = [vals + (seg_attributes[unique_segments[j]][seg_parcel_id_attribute], stats[j], pixels_num[j], unique_segments[j])
                       for j in range(seg_num)]

        print("fill pandas structure")
        print(datetime.datetime.now())
        df1 = pd.DataFrame.from_records(data_struct, columns=tuple(d_d.keys()) + tuple([seg_parcel_id_attribute, 'mean', 'count', 'fid']))

        # Concatenate pandas elements
        print("Concatenate into the panda data frames")
        print(datetime.datetime.now())
        df = pd.concat([df, df1], ignore_index=True)

    return df


# ONLY FOR S2
def read_file_list(file_list, get_par_from_file, keys, list_orbits, orbit_field_label='orbit'):

    # remove file name different from template
    filt_file_list = []
    for p in file_list:
        tmp = get_par_from_file.findall(p)
        if len(tmp)>0:
            filt_file_list.append(p)

    # extract parameters and put into list of dict
    par_list = []
    for p in filt_file_list:
        fil = os.path.basename(p)
        par = get_par_from_file.findall(fil)[-1]
        tmp_dict = {keys[i]: par[i]  for i in range(len(keys))}
        tmp_dict['file_name'] = p
        par_list.append(tmp_dict)

    # filter wrt orbit
    indx = []
    for i, p in enumerate(par_list):
        if p[orbit_field_label] in list_orbits:
            indx.append(i)

    filt_file_list = [filt_file_list[h] for h in indx]
    par_list = [par_list[h] for h in indx]

    return par_list


def remove_corrupted_files(data_list):

    ret_list = data_list.copy()
    for f in data_list:
        try:
            aux = gdal.Open(f)
        except Exception as e:
            ret_list.remove(f)
            print("Corrupted file:", f, e)

    del data_list

    return ret_list



def make_vrt(data_list, wkt_dst_srs, outputDir, output_vrt_file, outputBounds=None, resolution='average', resampling=gdal.GRA_Bilinear, srcNodata=0, error_threshold=0.125):

    if len(data_list) == 0:
        return 0

    output_tmp_file_list=[os.path.join(outputDir, os.path.basename(file)+".VRT") for file in data_list]

    for file, dst_file in zip(data_list, output_tmp_file_list):
        # Open source dataset and read source SRS
        gdal_data = gdal.Open(file)
        data_proj = gdal_data.GetProjection()
        if data_proj == '':
            d=dict(gdal.Info(file,format='json'))
            in_epsg=int(d['metadata']['GEOLOCATION']['SRS'].rsplit('"EPSG","')[-1].split('"')[0])
            src_srs =  osr.SpatialReference()
            src_srs.ImportFromEPSG(in_epsg)
            src_srs = src_srs.ExportToWkt()
        else:
            src_srs = None

        ## Call AutoCreateWarpedVRT()
        #tmp_ds = gdal.AutoCreateWarpedVRT(gdal_data, src_srs, wkt_dst_srs, resampling, error_threshold)

        ## Create the final warped raster
        #dst_ds = gdal.GetDriverByName('VRT').CreateCopy(dst_file, tmp_ds)

        # Warp to dst_srs
        dst_ds = gdal.Warp(dst_file, gdal_data, resampleAlg=resampling, srcNodata=srcNodata,
                           dstNodata=srcNodata, srcSRS=src_srs, dstSRS=wkt_dst_srs, errorThreshold=error_threshold)
        del dst_ds

    print(os.path.join(outputDir, output_vrt_file))
    if outputBounds:
        vrt_data=gdal.BuildVRT(output_vrt_file, output_tmp_file_list, separate=True, outputBounds=outputBounds,
                               resolution=resolution, srcNodata=srcNodata)
    else:
        vrt_data=gdal.BuildVRT(output_vrt_file, output_tmp_file_list, separate=True,
                               resolution=resolution, srcNodata=srcNodata)


    bands = vrt_data.RasterCount

    # to force gdal to write file
    del vrt_data

    return bands


def temporal_linear_fit(data_seg, times, smpl, linear_fit=True):
# Calculate linearly (or constant) fitted temporal trends

    data_seg_std = np.zeros_like(data_seg)
    data_seg_pred = np.zeros_like(data_seg)
    data_seg_std.fill(np.nan)
    data_seg_pred.fill(np.nan)

    for time_det in range(smpl, data_seg.shape[1]):
        data_x = np.array([(x - times[0]).days for x in times[time_det-smpl:time_det]])[None, :]

        # calculate fit
        data_y = data_seg[:, time_det-smpl:time_det]
        data_a, data_b = fit(data_x, data_y)

        # calculate statistics
        if linear_fit:
            data_mean = data_a + data_b*(times[time_det-1] - times[0]).days
            data_std = np.std(data_y - data_a[:, None] - data_b[:, None]*data_x, axis=1)
        else:
            data_mean = np.mean(data_y, axis=1)
            data_std = np.std(data_y, axis=1)

        data_seg_pred[:, time_det] = data_mean
        data_seg_std[:, time_det] = data_std

        del data_mean, data_std, data_y, data_a, data_b

    return data_seg_pred, data_seg_std


def CFAR_detection(data_seg, k_fact, data_seg_pred, data_seg_std, saturate_sigma_seg=None):
# Calculate detection and confidences

    if saturate_sigma_seg is not None:
        print(saturate_sigma_seg.shape, data_seg_std.shape)
        saturate_sigma_tmp = np.broadcast_to(saturate_sigma_seg, (data_seg_std.shape[0],))
        print(saturate_sigma_tmp.shape)
        data_seg_std = np.maximum(data_seg_std, saturate_sigma_tmp[:,None])

    data_det_cube = np.maximum((data_seg - data_seg_pred)/data_seg_std - k_fact, 0.0)

    return data_det_cube


def neighbor_smooth_trend(trend_data, cube_shape, seg_centers, percle=50, weather_radius_p=500, tile_size_p=100):

    tile_size = np.array(tile_size_p)
    rebin_factor = np.maximum(np.int_(np.round(np.array(cube_shape[1:])/tile_size)),np.array([1,1]))

    kde_st = scipy.spatial.KDTree(seg_centers)

    median_data_cube = np.zeros((trend_data.shape[1],) + tuple(rebin_factor), dtype=float)

    print("# lines:", median_data_cube.shape[1])
    for i in range(median_data_cube.shape[1]):
        print(i, end="")
        for j in range(median_data_cube.shape[2]):
            center_tile = np.array((i,j))*tile_size + tile_size/2
            close_seg = kde_st.query_ball_point(center_tile, weather_radius_p)
            if len(close_seg)>1:
                median_data_cube[:,i,j] = np.nanpercentile(trend_data[close_seg,:], 50, axis=0).T
            else:
                median_data_cube[:,i,j] = 0.0
        print("")

    return median_data_cube


def spt_interpolate_cube(in_data_cube, dst_shape, interpolation_degree=1, smoothing=0.0):

    if (in_data_cube.shape[1]==1) or (in_data_cube.shape[2]==1):
        out_fullRES_cube = in_data_cube
    else:
        axes = [np.linspace(0, dst, src, endpoint=False, retstep=True)
                for dst,src in zip(dst_shape, in_data_cube[0].shape)]
        axes = [smpl + step/2 for smpl,step in axes]
        in_data_cube_bil = [scipy.interpolate.RectBivariateSpline(*axes, in_data_cube[i], kx=interpolation_degree, ky=interpolation_degree, s=smoothing)
                            for i in range(len(in_data_cube))]

        out_fullRES_cube = np.zeros((in_data_cube.shape[0],) + (dst_shape), dtype='float32')

        y, x = np.ogrid[:dst_shape[0], :dst_shape[1]]
        for i in range(len(out_fullRES_cube)):
            out_fullRES_cube[i] = in_data_cube_bil[i](y,x)

    return out_fullRES_cube


def layer2mask(layerFile, rasterFile, outputFile, layer_type='ROI', class_attribute = None, options=['ALL_TOUCHED=False']):
# This function rasterizes the input layer file into a raster image having the map info derived from the input raster file
# Input:
#  layerFile: file name of the shape file containing geometries
#  rasterFile: file name of the raster image used to define the final projection and extention of the output rasterized data
#  outputFile: name of the output rasterized data
#  layer_type: ['ROI', 'segments', 'classes'] where respectivelly the raster is burnt with i) 1 for all geometries,
#  ii) with the FID of the geometries and finally iii) with the values of the attribute defined by the class_attribute
# Output:
#  None: results written on file

    # read raster projection and extension
    gdal_data = gdal.Open(rasterFile)
    data_geo_transform = gdal_data.GetGeoTransform()
    data_projection = gdal_data.GetProjection()
    osr_data_projection = osr.SpatialReference(wkt=osr.GetUserInputAsWKT(data_projection))
    rasterImgExtent = np.array([data_geo_transform[0],
                                data_geo_transform[0] + gdal_data.RasterXSize*data_geo_transform[1],
                                data_geo_transform[3] + gdal_data.RasterYSize*data_geo_transform[5],
                                data_geo_transform[3]])
    ImageXRes = np.abs(data_geo_transform[1])
    ImageYRes = np.abs(data_geo_transform[5])
    print("input raster transform", data_geo_transform)
    print("input raster extent", rasterImgExtent)
    print("input raster projection", osr_data_projection)

    # open the vectorial layer file
    ogr_data = ogr.Open(layerFile,0)
    Layer = ogr_data.GetLayer(0)
    LayerName = Layer.GetName()

    # create spatial index if not available
    print("Spatial index file:", layerFile[:-3]+'qix')
    if not os.path.exists(layerFile[:-3]+'qix'):
        ogr_data.ExecuteSQL('CREATE SPATIAL INDEX ON %s' % LayerName)

    if layer_type=='segments':
        Layer = ogr_data.ExecuteSQL('SELECT FID, * FROM "%s" ORDER BY OGR_GEOM_AREA DESC' % LayerName)
        wkt = 'POLYGON(({xmin} {ymin}, {xmin} {ymax}, {xmax} {ymax}, {xmax} {ymin}, {xmin} {ymin}))'.format(xmin=rasterImgExtent[0],
                                                                                                            xmax=rasterImgExtent[1],
                                                                                                            ymin=rasterImgExtent[2],
                                                                                                            ymax=rasterImgExtent[3],)
        Layer.SetSpatialFilter(ogr.CreateGeometryFromWkt(wkt))

    # make a zero-filled raster image with same size and projection of SAR data
    driver = gdal.GetDriverByName('GTiff')
    if layer_type=='ROI':
        outMask = driver.Create(outputFile, gdal_data.RasterXSize, gdal_data.RasterYSize, 1, gdal.GDT_Byte)
    elif layer_type=='segments':
        outMask = driver.Create(outputFile, gdal_data.RasterXSize, gdal_data.RasterYSize, 1, gdal.GDT_UInt32)
    elif layer_type=='classes':
        outMask = driver.Create(outputFile, gdal_data.RasterXSize, gdal_data.RasterYSize, 1, gdal.GDT_UInt32)
    outMask.SetGeoTransform(data_geo_transform)
    outMask.SetProjection(data_projection)
    outband = outMask.GetRasterBand(1)
    outband.Fill(0)
    outMask.FlushCache()

    # rasterize ROI layer over SAR data
    if layer_type=='ROI':
        gdal.RasterizeLayer(outMask, [1], Layer, burn_values=[1], options=options)
        attributes = {1: {}}
    elif layer_type=='segments':
        options.append('ATTRIBUTE=FID')
        gdal.RasterizeLayer(outMask, [1], Layer, options=options)
        attributes = {f.GetFID(): f.items() for f in Layer}
    elif layer_type=='classes':
        options.append('ATTRIBUTE='+class_attribute)
        gdal.RasterizeLayer(outMask, [1], Layer, options=options)
        attributes = {f[class_attribute]: f.items() for f in Layer}

    burned_pixels = np.count_nonzero(outMask.ReadAsArray())
    print("burned_pixels", burned_pixels)
    if burned_pixels ==0:
        print("Intersection between raster and layer is empty!")
    outband=None
    outMask=None

    return burned_pixels, attributes


