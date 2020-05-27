# Known issues
 - The SAFE formatting application sometimes outputs mosaics with black edges around tile edges
 - The SAFE formatting application sometimes outputs unusable (e.g. black) previews
 - The SAFE and L2A product previews are not gamma-corrected and can be too dark
 - The SAFE validation step flags as invalid products with even a single tile having a low-variance band as invalid (`NOTV`)
 - The crop type and crop mask processors don't perform the normalization step properly when using SVM classification
 - The crop type and mask training step sometimes crashes while loading the OpenCV models
 - MACCS can sometimes crash or hang under high load: https://github.com/InsightSoftwareConsortium/ITK/commit/d68c1c0f23fea97ab782c185095b4ef3259cec68
 - The product formatting and tile aggregation steps are inefficient
 - Performance of the multi-tile Crop Type and Crop Mask processors can be poor for tiles with a large number of input products, especially on hardware with a large number of cores
 - The trimming step of the Crop Mask processor still uses a large amount of memory
 - The unsupervised Crop Mask processor expects a reference map with the ESA-CCI LC map labels
 - The dashboard previews don't match their bounds rectangle because of projection mismatch
 - The website has display issues on Safari
 - When deleting a site, the folders for the L2A products that were not processed by MACCS are not deleted and should be deleted manually. Normally these folders contain no valid product and contain only log and EEF files.

# Change Log
## [2.0.2]
### Added
  - Script for importing THEIA products
  - Support for using download and usage of Sen2Cor L2A products (configurable via database)
  - Support for FAPAR and FCOVER production after switching to INRA algorithm in L3B processor
  
### Changed
  - The MAJA training interval was changed to 2 months instead of 3 months
  - Changed to INRA version algorithm as the default in LAI production
  - The option to install MACCS was removed and MAJA is considered by default
  - The dialog for uploading insitu files
  - The icons in the output of the jobs from the "monitoring" tab were updated to reflect more step states.
  - L3B automatic scheduling was changed from 10 days to 1 day, ensuring each time that all the previous L2A products before the current scheduled date were processed into L3B products.
    
### Fixed
  - Updates for the new changes in USGS API
  - Corrections for avoiding skipping one product per page in SciHub

# Change Log
## [2.0.1]
### Added
  - Windows binaries of the L3A and L3B processors
  
### Fixed
  - Upgrades from previous system installations when the first installed version was older than 1.8.3
  - Corrections for SciHub downloads when sites are defined near equator or prime meridian
  - Corrections for USGS queries according to the latest changes in the USGS responses
  - Corrections for MAJA "MTD_ALL.xml is not valid according to JPI" issue.
  - PhenoNDVI processor corrections
  - Updates of the L3B processor in order to handle during the scheduling also the unprocessed products since the start of the season, not only the ones from the last week
  - Corrections for handling site polygons that have holes
  - Update of the insert_l2a_product_to_db.py in order to support also L2A MAJA format.

# Change Log
## [2.0]

### Added
  - Support for L1C zip or tar.gz archived products in L2A processor 
  - Table l1_tile_history in order to differentiate between the cases when the L2A not processed by MACCS due to any system issue and cases when the L2A not processed by MACCS because they are too cloudy. 
  - Retry mechanism in L2A processing 
  - Cloud and snow information are extracted for each L1C tile
  - Support for MAJA processing. Sen2Agri L2A processor can use MAJA instead of MACCS
  - Support for MUSCATE L2A product format in the Sen2Agri processors
  - Evolution of data sources from sen2agri-services to support local DIAS repositories (CreoDIAS, MUNDI, ONDA)
  - When multiple failures encountered for download, try download from SciHub
  - Addition of email alerts in case of abnormal functioning of services

### Changed
 - The L2A processing is now tile oriented instead of product oriented. This allows a better paralelisation especially for sites where multiple tiles were present in on L1C product.
 - L2A logging mechanism updated to keep log files in case of errors.
 

### Fixed
 - Correction in referenced Sentinel-2 tile extents (sen2agri-services)


## [1.8.3]

