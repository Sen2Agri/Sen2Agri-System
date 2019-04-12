INSERT INTO config_metadata VALUES ('archiver.archive_path', 'Archive Path', 'string', false, 7);

INSERT INTO config_metadata VALUES ('archiver.max_age.l2a', 'L2A Product Max Age (days)', 'int', false, 7);
INSERT INTO config_metadata VALUES ('archiver.max_age.l3b', 'L3B Product Max Age (days)', 'int', false, 7);
INSERT INTO config_metadata VALUES ('archiver.max_age.s4c_l4a', 'L4A Product Max Age (days)', 'int', false, 7);
INSERT INTO config_metadata VALUES ('archiver.max_age.s4c_l4b', 'L4A Product Max Age (days)', 'int', false, 7);
INSERT INTO config_metadata VALUES ('archiver.max_age.s4c_l4c', 'L4A Product Max Age (days)', 'int', false, 7);

INSERT INTO config_metadata VALUES ('demmaccs.cog-tiffs', 'Produce L2A tiff files as Cloud Optimized Geotiff', 'bool', false, 16);
INSERT INTO config_metadata VALUES ('demmaccs.compress-tiffs', 'Compress the resulted L2A tiff files', 'bool', false, 16);
INSERT INTO config_metadata VALUES ('demmaccs.gips-path', 'path where the gips files are to be found', 'string', false, 16);
INSERT INTO config_metadata VALUES ('demmaccs.maccs-launcher', 'launcher for maccs within the keeping unit', 'string', false, 16);
INSERT INTO config_metadata VALUES ('demmaccs.output-path', 'path for l2a products', 'string', false, 16);
INSERT INTO config_metadata VALUES ('demmaccs.remove-fre', 'Remove FRE files from resulted L2A product', 'bool', false, 16);
INSERT INTO config_metadata VALUES ('demmaccs.remove-sre', 'Remove SRE files from resulted L2A product', 'bool', false, 16);
INSERT INTO config_metadata VALUES ('demmaccs.srtm-path', 'path where the srtm files are to be found', 'string', false, 16);
INSERT INTO config_metadata VALUES ('demmaccs.swbd-path', 'path where the swbd files are to be found', 'string', false, 16);
INSERT INTO config_metadata VALUES ('demmaccs.working-dir', 'working directory for demmaccs', 'string', false, 16);

INSERT INTO config_metadata VALUES ('downloader.enabled', 'Downloader is enabled', 'bool', false, 15);
INSERT INTO config_metadata VALUES ('downloader.l8.enabled', 'L8 downloader is enabled', 'bool', false, 15);
INSERT INTO config_metadata VALUES ('downloader.l8.max-retries', 'Maximum retries for downloading a product', 'int', false, 15);
INSERT INTO config_metadata VALUES ('downloader.l8.write-dir', 'Write directory for Landsat8', 'string', false, 15);
INSERT INTO config_metadata VALUES ('downloader.max-cloud-coverage', 'Maximum Cloud Coverage (%)', 'int', false, 15);
INSERT INTO config_metadata VALUES ('downloader.s1.enabled', 'S1 downloader is enabled', 'bool', false, 15);
INSERT INTO config_metadata VALUES ('downloader.s2.enabled', 'S2 downloader is enabled', 'bool', false, 15);
INSERT INTO config_metadata VALUES ('downloader.s2.max-retries', 'Maximum retries for downloading a product', 'int', false, 15);
INSERT INTO config_metadata VALUES ('downloader.s2.write-dir', 'Write directory for Sentinel2', 'string', false, 15);
INSERT INTO config_metadata VALUES ('downloader.start.offset', 'Season start offset in months', 'int', false, 15);

INSERT INTO config_metadata VALUES ('executor.listen-ip', 'Executor IP Address', 'string', true, 8);
INSERT INTO config_metadata VALUES ('executor.listen-port', 'Executor Port', 'int', true, 8);

