INSERT INTO config(key, site_id, value, last_updated) VALUES ('archiver.archive_path', NULL, '/mnt/archive/{site}/{processor}/', '2016-02-18 17:29:41.20487+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('archiver.max_age.l2a', NULL, '5', '2015-07-20 16:31:33.655476+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('archiver.max_age.l3b', NULL, '1', '2015-06-02 11:39:45.99546+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('archiver.max_age.s4c_l4a', NULL, '1', '2015-06-02 11:39:50.928228+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('archiver.max_age.s4c_l4b', NULL, '1', '2015-06-02 11:39:50.928228+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('archiver.max_age.s4c_l4c', NULL, '1', '2015-06-02 11:39:50.928228+03');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('demmaccs.cog-tiffs', NULL, '0', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('demmaccs.compress-tiffs', NULL, '0', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('demmaccs.gips-path', NULL, '/mnt/archive/gipp/', '2016-02-24 18:12:16.464479+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('demmaccs.maccs-launcher', NULL, '/opt/maccs/core/5.1/bin/maccs', '2016-02-25 16:29:07.763339+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('demmaccs.output-path', NULL, '/mnt/archive/maccs_def/{site}/{processor}/', '2016-02-24 18:09:17.379905+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('demmaccs.remove-fre', NULL, '0', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('demmaccs.remove-sre', NULL, '1', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('demmaccs.srtm-path', NULL, '/mnt/archive/srtm', '2016-02-25 11:11:36.372405+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('demmaccs.swbd-path', NULL, '/mnt/archive/swbd', '2016-02-25 11:12:04.008319+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('demmaccs.working-dir', NULL, '/mnt/archive/demmaccs_tmp/', '2016-02-25 17:31:06.01191+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.l8.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.l8.max-retries', NULL, '3', '2016-03-15 15:44:22.03691+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.l8.write-dir', NULL, '/mnt/archive/dwn_def/l8/default', '2016-02-26 19:30:06.821627+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.max-cloud-coverage', NULL, '100', '2016-02-03 18:05:38.425734+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s1.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s2.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s2.max-retries', NULL, '3', '2016-03-15 15:44:14.118906+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s2.write-dir', NULL, '/mnt/archive/dwn_def/s2/default', '2016-02-26 19:26:49.986675+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.start.offset', NULL, '3', '2016-07-20 20:05:00');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s1.write-dir', NULL, '/mnt/archive/dwn_def/s1/default/{site}', '2016-07-20 20:05:00');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.listen-ip', NULL, '127.0.0.1', '2015-06-03 17:03:39.541136+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.listen-port', NULL, '7777', '2015-07-07 12:17:06.182674+03');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.bands-extractor', NULL, '/usr/bin/otbcli', '2015-08-12 10:27:24.844182+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.color-mapping', NULL, '/usr/bin/otbcli_ColorMapping', '2015-11-17 17:06:25.784583+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.compression', NULL, '/usr/bin/otbcli_Convert', '2015-11-17 17:06:34.7028+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.compute-confusion-matrix', NULL, '/usr/bin/otbcli_ComputeConfusionMatrix', '2015-08-12 17:09:22.060276+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.compute-image-statistics', NULL, '/usr/bin/otbcli_ComputeImagesStatistics', '2016-02-23 12:29:52.586902+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.compute-images-statistics', NULL, '/usr/bin/otbcli_ComputeImagesStatistics', '2015-08-12 17:09:17.216345+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.concatenate-images', NULL, '/usr/bin/otbcli_ConcatenateImages', '2015-09-07 10:20:52.117401+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.crop-mask-fused', NULL, 'CropMaskFused.py', '2015-12-17 14:25:14.193131+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.crop-type-fused', NULL, 'CropTypeFused.py', '2015-12-17 14:25:14.193131+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.data-smoothing', NULL, '/usr/bin/otbcli', '2016-02-23 12:29:52.38366+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.dimensionality-reduction', NULL, 'otbcli_DimensionalityReduction', '2016-02-22 22:39:08.262715+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.dummy-module', NULL, '/usr/bin/cp', '2015-07-10 11:16:14.384866+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.feature-extraction', NULL, '/usr/bin/otbcli', '2015-08-12 17:09:15.485436+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.files-remover', NULL, '/usr/bin/rm', '2015-08-24 17:44:38.29255+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.gdalbuildvrt', NULL, '/usr/local/bin/gdalbuildvrt', '2018-08-30 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.gdal_translate', NULL, '/usr/local/bin/gdal_translate', '2018-08-30 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.image-classifier', NULL, '/usr/bin/otbcli_ImageClassifier', '2015-08-12 17:09:20.418973+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.image-compression', NULL, '/usr/bin/otbcli_Convert', '2016-02-22 22:39:08.386406+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-bv-err-image-invertion', NULL, '/usr/bin/otbcli', '2016-01-12 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-bv-image-invertion', NULL, '/usr/bin/otbcli', '2016-01-12 14:56:56.618005+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-bv-input-variable-generation', NULL, '/usr/bin/otbcli', '2016-02-15 18:45:57.225692+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-end-of-job', NULL, 'true', '2016-01-12 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-err-time-series-builder', NULL, '/usr/bin/otbcli', '2016-01-12 14:56:59.934442+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-inverse-model-learning', NULL, '/usr/bin/otbcli', '2016-02-15 18:46:01.765134+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-local-window-reproc-splitter', NULL, '/usr/bin/otbcli', '2016-01-12 14:57:01.522161+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-local-window-reprocessing', NULL, '/usr/bin/otbcli', '2016-01-12 14:57:00.637874+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-models-extractor', NULL, '/usr/bin/otbcli', '2016-01-12 14:56:55.693633+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-mono-date-mask-flags', NULL, '/usr/bin/otbcli', '2016-02-02 17:07:31.362185+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-msk-flags-time-series-builder', NULL, '/usr/bin/otbcli', '2016-02-02 17:17:09.511591+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-ndvi-rvi-extractor', NULL, '/usr/bin/otbcli', '2016-01-12 14:56:54.909962+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-prosail-simulator', NULL, '/usr/bin/otbcli', '2016-02-15 18:46:00.023288+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-time-series-builder', NULL, '/usr/bin/otbcli', '2016-01-12 14:56:58.365972+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.lai-training-data-generator', NULL, '/usr/bin/otbcli', '2016-02-15 18:46:00.887944+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.product-formatter', NULL, '/usr/bin/otbcli', '2015-11-17 17:06:45.334415+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.xml-statistics', NULL, '/usr/bin/otbcli', '2016-02-22 22:39:08.407059+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.s4c-crop-type', NULL, 'crop-type-wrapper.py', '2016-02-22 22:39:08.407059+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.s4c-grassland-mowing-s1', NULL, '/usr/share/sen2agri/S4C_L4B_GrasslandMowing/src_s1/S1_main.py', '2016-02-22 22:39:08.407059+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.s4c-grassland-mowing-s2', NULL, '/usr/share/sen2agri/S4C_L4B_GrasslandMowing/src_s2/S2_main.py', '2016-02-22 22:39:08.407059+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.l2a.name', NULL, 'L2A', '2015-06-03 17:02:50.028002+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.l2a.path', NULL, '/bin/false', '2015-07-20 16:31:23.208369+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.l3b.keep_job_folders', NULL, '0', '2016-03-09 16:41:20.194169+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.l3b.name', NULL, 'L3B', '2015-06-03 17:02:50.028002+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.l3b.path', NULL, '/bin/false', '2015-07-20 16:31:23.208369+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.l3b_lai.slurm_qos', NULL, 'qoslai', '2015-08-24 17:44:38.29255+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.s4c_l4a.slurm_qos', NULL, 'qoscroptype', '2015-08-24 17:44:38.29255+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.s4c_l4b.slurm_qos', NULL, 'qoscropmask', '2015-08-24 17:44:38.29255+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.s4c_l4c.slurm_qos', NULL, 'qoscomposite', '2015-08-24 17:44:38.29255+03');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.shapes_dir', NULL, '/mnt/archive/TilesShapesDirectory', '2015-06-03 17:03:39.541136+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.wrapper-path', NULL, '/usr/bin/sen2agri-processor-wrapper', '2015-07-23 16:54:54.092462+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.wrp-executes-local', NULL, '1', '2015-06-03 17:03:39.541136+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.wrp-send-retries-no', NULL, '3600', '2015-06-03 17:03:39.541136+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.wrp-timeout-between-retries', NULL, '1000', '2015-06-03 17:03:39.541136+03');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('general.scratch-path', NULL, '/mnt/archive/orchestrator_temp/{job_id}/{task_id}-{module}', '2015-07-10 17:54:17.288095+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('general.scratch-path.l3b_lai', NULL, '/mnt/archive/orchestrator_temp/l3b/{job_id}/{task_id}-{module}', '2015-07-10 17:54:17.288095+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('general.scratch-path.s4c_l4a', NULL, '/mnt/archive/orchestrator_temp/s4c_l4a/{job_id}/{task_id}-{module}', '2015-07-10 17:54:17.288095+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('general.scratch-path.s4c_l4b', NULL, '/mnt/archive/orchestrator_temp/s4c_l4b/{job_id}/{task_id}-{module}', '2015-07-10 17:54:17.288095+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('general.scratch-path.s4c_l4c', NULL, '/mnt/archive/orchestrator_temp/s4c_l4c/{job_id}/{task_id}-{module}', '2015-07-10 17:54:17.288095+03');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('http-listener.listen-port', NULL, '8082', '2015-07-03 13:59:21.338392+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('http-listener.root-path', NULL, '/srv/sen2agri-dashboard', '2015-07-03 13:58:57.734852+03');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('l8.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('monitor-agent.disk-path', NULL, '/mnt/archive/orchestrator_temp', '2015-07-20 10:27:29.301355+03');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('monitor-agent.scan-interval', NULL, '60', '2015-07-20 10:28:08.27395+03');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.cloud_optimized_geotiff_output', NULL, '0', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.fitted', NULL, '0', '2016-02-29 12:03:27.438284+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.generate_models', NULL, '1', '2016-02-29 12:03:08.445828+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.global_bv_samples_file', NULL, '/usr/share/sen2agri/LaiCommonBVDistributionSamples.txt', '2016-02-29 14:08:07.963143+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.laibandscfgfile', NULL, '/usr/share/sen2agri/Lai_Bands_Cfgs_Belcam.cfg', '2016-02-16 11:54:47.223904+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.link_l3c_to_l3b', NULL, '0', '2016-02-29 14:08:07.963143+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.localwnd.bwr', NULL, '2', '2016-01-12 14:54:40.30341+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.localwnd.fwr', NULL, '0', '2016-01-12 14:54:40.387588+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.lut_path', NULL, '/usr/share/sen2agri/lai.map', '2016-02-29 14:08:07.963143+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.modelsfolder', NULL, '/mnt/archive/L3B_GeneratedModels/', '2016-02-16 11:54:47.123972+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.produce_fapar', NULL, '1', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.produce_fcover', NULL, '1', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.produce_lai', NULL, '1', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.produce_ndvi', NULL, '1', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.rsrcfgfile', NULL, '/usr/share/sen2agri/rsr_cfg.txt', '2016-02-16 11:54:47.223904+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.tiles_filter', NULL, '', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.use_inra_version', NULL, '1', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.lai.use_lai_bands_cfg', NULL, '1', '2016-02-16 11:54:47.223904+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.mono_date_lai', NULL, '0', '2016-02-29 12:03:15.149961+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.mono_date_ndvi_only', NULL, '0', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.ndvi.tiles_filter', NULL, '', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.production_interval', NULL, '10', '2016-02-29 12:03:31.197823+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.reprocess', NULL, '0', '2016-02-29 12:03:21.142049+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.reproc_production_interval', NULL, '30', '2016-02-29 12:03:31.197823+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.sched_wait_proc_inputs', NULL, '0', '2015-07-10 17:54:17.288095+03');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l4a.reference_data_dir', NULL, 'N/A', '2019-02-19 11:09:58.820032+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4a.training_ratio', NULL, '0.5', '2019-02-19 11:09:58.820032+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4a.num_trees', NULL, '300', '2019-02-19 11:09:58.820032+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4a.sample_size', NULL, '0.2', '2019-02-19 11:09:58.820032+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4a.count_threshold', NULL, '1000', '2019-02-19 11:09:58.820032+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4a.count_min', NULL, '10', '2019-02-19 11:09:58.820032+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4a.smote_target', NULL, '1000', '2019-02-19 11:09:58.820032+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4a.smote_k', NULL, '5', '2019-02-19 11:09:58.820032+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4b.input_amp', NULL, 'N/A', '2019-02-19 11:09:58.820032+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4b.input_cohe', NULL, 'N/A', '2019-02-19 11:10:25.068169+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4b.input_ndvi', NULL, 'N/A', '2019-02-19 11:09:43.978921+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4c.input_amp', NULL, 'N/A', '2019-02-18 15:28:22.404745+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4c.input_cohe', NULL, 'N/A', '2019-02-18 15:28:41.060339+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4c.input_ndvi', NULL, 'N/A', '2019-02-18 15:27:41.861613+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4b.config_path', NULL, '/usr/share/sen2agri/S4C_L4B_Configurations/S4C_L4B_Config.cfg', '2019-02-18 15:27:41.861613+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4c.config_path', NULL, '/usr/share/sen2agri/S4C_L4C_Configurations/S4C_L4C_Config_NLD.cfg', '2019-02-18 15:27:41.861613+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.lpis.path', NULL, '/mnt/archive/lpis/{site}', '2019-06-11 16:15:00.0+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('resources.working-mem', NULL, '1024', '2015-09-08 11:03:21.87284+03');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('s1.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('s1.preprocessing.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('s1.preprocessing.path', NULL, '/mnt/archive/{site}/l2a-s1', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('s1.preprocessing.work.dir', NULL, '/mnt/archive/s1_preprocessing_work_dir', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('s2.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.lookup.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.object.storage.move.deleteAfter', NULL, 'false', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.object.storage.move.enabled', NULL, 'false', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.object.storage.move.product.types', NULL, '', '2017-10-24 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.retry.enabled', NULL, 'true', '2017-10-24 14:56:57.501918+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('site.upload-path', NULL, '/mnt/upload/{user}', '2016-03-01 15:02:31.980394+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s2.forcestart', NULL, 'false', '2019-04-12 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.l8.forcestart', NULL, 'false', '2019-04-12 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s1.forcestart', NULL, 'false', '2019-04-12 14:56:57.501918+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.skip.existing', NULL, 'false', '2019-04-12 14:56:57.501918+02');

INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b_lai.sub_products', NULL, 'L3B,L3C', '2019-04-12 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4c.sub_steps', NULL, 'ALL,DataExtraction,CatchCrop,Fallow,NFC,HarvestOnly', '2019-04-12 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4c.nrt_data_extr_enabled', NULL, 'true', '2019-04-12 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4c.ts_input_tables_dir', NULL, '/mnt/archive/agric_practices_files/input_files', '2019-04-12 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4c.ndvi_data_extr_dir', NULL, '/mnt/archive/agric_practices_files/data_extraction/ndvi', '2019-04-12 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4c.amp_data_extr_dir', NULL, '/mnt/archive/agric_practices_files/data_extraction/amp', '2019-04-12 14:56:57.501918+02');
INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s4c_l4c.cohe_data_extr_dir', NULL, '/mnt/archive/agric_practices_files/data_extraction/cohe', '2019-04-12 14:56:57.501918+02');