### Added
  - New flags for LAI monodate implementing INRA algorithm
  - Users management from the system IHM
  - Data sources configuration from the system IHM
  - Filtering products by tile, season or time interval for the Products and Custom Jobs in IHM.

### Changed
 - The map presenting the products in the IHM
 - The default data source for downloading S2 products is now SciHub and not AWS (after AWS introduced Requester Pays)
 - Updated the SciHub query datasource according to the changes performed by ESA in the requests API
 - The monitoring page to present statistics about the downloaded, downloading or estimated number of products to be downloaded.
 

### Fixed
 - Issues during downloading from SciHub or USGS.
 - The issue with configuring a new logging level in application.properties.

 
## [Patch for 1.8 & 1.8.1 version]
### Fixed
 - Handling of multi-polygons site extent geometry
 - Decimal precision in geometry
 - Handling of the time lag between the Sci-Hub and AWS acquisition availability (normally, this can be tuned by changing values in datasource table for max_retries and retry_interval).
### Added
 - A new configuration key was added in order to force queries from the start of the season instead of the last successfuly download date. The config key (that can be set only for passing again all seasons of the site and not for a nominal functionning) can be set to "config" table as "scheduled.lookup.all_products.enabled = true". After a complete pass of the season is made, this key should be removed or set to false (and sen2agri-services restared) as it does not functions correctly with the near-realtime downloading mode.
 - Support for local repositories that have paths in the format 2018/1/2 (and not 2018/01/02) by setting in the sen2agri-services.properties file the following keys:
 AWSDataSource.Sentinel2.usePadding=false
 AWSDataSource.Landsat8.usePadding=false

## [1.8.1]
### Fixed
 - LAI Multi-Date processors are now functionning also for products obtained using the new L3B monodate implementing INRA algorithm.
 
## [1.8.0]
### Added
 - The L4A/L4B processors use a platform-independent algorithm for the training/validation split. This is a step towards allowing the processor to work on other platforms and give the same results.
 - Added the sen2agri-services application that is performing the following:
    o Replaces the old downloaders. Now there are defined datasources that allow customization of sources for query and for download. For example, for S2, the query can be performed from SciHub but the download can be performed from AWS or from a local archive repository (if products are already downloaded locally)
    o Offers RESTful services for interrogating system for :
        * sites configurations, enable/disable/delete sites, retrieve the site seasons
        * enable/disable downloading for a site, for a certain satellite
        * start or stop the downloads for a site/satellite
        * retrieve or update the configuration of a datasource
        * receive notifications from the system
 - Added the possibility to delete a site from the web interface
 - Added the possibility to upload insitu and strata data from the web interface
 - Added INRA implementation for the LAI. In this moment, it works only for the Sentinel2 products and is disabled by default.  
 - GUI Products tab has now the possibility of filtering products by site, season, tiles, product type or by specifying an interval
 - Option to disable L8 (downloading and processing) for a site from the IHM
 - Added the possibility to delete a scheduled job from the Dashboard in IHM
 - Added buttons for Pause/Resume/Cancel a job in the System Monitoring page of the IHM
 - When the final product is creating, a lock file is marking that the product is not complete yet.
 - Added options in the configuration to delete SRE or FRE file or to compress the outputs when MACCS creates the final product.
 - Added a new state for the downloader_history in order to know when a product is currently processing by MACCS.
 - Display version of the system in IHM website footer 
 
### Changed
 - Breaking change: the L4A and L4B processors now use a different algorithm for the trainig/validation split. Output files will be different from the ones in the previous versions. The accuracy scores will often be lower because poorly-represented classes will now be present in the validation set. They weren't taken into account previously.
 - The SampleSelection application places all the features in both the training and the validation sets if there are too few of them and one of them would end up empty according to the configured split (e.g. 75% training with only 2 features). This allows taking them into account for validation, although with lower accuracy.
 - Removed L2A processor from the GUI Dashboard tab since it can not be launch from here
 - Changed processor names in the GUI Dashboard.
 - The downloads stops now when a site is disabled.
 - Corrected the LAI to avoid saturation of values.
 - Usage of CCI 2015 instead of CCI 2010.
 