INSERT INTO config_metadata VALUES ('executor.module.path.bands-extractor', 'BandsExtractor Path', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.color-mapping', 'Color Mapping Path', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.compression', 'Compression Path', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.compute-confusion-matrix', 'Compute Confusion Matrix Path', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.compute-image-statistics', 'Compute image statistics', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.compute-images-statistics', 'Compute Images Statistics Path', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.concatenate-images', 'Concatenate Images Path', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.crop-mask-fused', 'Crop mask script with stratification', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.crop-type-fused', 'Crop type script with stratification', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.data-smoothing', 'Data smoothing', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.dimensionality-reduction', 'Dimensionality reduction', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.dummy-module', 'Dummy module path', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.files-remover', 'Removes the given files (ex. cleanup of intermediate files)', 'file', false, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.gdalbuildvrt', 'Path for gdalbuildvrt', 'bool', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.gdal_translate', 'Path for gdal_translate', 'bool', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.image-classifier', 'Image Classifier Path', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.image-compression', 'Image compression', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-bv-err-image-invertion', 'Applies the error regression model to the set of input reflectances', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-bv-image-invertion', 'Applies the regression model to the set of input reflectances', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-bv-input-variable-generation', 'BV input variables generator', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-end-of-job', 'End of LAI monodate job', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-err-time-series-builder', 'Builds a raster with all error images time series', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-inverse-model-learning', 'Inverse model learning', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-local-window-reproc-splitter', 'Splits the local window LAI as image for each date', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-local-window-reprocessing', 'Executes the local window reprocessing', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-models-extractor', 'Determines the model to be used for the current execution', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-mono-date-mask-flags', 'Extracts the mask flags for the monodate LAI', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-msk-flags-time-series-builder', 'Builds a raster with all masks from the time series', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-ndvi-rvi-extractor', 'LAI NDVI and RVI features extractor', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-prosail-simulator', 'Prosail simulator', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-time-series-builder', 'Builds a raster with all time series', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.lai-training-data-generator', 'Training data generator', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.product-formatter', 'Product Formatter Path', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.xml-statistics', 'XML Statistics', 'file', true, 8);

INSERT INTO config_metadata VALUES ('executor.module.path.s4c-crop-type', 'L4A Crop Type main execution script path', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.s4c-grassland-mowing-s1', 'L4B S1 main execution script path', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.module.path.s4c-grassland-mowing-s2', 'L4B S2 main execution script path', 'file', true, 8);

INSERT INTO config_metadata VALUES ('executor.processor.l2a.name', 'L2A Processor Name', 'string', true, 8);
INSERT INTO config_metadata VALUES ('executor.processor.l2a.path', 'L2A Processor Path', 'file', false, 8);
INSERT INTO config_metadata VALUES ('executor.processor.l3b.keep_job_folders', 'Keep L3B/C/D temporary product files for the orchestrator jobs', 'int', false, 8);
INSERT INTO config_metadata VALUES ('executor.processor.l3b.name', 'L3B Processor Name', 'string', true, 8);
INSERT INTO config_metadata VALUES ('executor.processor.l3b.path', 'L3B Processor Path', 'file', false, 8);
INSERT INTO config_metadata VALUES ('executor.processor.l3b_lai.slurm_qos', 'Slurm QOS for LAI processor', 'string', true, 8);
INSERT INTO config_metadata VALUES ('executor.processor.s4c_l4a.slurm_qos', 'Slurm QOS for L4A processor', 'string', true, 8);
INSERT INTO config_metadata VALUES ('executor.processor.s4c_l4b.slurm_qos', 'Slurm QOS for L4B processor', 'string', true, 8);
INSERT INTO config_metadata VALUES ('executor.processor.s4c_l4c.slurm_qos', 'Slurm QOS for L4C processor', 'string', true, 8);

INSERT INTO config_metadata VALUES ('executor.shapes_dir', 'Tiles shapes directory', 'directory', true, 8);
INSERT INTO config_metadata VALUES ('executor.wrapper-path', 'Processor Wrapper Path', 'file', true, 8);
INSERT INTO config_metadata VALUES ('executor.wrp-executes-local', 'Execution of wrappers are only local', 'int', true, 8);
INSERT INTO config_metadata VALUES ('executor.wrp-send-retries-no', 'Number of wrapper retries to connect to executor when TCP error', 'int', true, 8);
INSERT INTO config_metadata VALUES ('executor.wrp-timeout-between-retries', 'Timeout between wrapper retries to executor when TCP error', 'int', true, 8);

INSERT INTO config_metadata VALUES ('general.scratch-path', 'Default path for temporary files', 'string', false, 1);
INSERT INTO config_metadata VALUES ('general.scratch-path.l3b_lai', 'Path for L3B/L3C/L3D temporary files', 'string', false, 1);
INSERT INTO config_metadata VALUES ('general.scratch-path.s4c_l4a', 'Path for L4A temporary files', 'string', false, 1);
INSERT INTO config_metadata VALUES ('general.scratch-path.s4c_l4b', 'Path for L4B temporary files', 'string', false, 1);
INSERT INTO config_metadata VALUES ('general.scratch-path.s4c_l4c', 'Path for L4C temporary files', 'string', false, 1);

INSERT INTO config_metadata VALUES ('http-listener.listen-port', 'Dashboard Listen Port', 'int', true, 12);
INSERT INTO config_metadata VALUES ('http-listener.root-path', 'Document Root Path', 'directory', true, 12);

INSERT INTO config_metadata VALUES ('l8.enabled', 'L8 is enabled', 'bool', false, 15);

INSERT INTO config_metadata VALUES ('monitor-agent.disk-path', 'Disk Path To Monitor For Space', 'directory', false, 13);
INSERT INTO config_metadata VALUES ('monitor-agent.scan-interval', 'Measurement Interval (s)', 'int', true, 13);

INSERT INTO config_metadata VALUES ('processor.l3b.cloud_optimized_geotiff_output', 'Generate L3B Cloud Optimized Geotiff outputs', 'bool', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.fitted', 'Specifies if fitting reprocessing (end of season) should be performed for LAI', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.generate_models', 'Specifies if models should be generated or not for LAI', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.global_bv_samples_file', 'Common LAI BV sample distribution file', 'file', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.laibandscfgfile', 'Configuration of the bands to be used for LAI', 'file', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.link_l3c_to_l3b', 'Trigger an L3C product creation after L3B product creation', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.localwnd.bwr', 'Backward radius of the window for the local algorithm', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.localwnd.fwr', 'Forward radius of the window for the local algorithm', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.lut_path', 'L3B LUT file path', 'file', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.modelsfolder', 'Folder where the models are located', 'directory', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.produce_fapar', 'L3B LAI processor will produce FAPAR', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.produce_fcover', 'L3B LAI processor will produce FCOVER', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.produce_lai', 'L3B LAI processor will produce LAI', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.produce_ndvi', 'L3B LAI processor will produce NDVI', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.rsrcfgfile', 'L3B RSR file configuration for ProsailSimulator', 'file', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.tiles_filter', 'processor.l3b.lai.tiles_filter', 'string', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.use_inra_version', 'L3B LAI processor will use INRA algorithm implementation', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.lai.use_lai_bands_cfg', 'Use LAI bands configuration file', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.mono_date_lai', 'Specifies if monodate processing should be performed for LAI', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.mono_date_ndvi_only', 'L3B processor will generate only NDVI', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.ndvi.tiles_filter', 'L3B NDVI only processor tiles filter', 'string', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.production_interval', 'The backward processing interval from the scheduled date for L3B products', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.reprocess', 'Specifies if N-Day reprocessing should be performed for LAI', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.reproc_production_interval', 'The backward processing interval from the scheduled date for L3C products', 'int', false, 4);
INSERT INTO config_metadata VALUES ('processor.l3b.sched_wait_proc_inputs', 'L3B/L3C/L3D LAI scheduled jobs wait for products to become available', 'int', false, 4);

INSERT INTO config_metadata VALUES ('processor.l4a.reference_data_dir', 'Crop Tye folder where insitu data are checked', 'string', false, 5);

INSERT INTO config_metadata VALUES ('processor.s4c_l4a.training_ratio', 'Training/total samples ratio', 'float', TRUE, 5, TRUE, 'Training/total samples ratio', '{"min":"0","step":"0.5","max":""}');
INSERT INTO config_metadata VALUES ('processor.s4c_l4a.num_trees', 'Number of RF trees', 'int', TRUE, 5, TRUE, 'Number of RF trees', '{"min":"0","step":"1","max":""}');
INSERT INTO config_metadata VALUES ('processor.s4c_l4a.sample_size', 'Sample size', 'float', TRUE, 5, TRUE, 'Sample size', '{"min":"0","step":"0.1","max":""}');
INSERT INTO config_metadata VALUES ('processor.s4c_l4a.count_threshold', 'Count threshold', 'int', TRUE, 5, TRUE, 'Count threshold', '{"min":"0","step":"1","max":""}');
INSERT INTO config_metadata VALUES ('processor.s4c_l4a.count_min', 'Minimum count', 'int', TRUE, 5, TRUE, 'Minimum count', '{"min":"0","step":"1","max":""}');
INSERT INTO config_metadata VALUES ('processor.s4c_l4a.smote_target', 'Target sample count for SMOTE', 'int', TRUE, 5, TRUE, 'Target sample count for SMOTE', '{"min":"0","step":"1","max":""}');
INSERT INTO config_metadata VALUES ('processor.s4c_l4a.smote_k', 'Number of SMOTE neighbours', 'int', TRUE, 5, TRUE, 'Number of SMOTE neighbours', '{"min":"0","step":"1","max":""}');

INSERT INTO config_metadata VALUES ('processor.s4c_l4b.input_amp', 'The list of AMP products', 'select', FALSE, 19, TRUE, 'Available AMP input files', '{"name":"inputFiles_AMP[]","product_type_id":10}');
INSERT INTO config_metadata VALUES ('processor.s4c_l4b.input_cohe', 'The list of COHE products', 'select', FALSE, 19, TRUE, 'Available COHE input files', '{"name":"inputFiles_COHE[]","product_type_id":11}');
INSERT INTO config_metadata VALUES ('processor.s4c_l4b.input_ndvi', 'The list of NDVI products', 'select', FALSE, 19, TRUE, 'Available NDVI input files', '{"name":"inputFiles_NDVI[]","product_type_id":3}');
INSERT INTO config_metadata VALUES ('processor.s4c_l4c.input_amp', 'The list of AMP products', 'select', FALSE, 20, TRUE, 'Available AMP input files', '{"name":"inputFiles_AMP[]","product_type_id":10}');
INSERT INTO config_metadata VALUES ('processor.s4c_l4c.input_cohe', 'The list of COHE products', 'select', FALSE, 20, TRUE, 'Available COHE input files', '{"name":"inputFiles_COHE[]","product_type_id":11}');
INSERT INTO config_metadata VALUES ('processor.s4c_l4c.input_ndvi', 'The list of NDVI products', 'select', FALSE, 20, TRUE, 'Available NDVI input files', '{"name":"inputFiles_NDVI[]","product_type_id":3}');

INSERT INTO config_metadata VALUES ('processor.s4c_l4b.config_path', 'The default configuration files for all L4B processors', 'file', FALSE, 19);
INSERT INTO config_metadata VALUES ('processor.s4c_l4c.config_path', 'The default configuration files for all L4C processors', 'file', FALSE, 20);


INSERT INTO config_metadata VALUES ('resources.working-mem', 'OTB applications working memory (MB)', 'int', true, 14);

INSERT INTO config_metadata VALUES ('s1.enabled', 'S1 is enabled', 'bool', false, 15);
INSERT INTO config_metadata VALUES ('s1.preprocessing.enabled', 'S1 preprocessing is enabled', 'bool', false, 15);
INSERT INTO config_metadata VALUES ('s1.preprocessing.path', 'The path where the S1 L2 products will be created', 'string', false, 15);
INSERT INTO config_metadata VALUES ('s2.enabled', 'S2 is enabled', 'bool', false, 15);

INSERT INTO config_metadata VALUES ('scheduled.lookup.enabled', 'Scheduled lookup is enabled', 'bool', false, 15);
INSERT INTO config_metadata VALUES ('scheduled.object.storage.move.deleteAfter', 'Delete the products after they were uploaded to object storage', 'bool', false, 15);
INSERT INTO config_metadata VALUES ('scheduled.object.storage.move.enabled', 'Scheduled object storage move enabled', 'bool', false, 15);
INSERT INTO config_metadata VALUES ('scheduled.object.storage.move.product.types', 'Product types to move to object storage (separated by ;)', 'string', false, 15);
INSERT INTO config_metadata VALUES ('scheduled.retry.enabled', 'Scheduled retry is enabled', 'bool', false, 15);

INSERT INTO config_metadata VALUES ('site.upload-path', 'Upload path', 'string', false, 17);