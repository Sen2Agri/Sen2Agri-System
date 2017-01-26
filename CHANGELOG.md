# Change Log

## [1.5.0] - 2016-01-20
### Added
 - Added script for cancelling all scheduled jobs
 - Added support for year in season date (not well tested yet and also not implemented yet in UI)
 - Added support for the new S2 format

### Changed
 - The crop mask and crop type processors no longer have the `-mission` and `-prodspertile` arguments [Forge #151877]
 - L8 products no longer have to be duplicated for the crop mask and crop type processors [Forge #151877]
 - Only main mission input products are used for the crop mask segmentation step, improving accuracy of the result
 - The installer (but not the upgrade script) now changes `upload_max_filesize` in `/etc/php.ini` from `2M` to `40M`
 - The installer (but not the upgrade script) now sets `max_input_vars` in `/etc/php.ini` to `10000`
 - The crop mask no data filling step now replaces `NaN` values with `0` to avoid possible issues later when computing statistics
 - The crop mask and crop type processors now increase the soft `RLIMIT_NOFILE` limit to the hard one
 - The SNAP adapters have been updated
 - The daemons no longer log messages twice [Forge #150734]
 - The NDVI no data filling step is now around 20x faster
 - The crop mask products contain a crop/no crop "legend" in the MTD XML file [Forge #150419]
 - The "NODATA" special value is no longer duplicated in the MTD XML file
 - The crop mask and crop type processors now process multiple tiles in parallel (`-max-parallelism`) with fewer threads per tile (`-tile-threads-hint`), improving performance on large systems
 - Improved performance of the quality flags extraction step when Landsat8 inputs are present
 - Increased reprojection resampler grid spacing to 200 m
 - The mosaics of the L4B products are better when `-include-raw-mask` is used
 - The `sen2agri-app` package now includes a `systemd` override to increase the `RLIMIT_NOFILE` for SLURM jobs
 - Change in IPP file in LAI multi date format to have source hdr files but also source L3B files
 - Added flags to ignore vegetation indices in LAI Mono date model creation.
 - Change for LANDSAT8 page evolutions (Error 503 during download)
 - Changed the LAI Fitted for providing bands for all dates
 - DEM creation no longer fails when SRTM tiles are missing

### Fixed
 - The automated and custom jobs no longer use tiles from a different site if two sites contain the same tile
 - Landsat8 reprojection is now performed correctly
 - Correction in L3A aggregate tiles in order to have both 10M and 20M bands (not only 20m bands)

### Known issues
- The multi-tile implementations of the Crop Mask and Crop Type processors are not yet documented
- The training/validation polygon splitting step of the Crop Type and supervised Crop Mask processors can lose polygons
- With multiple input tiles, the training pixel sampling for crop type and crop mask products can be skewed if the training classes are not uniformly distributed
- The SAFE formatting application sometimes outputs mosaics with black edges around tile edges
- The SAFE formatting application sometimes outputs unusable (e.g. black) previews
- The SAFE and L2A product previews are not gamma-corrected and can be too dark
- The crop type and crop mask processors don't perform the normalization step properly when using SVM classification
- The crop type and mask training step sometimes crashes while loading the OpenCV models
- MACCS and quicklook generation sometimes crash or hang
- The product formatting and tile aggregation steps are inefficient
- Performance of the multi-tile Crop Type and Crop Mask processors can be poor for tiles with a large number of input products, especially on hardware with a large number of cores
- The trimming step of the Crop Mask processor still uses a large amount of memory
- The unsupervised Crop Mask processor expects a reference map with the ESA-CCI LC map labels
- The dashboard previews don't match their bounds rectangle because of projection mismatch
- The LAI model is created for each tile. The SDD and ATBD should be updated if another behaviour is desired and needs to be implemented.

## [1.4] - 2016-11-17
### Added
- Multi-tile implementation of the unsupervised Crop Mask processor, which should be more precise, faster and use less temporary disk space [Forge #150414]
- Crop mask and crop type products now include a QGIS style file
- The crop type processor can optionally use the Sentinel-2 red edge bands via the `-red-edge` argument
- The Sentinel-2 downloader is now compatible with the upcoming product format changes

### Changed
- The crop mask segmentation is now performed on a "nodata-filled" version of the NDVI series. No data pixels are replaced with band averages.
- The multi-tile crop mask and crop type classification is now faster for tiles that are intersected by multiple strata
- Classification is no longer perform the classification on tile regions that are outside of a strata. Previously, the strata covering the most of the tile was used.
- Strata intersecting a small region of a tile are now classified accordingly. Previously, pixels in those region used the strata covering most of the tile.
- Crop mask and crop type try to use a system-wide LUT when they are running as a local install and another LUT is not found
- The unsupervised crop mask processor now considers CCI-LC class 10 as crop instead of the previous rule ("11 or 20 or even 10, but only if there is no pixel with class 11") [Forge #150434]
- Improved reprojection accuracy of crop mask reference map
- The crop mask and crop type processors now reproject the input images if they are not in the same SRS as the images from the main sensor
- The packaged OTB version no longer performs file existence checks before opening images

### Fixed
- Crop mask and crop type no longer crash when the LUT is not found
- The larger temporary files are now removed by the crop mask and crop type processors unless -keepfiles is used
- The RPM package version numbers are now correct [Forge #150393]
- The RPM packages correctly preserve the configuration files changed by the administrator
- 20m composite products are now generated
- The LEGACY_DATA mosaic now uses nearest-neighbour resampling for L4A and L4B product
- The LEGACY_DATA mosaic now uses tile consensus projection or `EPSG:4326` instead of picking the majority one
- The LEGACY_DATA mosaic of L4A products is no longer made from both segmented and raw masks [Forge #150432]
- Quality flags extraction no longer gives wrong results when Landsat 8 products in a different projection are used
- The L3A, L3B, L3C and quality flags extraction step of the L4A and L4B processors no longer crash when input products are in the current directory [Forge #150394]
- Fixed misleading `demmaccs.py` command line help message (positional arguments placed after optional arguments taking multiple values)
- Improved startup ordering between sen2agri-executor, SLURM and Postgres

### Known issues
- The multi-tile implementations of the Crop Mask and Crop Type processors are not yet documented
- SNAP adapters need to be updated
- The training/validation polygon splitting step of the Crop Type and supervised Crop Mask processors can lose polygons
- With multiple input tiles, the training pixel sampling for crop type and crop mask products can be skewed if the training classes are not uniformly distributed
- The SAFE formatting application sometimes outputs mosaics with black edges around tile edges
- The SAFE formatting application sometimes outputs unusable (e.g. black) previews
- The SAFE and L2A product previews are not gamma-corrected and can be too dark
- The crop type and crop mask processors don't perform the normalization step properly when using SVM classification
- The crop type and mask training step sometimes crashes while loading the OpenCV models
- MACCS and quicklook generation sometimes crash or hang
- The NDVI no data filling step is inefficient
- The product formatting and tile aggregation steps are inefficient
- Performance of the multi-tile Crop Type and Crop Mask processors can be poor for tiles with a large number of input products, especially on hardware with a large number of cores
- The trimming step of the Crop Mask processor still uses a large amount of memory
- The unsupervised Crop Mask processor expects a reference map with the ESA-CCI LC map labels
- The dashboard previews don't match their bounds rectangle because of projection mismatch
- The LAI model is created for each tile. The SDD and ATBD should be updated if another behaviour is desired and needs to be implemented.

## [1.3.1] - 2016-10-10
### Added
- A change log
- Multi-tile implementations of the Crop Type and supervised Crop Mask processors which should be faster and use less temporary disk space. The new executables are called `CropTypeFused.py` and `CropMaskFused.py`. [Forge #150414]
- The processors mentioned above support classification over multiple strata in a single site
- The processors mentioned above now create a parameters file (`IPP`) even when run in the manual mode. Note that the XML schema has changed from previous versions.
- Upgrading from `1.3` to `1.3.1` is now possible by running the `update.sh` script. Upgrading from previous versions is not supported. Upgrading directly from `1.3` will not be possible in future versions.
- The SAFE formatting application now accepts the desired output SRS. This is not available when running the processors.
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

### Known issues
- The training/validation polygon splitting step of the Crop Type and supervised Crop Mask processors can lose polygons
- With multiple input tiles, the training pixel sampling distribution for crop type and crop mask products can be skewed if the training classes are not uniformly distributed
- Performance of the multi-tile Crop Type and Crop Mask processors can be poor for tiles with a large number of input products
- The trimming step of the Crop Mask processor still uses a large amount of memory
- The SAFE formatting application uses both segmented and raw crop masks for the mosaic
- The SAFE formatting application outputs mosaics that are outside of the SRS bounds for products spanning multiple SRSs. It should use `EPSG:4326/WGS 84` unless told otherwise.
- The SAFE formatting application sometimes outputs mosaics with black edges around tile edges when some tiles need to be reprojected.
- The SAFE formatting application sometimes outputs unusable previews
- The SAFE formatting application uses bilinear resampling for the crop type and crop mask mosaics. It should use nearest-neighbour instead.
- The L4A and L4B products don't contain a QGIS style file
- The unsupervised Crop Mask processor expects a reference map with the ESA-CCI LC map labels. It should expect a binary map.
- The SAFE and L2A product previews are not gamma-corrected and can be too dark
- The version number of the RPM packages is incorrect [Forge #150393]
- The multi-tile implementations of the Crop Mask and Crop Type processors are not yet documented in the manual
- The L3A, L3B, L3C processors and the quality flags extraction step of the L4A and L4B processors fail when one of the input product paths contains no directory separator. A workaround is to place a `./` before the product file. [Forge #150394]
- The dashboard previews don't match their bounds rectangle because of WGS 84 / Web Mercator projection mismatch
- The `demmaccs.py` command line help message contains positional arguments (`input` and `output`) placed immediately after optional arguments taking multiple values (`--prev-l2a-tiles` and `--prev-l2a-products-paths`). This can prove troublesome for unsuspecting users. The workaround is to put a `--` or a different optional argument before the positional arguments
- For LAI the model is created for each tile. The SDD and ATBD should be updated if another behaviour is desired and needs to be implemented.