### Fixed
 - Fixed crash in the SampleSelection application used by the L4 processors when a feature with no geometry is present
 - The SampleSelection application always puts features in one of the training or validation set. Previously, they were sometimes lost.
 - Composite Python Processor was updated to generate both 10 and 20 m resolutions.
 - Corrections in LAI and Pheno NDVI processors Python scripts.
 

## [1.7.0]
### Added
 - The unsupervised crop mask processor can now use (optionally, on by default) the red edge bands
 - Added support to `sen2agri-downloader` for ingesting data from a local archive
 - The system should now download and process Sentinel-2 B products
 - Added a new flag for L1C products processing in order to determine if the product was handled OK or not by MACCS.
 - Support in the Sentinel2 downloader in order to use local store for the L1C products.

### Changed
 - Products in the `RT` and `T2` Landsat 8 collections are no longer downloaded
 - Changed the crop mask post-filtering to ignore negative reflectance values
 - Reduced the CPU usage of the crop mask post-filtering NDVI extraction step by 20%, and the wall clock time by 15% when running with 4 threads
 - Reduced the CPU and disk usage by merging the crop mask post-filtering no data filling and PCA steps; CPU usage went down by 27% and disk usage by about 20 GB for a single tile
 - The Landsat 8 downloader no longer prints a status line if the standard output is not a TTY
 - Dropped local GDAL install and `cifs-utils` dependency
 - The Sentinel-2 downloader now accepts products that are missing a cloud coverage value, as SciHub seems to no longer return it

### Fixed
 - Fixed an issue that made the crop mask post-filtering output invalid rasters when both Sentinel-2 and Landsat 8 products were used for post-filtering; this was disabled by default
 - Changed the post-filtered crop mask data type from `Float32` to `Int16`
 - Fixed an issue that caused the supervised crop mask processor to crash on tiles spanned by multiple strata
 - Fixed an issue that sometimes caused MACCS to be passed an L3C product instead of the previous L2A one
 - Fixed a potential division by zero in the Landsat 8 downloader
 - Fixed new L8 support in `insert_l2a_product_to_db.py`
 - Fixed crash `offline_l1_handler.py` related to season dates that include years

## [1.6.1] - 2017-07-05
### Added
 - Landsat 8 collections support for the downloader

### Fixed
 - The Landsat 8 downloader now properly reports USGS authentication errors when using a proxy
 - Fixed an issue in the retrieval of season dates for sites with more than one season

## [1.6.0] - 2017-06-08
### Added
 - The possibility to select the input bands when creating the LAI monodate products, including 20M bands and possibility to use or not NDVI and RVI.
 - Scripts for deleting a site and for filtering a site tiles
 - Multiple seasons in the UI
 - The website monitoring page can now display the command line of a job and its output
 - The supervised crop mask processor can now use (optionally, on by default) features derived from the red edge bands

### Changed
 - In the LAI scheduled jobs now are taken into account the insertion dates instead of product creation date.
 - Limited the crop type processor training to `10 000` samples / tile
 - The crop type and supervised crop mask processors now include the in-situ data in the output product
 - The short name and extent of sites can no longer be updated from the website
 - The website monitoring page now sorts jobs by submission time instead of end time
 - The installer now sets `executor.listen-ip` to `127.0.0.1` and the SLURM node name to `localhost`
 - The binaries were recompiled for `GEOS 1.5`
 - Increased crop mask and crop type tile thread hint from `2` to `5` to reduce the risk of out-of-memory conditions

### Fixed
 - Changed the advanced mode for processors parameters in the sen2agri-config configuration.
 - In IPP file name for L3A is not inserted the used bands_mapping and not the one from the DB.
 - The missing L8 HDR from the L3C/L3D IPP file
 - Corrected the date order in the creation of L3C/L3D products.
 - Fixed a crash in the crop type processor when one of the tiles was missing Landsat 8 data
 - Website custom L4A jobs use a value of `40 000` training samples / tile instead of `4 000` to match the processor default

