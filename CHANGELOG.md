# Change Log

## [1.3.1] - 2016-10-TBD
### Added
- A change log
- Multi-tile implementations of the Crop Type and supervised Crop Mask processors which should be faster and use less temporary disk space. The new executables are called `CropTypeFused.py` and `CropMaskFused.py`.
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
- The version number of the RPM packages is incorrect
- The multi-tile implementations of the Crop Mask and Crop Type processors are not yet documented in the manual
- The L3A, L3B, L3C processors and the quality flags extraction step of the L4A and L4B processors fail when one of the input product paths contains no directory separator. A workaround is to place a `./` before the product file.
- The dashboard previews don't match their bounds rectangle because of WGS 84 / Web Mercator projection mismatch
- The `demmaccs.py` command line help message contains positional arguments (`input` and `output`) placed immediately after optional arguments taking multiple values (`--prev-l2a-tiles` and `--prev-l2a-products-paths`). This can prove troublesome for unsuspecting users. The workaround is to put a `--` or a different optional argument before the positional arguments
- For LAI the model is created for each tile. The SDD and ATBD should be updated if another behaviour is desired and needs to be implemented.