## [1.5.0] - 2017-01-20
### Added
 - Added script for cancelling all scheduled jobs
 - Added support for year in season date (not well tested yet and also not implemented yet in UI)
 - Added support for the new S2 format
 - A script to clear all running jobs is now available in the source package [Forge #151691]
 - The multi-tile implementations of the crop mask and crop type processors have been documented in the user manual [Forge #150462]

### Changed
 - The crop mask and crop type processors no longer have the `-mission` and `-prodspertile` arguments [Forge #151877]
 - L8 products no longer have to be duplicated for the crop mask and crop type processors [Forge #151877]
 - Only main mission input products are used for the crop mask segmentation step, improving accuracy of the result
 - The installer (but not the upgrade script) now changes `upload_max_filesize` in `/etc/php.ini` from `2M` to `40M` [Forge #151750]
 - The installer (but not the upgrade script) now sets `max_input_vars` in `/etc/php.ini` to `10000`
 - The crop mask no data filling step now replaces `NaN` values with `0` to avoid possible issues later when computing statistics
 - The crop mask and crop type processors now increase the soft `RLIMIT_NOFILE` limit to the hard one
 - The SNAP adapters have been updated
 - The daemons no longer log messages twice [Forge #150734]
 - The NDVI no data filling step is now around 20x faster
 - The crop mask products contain a crop/no crop "legend" in the MTD XML file [Forge #150419]
 - The "NODATA" special value is no longer duplicated in the MTD XML file
 - The crop mask and crop type processors now process multiple tiles in parallel (`-max-parallelism`) with fewer threads per tile (`-tile-threads-hint`), improving performance on large systems
 - The mosaics of the L4B products are better when `-include-raw-mask` is used
 - The `sen2agri-app` package now includes a `systemd` override to increase the `RLIMIT_NOFILE` for SLURM jobs
 - Change in IPP file in LAI multi date format to have source hdr files but also source L3B files
 - Added flags to ignore vegetation indices in LAI Mono date model creation.
 - Change for LANDSAT8 page evolutions (Error 503 during download) [Forge #151541]
 - Changed the LAI Fitted for providing bands for all dates
 - DEM creation no longer fails when SRTM tiles are missing

### Fixed
 - The automated and custom jobs no longer use tiles from a different site if two sites contain the same tile
 - Landsat8 reprojection is now performed correctly [Forge #150398]
 - Correction in L3A aggregate tiles in order to have both 10M and 20M bands (not only 20m bands)
 - Improved training pixel sampling in crop mask and crop type processors

## [1.4] - 2016-11-17
### Added
 - Multi-tile implementation of the unsupervised Crop Mask processor, which should be more precise, faster and use less temporary disk space [Forge #150414]
 - Crop mask and crop type products now include a QGIS style file [Forge #150435]
 - The crop type processor can optionally use the Sentinel-2 red edge bands via the `-red-edge` argument
 - The Sentinel-2 downloader is now compatible with the upcoming product format changes

### Changed
 - The crop mask segmentation is now performed on a "nodata-filled" version of the NDVI series. No data pixels are replaced with band averages. [Forge #150425]
 - The multi-tile crop mask and crop type classification is now faster for tiles that are intersected by multiple strata
 - Classification is no longer perform the classification on tile regions that are outside of a strata. Previously, the strata covering the most of the tile was used.
 - Strata intersecting a small region of a tile are now classified accordingly. Previously, pixels in those region used the strata covering most of the tile.
 - Crop mask and crop type try to use a system-wide LUT when they are running as a local install and another LUT is not found
 - The unsupervised crop mask processor now considers CCI-LC class 10 as crop instead of the previous rule ("11 or 20 or even 10, but only if there is no pixel with class 11") [Forge #150434]
 - Improved reprojection accuracy of crop mask reference map
 - The crop mask and crop type processors now reproject the input images if they are not in the same SRS as the images from the main sensor [Forge #150398]
 - The packaged OTB version no longer performs file existence checks before opening images

### Fixed
 - Crop mask and crop type no longer crash when the LUT is not found
 - The larger temporary files are now removed by the crop mask and crop type processors unless -keepfiles is used
 - The RPM package version numbers are now correct [Forge #150393]
 - The RPM packages correctly preserve the configuration files changed by the administrator
 - 20m composite products are now generated [Forge #150874]
 - The LEGACY_DATA mosaic now uses nearest-neighbour resampling for L4A and L4B product
 - The LEGACY_DATA mosaic now uses tile consensus projection or `EPSG:4326` instead of picking the majority one
 - The LEGACY_DATA mosaic of L4A products is no longer made from both segmented and raw masks [Forge #150432]
 - Quality flags extraction no longer gives wrong results when Landsat 8 products in a different projection are used
 - The L3A, L3B, L3C and quality flags extraction step of the L4A and L4B processors no longer crash when input products are in the current directory [Forge #150394]
 - Fixed misleading `demmaccs.py` command line help message (positional arguments placed after optional arguments taking multiple values)
 - Improved startup ordering between sen2agri-executor, SLURM and Postgres

## [1.3.1] - 2016-10-10
### Added
 - A change log
 - Multi-tile implementations of the Crop Type and supervised Crop Mask processors which should be faster and use less temporary disk space. The new executables are called `CropTypeFused.py` and `CropMaskFused.py`. [Forge #150414]
 - The processors mentioned above support classification over multiple strata in a single site
 - The processors mentioned above now create a parameters file (`IPP`) even when run in the manual mode. Note that the XML schema has changed from previous versions.
 - Upgrading from `1.3` to `1.3.1` is now possible by running the `update.sh` script. Upgrading from previous versions is not supported. Upgrading directly from `1.3` will not be possible in future versions.
 - The SAFE formatting application now accepts the desired output SRS. This is not available when running the processors. [Forge #150440]
 - A script to insert L2A products into the database (`insert_l2a_product_to_db.py`)
 - New keys in the database config for the QOS names for each processor used to limit the number of jobs of a processor to be executed at a given moment of time in Slurm
 - Added removal of temporary files also during the execution of jobs for Composite and LAI (and not only at the end)

### Changed
 - Slight performance improvement for unsupervised Crop Mask processor from writing fewer temporary files to disk
 - Reduced by about half the memory usage of the unsupervised crop mask training step
 - Reduced by about half the memory usage of the unsupervised crop mask trimming step
 - Increased default supervised crop mask training sample count from `4000` to `40000`
 - Changed default crop type and crop mask resampling mode from `gapfill` to `resample`
 - Updated SNAP adapters
 - The dashboard no longer performs AJAX requests to a web service running on port `8080`. This simplifies deployment behind a firewall and makes reverse proxy-ing possible. When configuring a reverse proxy, note that the dashboard still assumes it's running from the root of the site, so rewrite rules need to be defined accordingly.
 - The install script is now more robust against being run multiple times without dropping the `sen2agri` PostgreSQL database. This is still not a supported scenario.
 - Temporary files are now removed when running the the automated mode
 - Landsat 8 download works again by making use of the CSRF token on the USGS site
 - The `sen2agri-processors` package now increases the open file limit (`nofile`) by via a file under `/etc/security/limits.d/`

### Fixed
 - The server hostname no longer needs to be configured in `ConfigParams.php`. This should have been done by the installer, but was unreliable at times
 - The dashboard products page is now much faster
 - The dashboard no longer displays the load average multiplied by 100
 - The SAFE formatting application chose the output SRS at random when creating the `LEGACY_DATA` mosaics and previews. It now chooses the one that is the most common among the product tiles
 - The number of concurrent jobs started at once in the automated mode can now limited via the `GrpJobs` SLURM `QOS` parameter (e.g. `sacctmgr modify qos qoscomposite set GrpJobs=5`)
 - A missing `gdal-python` dependency was added to the `sen2agri-processors` package
 - Fixed proxy usage in the Sentinel-2 and Landsat 8 downloaders
 - The `sen2agri-executor` daemon and the downloader and `sen2agri-demmaccs` timers now start automatically after a system reboot
 - The `demmaccs.py` script no longer fails when given paths that don't end in a directory separator
 - Correction for low value reflectances in the composite (correction for error in ATBD)

### Security
 - The dashboard no longer calls via AJAX an unauthenticated web service endpoint
