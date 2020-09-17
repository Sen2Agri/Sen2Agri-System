begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version in ('2.0.2', '2.0.3')) then
            raise notice 'upgrading from 2.0.2 to 2.0.3';

            raise notice 'patching 2.0.2';
            
            INSERT INTO processor (id, name, short_name, label) VALUES (3, 'L3B Vegetation Status','l3b', 'L3B &mdash; LAI/FAPAR/FCOVER/NDVI') on conflict(id) DO UPDATE SET name = 'L3B Vegetation Status', short_name = 'l3b', label = 'L3B &mdash; LAI/FAPAR/FCOVER/NDVI';
            INSERT INTO processor (id, name, short_name, label) VALUES (4, 'L3E Pheno NDVI metrics','l3e', 'L3E &mdash; Phenology Indices')
                on conflict(id) DO UPDATE SET name = 'L3E Pheno NDVI metrics', short_name = 'l3e', label = 'L3E &mdash; Phenology Indices';
            
            INSERT INTO processor (id, name, short_name, label) VALUES (12, 'S2A L3C LAI N-Days Reprocessing','s2a_l3c', 'S2A L3C &mdash; LAI N-Days Reprocessing') 
                on conflict(id) DO nothing;
            INSERT INTO processor (id, name, short_name, label) VALUES (13, 'S2A L3D LAI Fitted Reprocessing','s2a_l3d', 'S2A L3d &mdash; LAI Fitted Reprocessing') 
                on conflict(id) DO nothing;
            
            INSERT INTO config_category VALUES (2, 'L2A Atmospheric Correction', 1, false) on conflict(id) DO UPDATE SET name = 'L2A Atmospheric Correction',               allow_per_site_customization = false;

            INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l2a.s2.implementation', NULL, 'maja', '2020-09-07 14:17:52.846794+03') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l2a.s2.retry-interval', NULL, '1 day', '2020-09-07 14:36:37.906825+03') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l2a.optical.num-workers', NULL, '4', '2020-09-07 14:36:37.906825+03') on conflict DO nothing;

            INSERT INTO config_metadata VALUES ('processor.l2a.s2.implementation', 'L2A processor to use for Sentinel-2 products (`maja` or `sen2cor`)', 'string', false, 2) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('processor.l2a.s2.retry-interval', 'Retry interval for the L2A processor', 'string', false, 2) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('processor.l2a.optical.num-workers', 'Parallelism degree of the L2A processor', 'int', false, 2) on conflict DO nothing;

            DELETE FROM config WHERE key = 'processor.l3b_lai.sub_products';
            DELETE FROM config WHERE key = 'processor.l3b.lai.link_l3c_to_l3b';
            
            DELETE FROM config WHERE key = 'executor.processor.l3b_lai.slurm_qos';
            DELETE FROM config WHERE key = 'executor.processor.l3e_pheno.slurm_qos';
            INSERT INTO config(key, site_id, value) VALUES ('executor.processor.l3b.slurm_qos', NULL, 'qoslai') on conflict DO nothing;
            INSERT INTO config(key, site_id, value) VALUES ('executor.processor.l3e.slurm_qos', NULL, 'qospheno') on conflict DO nothing;

            DELETE FROM config WHERE key = 'general.scratch-path.l3b_lai';
            DELETE FROM config WHERE key = 'general.scratch-path.l3e_pheno';
            INSERT INTO config(key, site_id, value) VALUES ('general.scratch-path.l3b', NULL, '/mnt/archive/orchestrator_temp/l3b/{job_id}/{task_id}-{module}') on conflict DO nothing;
            INSERT INTO config(key, site_id, value) VALUES ('general.scratch-path.l3e', NULL, '/mnt/archive/orchestrator_temp/l3e/{job_id}/{task_id}-{module}') on conflict DO nothing;
            
            INSERT INTO config_category VALUES (24, 'S2A L3C LAI N-Days Reprocessing', 4, true) on conflict DO nothing;
            INSERT INTO config_category VALUES (25, 'S2A L3D LAI Fitted Reprocessing', 4, true)on conflict DO nothing;

            DELETE FROM config WHERE key = 'executor.module.path.bands-extractor';
            DELETE FROM config WHERE key = 'executor.module.path.composite-mask-handler';
            DELETE FROM config WHERE key = 'executor.module.path.composite-preprocessing';
            DELETE FROM config WHERE key = 'executor.module.path.composite-splitter';
            DELETE FROM config WHERE key = 'executor.module.path.composite-total-weight';
            DELETE FROM config WHERE key = 'executor.module.path.composite-update-synthesis';
            DELETE FROM config WHERE key = 'executor.module.path.composite-weigh-aot';
            DELETE FROM config WHERE key = 'executor.module.path.composite-weigh-on-clouds';
            DELETE FROM config WHERE key = 'executor.module.path.data-smoothing';
            DELETE FROM config WHERE key = 'executor.module.path.erosion';
            DELETE FROM config WHERE key = 'executor.module.path.feature-extraction';
            DELETE FROM config WHERE key = 'executor.module.path.features-with-insitu';
            DELETE FROM config WHERE key = 'executor.module.path.features-without-insitu';
            DELETE FROM config WHERE key = 'executor.module.path.lai-bv-err-image-invertion';
            DELETE FROM config WHERE key = 'executor.module.path.lai-bv-image-invertion';
            DELETE FROM config WHERE key = 'executor.module.path.lai-bv-input-variable-generation';
            DELETE FROM config WHERE key = 'executor.module.path.lai-err-time-series-builder';
            DELETE FROM config WHERE key = 'executor.module.path.lai-fitted-reproc-splitter';
            DELETE FROM config WHERE key = 'executor.module.path.lai-fitted-reprocessing';
            DELETE FROM config WHERE key = 'executor.module.path.lai-inverse-model-learning';
            DELETE FROM config WHERE key = 'executor.module.path.lai-local-window-reproc-splitter';
            DELETE FROM config WHERE key = 'executor.module.path.lai-local-window-reprocessing';
            DELETE FROM config WHERE key = 'executor.module.path.lai-models-extractor';
            DELETE FROM config WHERE key = 'executor.module.path.lai-mono-date-mask-flags';
            DELETE FROM config WHERE key = 'executor.module.path.lai-msk-flags-time-series-builder';
            DELETE FROM config WHERE key = 'executor.module.path.lai-ndvi-rvi-extractor';
            DELETE FROM config WHERE key = 'executor.module.path.lai-prosail-simulator';
            DELETE FROM config WHERE key = 'executor.module.path.lai-time-series-builder';
            DELETE FROM config WHERE key = 'executor.module.path.lai-training-data-generator';
            DELETE FROM config WHERE key = 'executor.module.path.majority-voting';
            DELETE FROM config WHERE key = 'executor.module.path.pheno-ndvi-metrics';
            DELETE FROM config WHERE key = 'executor.module.path.pheno-ndvi-metrics-splitter';
            DELETE FROM config WHERE key = 'executor.module.path.product-formatter';
            DELETE FROM config WHERE key = 'executor.module.path.random-selection';
            DELETE FROM config WHERE key = 'executor.module.path.sample-selection';
            DELETE FROM config WHERE key = 'executor.module.path.statistic-features';
            DELETE FROM config WHERE key = 'executor.module.path.temporal-features';
            DELETE FROM config WHERE key = 'executor.module.path.temporal-resampling';
            DELETE FROM config WHERE key = 'executor.module.path.train-images-classifier-new';
            DELETE FROM config WHERE key = 'executor.module.path.trimming';
            DELETE FROM config WHERE key = 'executor.module.path.xml-statistics';
            
            DELETE FROM config WHERE key = 'processor.l3b.lai.localwnd.bwr';
            DELETE FROM config WHERE key = 'processor.l3b.lai.localwnd.fwr';
            
            DELETE FROM config WHERE key = 'processor.l3b.mono_date_lai';
            DELETE FROM config WHERE key = 'processor.l3b.reprocess';
            DELETE FROM config WHERE key = 'processor.l3b.fitted';
            
            DELETE FROM config WHERE key = 'processor.l3b.mono_date_ndvi_only';
            DELETE FROM config WHERE key = 'processor.l3b.ndvi.tiles_filter';


            DELETE FROM config WHERE key = 'processor.l3b.lai.produce_ndvi';
            DELETE FROM config WHERE key = 'processor.l3b.lai.produce_lai';
            DELETE FROM config WHERE key = 'processor.l3b.lai.produce_fapar';
            DELETE FROM config WHERE key = 'processor.l3b.lai.produce_fcover';

            DELETE FROM config_metadata WHERE key = 'executor.module.path.bands-extractor';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.composite-mask-handler';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.composite-preprocessing';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.composite-splitter';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.composite-total-weight';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.composite-update-synthesis';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.composite-weigh-aot';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.composite-weigh-on-clouds';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.data-smoothing';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.erosion';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.feature-extraction';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.features-with-insitu';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.features-without-insitu';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-bv-err-image-invertion';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-bv-image-invertion';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-bv-input-variable-generation';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-err-time-series-builder';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-fitted-reproc-splitter';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-fitted-reprocessing';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-inverse-model-learning';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-local-window-reproc-splitter';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-local-window-reprocessing';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-models-extractor';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-mono-date-mask-flags';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-msk-flags-time-series-builder';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-ndvi-rvi-extractor';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-prosail-simulator';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-time-series-builder';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-training-data-generator';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.majority-voting';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.pheno-ndvi-metrics';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.pheno-ndvi-metrics-splitter';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.product-formatter';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.random-selection';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.sample-selection';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.statistic-features';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.temporal-features';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.temporal-resampling';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.train-images-classifier-new';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.trimming';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.xml-statistics';

            DELETE FROM config_metadata WHERE key = 'processor.l3b.lai.link_l3c_to_l3b';
            DELETE FROM config_metadata WHERE key = 'processor.l3b.mono_date_ndvi_only';
            DELETE FROM config_metadata WHERE key = 'processor.l3b.ndvi.tiles_filter';
            
            DELETE FROM config_metadata WHERE key = 'processor.l3b.lai.produce_ndvi';
            DELETE FROM config_metadata WHERE key = 'processor.l3b.lai.produce_lai';
            DELETE FROM config_metadata WHERE key = 'processor.l3b.lai.produce_fapar';
            DELETE FROM config_metadata WHERE key = 'processor.l3b.lai.produce_fcover';
                                                     
            DELETE FROM config_metadata WHERE key = 'executor.processor.l3b_lai.slurm_qos';
            DELETE FROM config_metadata WHERE key = 'executor.processor.l3e_pheno.slurm_qos';
            DELETE FROM config_metadata WHERE key = 'general.scratch-path.l3b_lai';
            DELETE FROM config_metadata WHERE key = 'general.scratch-path.l3e_pheno';
            
            DELETE FROM config_metadata WHERE key = 'processor.l3b.fitted';
            DELETE FROM config_metadata WHERE key = 'processor.l3b.lai.localwnd.bwr';
            DELETE FROM config_metadata WHERE key = 'processor.l3b.lai.localwnd.fwr';
            DELETE FROM config_metadata WHERE key = 'processor.l3b.mono_date_lai';
            DELETE FROM config_metadata WHERE key = 'processor.l3b.reprocess';

            INSERT INTO config_metadata VALUES ('general.scratch-path.s2a_l3c', 'Path for L3C temporary files', 'string', false, 1) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('general.scratch-path.s2a_l3d', 'Path for L3D temporary files', 'string', false, 1)on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('executor.processor.s2a_l3c.keep_job_folders', 'Keep L3C temporary product files for the orchestrator jobs', 'int', false, 8) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('executor.processor.s2a_l3d.keep_job_folders', 'Keep L3D temporary product files for the orchestrator jobs', 'int', false, 8) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('executor.processor.s2a_l3c.slurm_qos', 'Slurm QOS for LAI processor', 'string', true, 8) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('executor.processor.s2a_l3d.slurm_qos', 'Slurm QOS for LAI processor', 'string', true, 8) on conflict DO nothing;
            

            INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s2a_l3c.lut_path', NULL, '/usr/share/sen2agri/lai.map', '2020-09-03 14:08:07.963143+02') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s2a_l3d.lut_path', NULL, '/usr/share/sen2agri/lai.map', '2020-09-03 14:08:07.963143+02') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s2a_l3c.localwnd.bwr', NULL, '2', '2020-09-03 14:54:40.30341+02') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.s2a_l3c.localwnd.fwr', NULL, '0', '2020-09-03 14:54:40.387588+02') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('general.scratch-path.s2a_l3c', NULL, '/mnt/archive/orchestrator_temp/s2a_l3c/{job_id}/{task_id}-{module}', '2015-07-10 17:54:17.288095+03') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('general.scratch-path.s2a_l3d', NULL, '/mnt/archive/orchestrator_temp/s2a_l3d/{job_id}/{task_id}-{module}', '2015-07-10 17:54:17.288095+03') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.s2a_l3c.keep_job_folders', NULL, '0', '2016-03-09 16:41:20.194169+02') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.s2a_l3d.keep_job_folders', NULL, '0', '2016-03-09 16:41:20.194169+02') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.s2a_l3c.slurm_qos', NULL, 'qoslai', '2015-08-24 17:44:38.29255+03') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.processor.s2a_l3d.slurm_qos', NULL, 'qoslai', '2015-08-24 17:44:38.29255+03') on conflict DO nothing;
            
            
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.filter.produce_ndvi', NULL, '1', '2017-10-24 14:56:57.501918+02') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.filter.produce_lai', NULL, '1', '2017-10-24 14:56:57.501918+02') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.filter.produce_fapar', NULL, '1', '2017-10-24 14:56:57.501918+02') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.filter.produce_fcover', NULL, '1', '2017-10-24 14:56:57.501918+02') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.filter.produce_in_domain_flags', NULL, '0', '2017-10-24 14:56:57.501918+02') on conflict DO nothing;
            
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s1.query.days.back', NULL, '0', '2020-07-02 14:56:57.501918+02') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.s2.query.days.back', NULL, '0', '2020-07-02 14:56:57.501918+02') on conflict DO nothing;
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('downloader.l8.query.days.back', NULL, '0', '2020-07-02 14:56:57.501918+02') on conflict DO nothing;
            
            INSERT INTO config_metadata VALUES ('processor.s2a_l3c.lut_path', 'L3C LUT file path', 'file', false, 24) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('processor.s2a_l3d.lut_path', 'L3D LUT file path', 'file', false, 25) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('processor.s2a_l3c.localwnd.bwr', 'Backward radius of the window for N-day reprocessing', 'int', false, 24, true, 'Backward window') on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('processor.s2a_l3c.localwnd.fwr', 'Forward radius of the window for N-day reprocessing', 'int', true, 24, true, 'Forward window') on conflict DO nothing;
            
            INSERT INTO config_metadata VALUES ('processor.l3b.filter.produce_ndvi', 'L3B LAI processor will produce NDVI', 'int', false, 4) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('processor.l3b.filter.produce_lai', 'L3B LAI processor will produce LAI', 'int', false, 4) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('processor.l3b.filter.produce_fapar', 'L3B LAI processor will produce FAPAR', 'int', false, 4) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('processor.l3b.filter.produce_fcover', 'L3B LAI processor will produce FCOVER', 'int', false, 4) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('processor.l3b.filter.produce_in_domain_flags', 'L3B processor will input domain flags', 'int', false, 4) on conflict DO nothing;
            
            INSERT INTO config_metadata VALUES ('executor.processor.l3b.slurm_qos', 'Slurm QOS for LAI processor', 'string', true, 8) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('general.scratch-path.l3b', 'Path for L3B temporary files', 'string', false, 1) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('executor.processor.l3e.slurm_qos', 'Slurm QOS for Pheno NDVI processor', 'string', true, 8) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('general.scratch-path.l3e', 'Path for L3E temporary files', 'string', false, 1) on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('executor.processor.l3b.keep_job_folders', 'Keep L3B temporary product files for the orchestrator jobs', 'int', false, 8) on conflict(key) DO UPDATE SET friendly_name = 'Keep L3B temporary product files for the orchestrator jobs';

            INSERT INTO config_metadata VALUES ('processor.l3a.half_synthesis', 'Half synthesis interval in days', 'int', false, 3, true, 'Half synthesis') on conflict(key) DO UPDATE SET is_site_visible = true, label = 'Half synthesis' ;
            INSERT INTO config_metadata VALUES ('processor.l4a.random_seed', 'The random seed used for training', 'float', true, 5, true, 'Random seed') on conflict(key) DO UPDATE SET is_advanced  = true, is_site_visible = true , label = 'Random seed';
            INSERT INTO config_metadata VALUES ('processor.l4a.window', 'The window expressed in number of records used for the temporal features extraction', 'int', true, 5, true, 'Window records') on conflict(key) DO UPDATE SET is_advanced  = true, is_site_visible = true , label = 'Window records';
            INSERT INTO config_metadata VALUES ('processor.l4a.smoothing-lambda', 'The lambda parameter used in data smoothing', 'float', true, 5, true, 'Lambda') on conflict(key) DO UPDATE SET is_advanced  = true, is_site_visible = true , label = 'Lambda';
            INSERT INTO config_metadata VALUES ('processor.l4a.segmentation-spatial-radius', 'The spatial radius of the neighborhood used for segmentation', 'int', true, 5, true, 'Spatial radius') on conflict(key) DO UPDATE SET is_advanced  = true, is_site_visible = true , label = 'Spatial radius';
            INSERT INTO config_metadata VALUES ('processor.l4a.segmentation-minsize', 'Minimum size of a region (in pixel unit) in segmentation.', 'int', true, 5, true, 'Minim size of a region') on conflict(key) DO UPDATE SET is_advanced  = true, is_site_visible = true , label = 'Minim size of a region';
            INSERT INTO config_metadata VALUES ('processor.l4a.training-samples-number', 'The number of samples included in the training set', 'int', true, 5, true, 'Training set sample') on conflict(key) DO UPDATE SET is_advanced  = true, is_site_visible = true , label = 'Training set sample';
            
            DELETE FROM config WHERE key = 'executor.module.path.lai-end-of-job';
            DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-end-of-job';

            INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.no-operation-step', NULL, '/usr/bin/true', '2016-01-12 14:56:57.501918+02') on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('executor.module.path.no-operation-step', 'A job no operation step executable', 'string', true, 8) on conflict DO nothing;

            INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.end-of-job', NULL, '/usr/bin/true', '2016-01-12 14:56:57.501918+02') on conflict DO nothing;
            INSERT INTO config_metadata VALUES ('executor.module.path.end-of-job', 'End of a multi root steps job', 'string', true, 8) on conflict DO nothing;


            INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.reports.enabled', NULL, 'true', '2020-05-04 14:56:57.501918+02') on conflict (key, COALESCE(site_id, -1)) DO UPDATE SET value = 'true';
            INSERT INTO config(key, site_id, value, last_updated) VALUES ('scheduled.reports.interval', NULL, '24', '2020-05-04 14:56:57.501918+02') on conflict DO nothing;
        
            _statement := $str$                
                CREATE OR REPLACE FUNCTION public.check_season()
                        RETURNS TRIGGER AS
                    $BODY$
                    BEGIN
                        IF NOT EXISTS (SELECT id FROM public.season WHERE id != NEW.id AND site_id = NEW.site_id AND enabled = true AND start_date <= NEW.start_date AND end_date >= NEW.end_date) THEN
                            RETURN NEW;
                        ELSE
                            RAISE EXCEPTION 'Nested seasons are not allowed';
                        END IF;
                    END;
                    $BODY$
                    LANGUAGE plpgsql VOLATILE;
            $str$;
            raise notice '%', _statement;
            execute _statement;   

            if not exists (select tgname from pg_trigger where not tgisinternal and tgrelid = 'season'::regclass and tgname = 'check_season_dates') then
                _statement := $str$
                    CREATE TRIGGER check_season_dates 
                        BEFORE INSERT OR UPDATE ON public.season
                        FOR EACH ROW EXECUTE PROCEDURE public.check_season();
                $str$;
                raise notice '%', _statement;
                execute _statement;
            end if;
            -- new tables creation, if not exist 
            _statement := $str$
                create table if not exists product_provenance(
                    product_id int not null,
                    parent_product_id int not null,
                    parent_product_date timestamp with time zone not null,
                    constraint product_provenance_pkey primary key (product_id, parent_product_id)
                );
                
                create or replace function sp_insert_product_provenance(
                    _product_id int,
                    _parent_product_id int,
                    _parent_product_date int
                )
                returns void
                as $$
                begin
                    insert into product_provenance(product_id, parent_product_id, parent_product_date)
                    values (_product_id, _parent_product_id, _parent_product_date);
                end;
                $$
                language plpgsql volatile;
                
                create or replace function sp_is_product_stale(
                    _product_id int,
                    _parent_products int[]
                )
                returns boolean
                as $$
                begin
                    return exists (
                        select pid
                        from unnest(_parent_products) as pp(pid)
                        where not exists (
                            select *
                            from product_provenance
                            where (product_id, parent_product_id) = (_product_id, pid)
                        )
                    ) or exists (
                        select *
                        from product_provenance
                        where exists (
                            select *
                            from product
                            where product.id = product_provenance.parent_product_id
                              and product.created_timestamp >= product_provenance.parent_product_date
                        )
                    );
                end;
                $$
                language plpgsql stable;     

            $str$;
            raise notice '%', _statement;
            execute _statement;
            
            _statement := $str$
                CREATE OR REPLACE FUNCTION sp_get_dashboard_products(
                    _site_id integer[] DEFAULT NULL::integer[],
                    _product_type_id integer[] DEFAULT NULL::integer[],
                    _season_id smallint DEFAULT NULL::smallint,
                    _satellit_id integer[] DEFAULT NULL::integer[],
                    _since_timestamp timestamp with time zone DEFAULT NULL::timestamp with time zone,
                    _until_timestamp timestamp with time zone DEFAULT NULL::timestamp with time zone,
                    _tiles character varying[] DEFAULT NULL::character varying[])
                  RETURNS SETOF json AS
                $BODY$
                        DECLARE q text;
                        BEGIN
                            q := $sql$
                            WITH site_names(id, name, geog, row) AS (
                                select id, name, st_astext(geog), row_number() over (order by name)
                                from site
                                ),
                                product_type_names(id, name, description, row) AS (
                                select id, name, description, row_number() over (order by description)
                                from product_type
                                ),
                                data(id, satellite_id, product, product_type_id, product_type,product_type_description,processor,site,full_path,quicklook_image,footprint,created_timestamp, site_coord) AS (
                                SELECT
                                P.id,
                                P.satellite_id,
                                P.name,
                                PT.id,
                                PT.name,
                                PT.description,
                                PR.name,
                                S.name,
                                P.full_path,
                                P.quicklook_image,
                                P.footprint,
                                P.created_timestamp,
                                S.geog
                                FROM product P
                                JOIN product_type_names PT ON P.product_type_id = PT.id
                                JOIN processor PR ON P.processor_id = PR.id
                                JOIN site_names S ON P.site_id = S.id
                                WHERE TRUE -- COALESCE(P.is_archived, FALSE) = FALSE
                                AND EXISTS (
                                    SELECT * FROM season WHERE season.site_id =P.site_id AND P.created_timestamp BETWEEN season.start_date AND season.end_date + interval '1 day'
                            $sql$;
                            IF $3 IS NOT NULL THEN
                            q := q || $sql$
                                AND season.id=$3
                                $sql$;
                            END IF;

                            q := q || $sql$
                            )
                            $sql$;

                            IF $1 IS NOT NULL THEN
                            q := q || $sql$
                                AND P.site_id = ANY($1)
                            $sql$;
                            END IF;

                            IF $2 IS NOT NULL THEN
                            q := q || $sql$
                                AND P.product_type_id= ANY($2)

                                $sql$;
                            END IF;

                        IF $5 IS NOT NULL THEN
                        q := q || $sql$
                            AND P.created_timestamp >= to_timestamp(cast($5 as TEXT),'YYYY-MM-DD HH24:MI:SS')
                            $sql$;
                        END IF;

                        IF $6 IS NOT NULL THEN
                        q := q || $sql$
                            AND P.created_timestamp <= to_timestamp(cast($6 as TEXT),'YYYY-MM-DD HH24:MI:SS') + interval '1 day'
                            $sql$;
                        END IF;

                        IF $7 IS NOT NULL THEN
                        q := q || $sql$
                            AND P.tiles <@$7 AND P.tiles!='{}'
                            $sql$;
                        END IF;


                        q := q || $sql$
                            ORDER BY S.row, PT.row, P.name
                            )
                        --         select * from data;
                            SELECT array_to_json(array_agg(row_to_json(data)), true) FROM data;
                            $sql$;

                        --     raise notice '%', q;

                            RETURN QUERY
                            EXECUTE q
                            USING _site_id, _product_type_id, _season_id, _satellit_id, _since_timestamp, _until_timestamp, _tiles;
                        END
                        $BODY$
                  LANGUAGE plpgsql STABLE;
            $str$;
            raise notice '%', _statement;
            execute _statement;
            
        
            _statement := $str$
                DROP FUNCTION IF EXISTS sp_get_dashboard_products_nodes(integer[], integer[], smallint, integer[], timestamp with time zone, timestamp with time zone, character varying[], boolean);
                
                CREATE OR REPLACE FUNCTION sp_get_dashboard_products_nodes(
                    _user_name character varying,
                    _site_id integer[] DEFAULT NULL::integer[],
                    _product_type_id integer[] DEFAULT NULL::integer[],
                    _season_id smallint DEFAULT NULL::smallint,
                    _satellit_id integer[] DEFAULT NULL::integer[],
                    _since_timestamp timestamp with time zone DEFAULT NULL::timestamp with time zone,
                    _until_timestamp timestamp with time zone DEFAULT NULL::timestamp with time zone,
                    _tiles character varying[] DEFAULT NULL::character varying[],
                    _get_nodes boolean DEFAULT false)
                  RETURNS SETOF json AS
                $BODY$
                    DECLARE q text;
                    BEGIN
                        q := $sql$
                        WITH
                        product_type_names(id, name, description, row, is_raster) AS (
                        select id, name, description, row_number() over (order by description), is_raster
                        from product_type
                        -- LPIS products should be excluded
                        where name != 'lpis'
                        ),
                    $sql$;

                    IF $9 IS TRUE THEN
                        q := q || $sql$
                        site_names(id, name, geog, row) AS (
                        select s.id, s.name, st_astext(s.geog), row_number() over (order by s.name)
                        from site s
                        join public.user u on u.login = $1 and (u.role_id = 1 or s.id in (select * from unnest(u.site_id)))
                        ),
                        data(id, product, footprint, site_coord, product_type_id, satellite_id, is_raster) AS (

                        SELECT
                        P.id,
                        P.name,
                        P.footprint,
                        S.geog,
                        PT.id,
                        P.satellite_id,
                        PT.is_raster
                        $sql$;
                        ELSE
                        q := q || $sql$
                        site_names(id, name,  row) AS (
                        select id, name, row_number() over (order by name)
                        from site
                        ),
                          data(id, satellite_id, product_type_id, product_type_description,site, site_id) AS (
                        SELECT
                        P.id,
                        P.satellite_id,
                        PT.id,
                        PT.description,
                        S.name,
                        S.id
                         $sql$;
                    END IF;

                     q := q || $sql$
                        FROM product P
                        JOIN product_type_names PT ON P.product_type_id = PT.id
                        JOIN processor PR ON P.processor_id = PR.id
                        JOIN site_names S ON P.site_id = S.id
                        WHERE TRUE -- COALESCE(P.is_archived, FALSE) = FALSE
                        AND EXISTS (
                            SELECT * FROM season WHERE season.site_id = P.site_id AND P.created_timestamp BETWEEN season.start_date AND season.end_date + interval '1 day'
                        $sql$;
                        IF $4 IS NOT NULL THEN
                        q := q || $sql$
                        AND season.id=$4
                        $sql$;
                        END IF;

                        q := q || $sql$
                        )
                        $sql$;
                         raise notice '%', _site_id;raise notice '%', _product_type_id;raise notice '%', _satellit_id;
                        IF $2 IS NOT NULL THEN
                        q := q || $sql$
                        AND P.site_id = ANY($2)

                        $sql$;
                        END IF;

                        IF $3 IS NOT NULL THEN
                        q := q || $sql$
                        AND P.product_type_id= ANY($3)

                        $sql$;
                        END IF;

                    IF $6 IS NOT NULL THEN
                    q := q || $sql$
                        AND P.created_timestamp >= to_timestamp(cast($6 as TEXT),'YYYY-MM-DD HH24:MI:SS')
                        $sql$;
                    END IF;

                    IF $7 IS NOT NULL THEN
                    q := q || $sql$
                        AND P.created_timestamp <= to_timestamp(cast($7 as TEXT),'YYYY-MM-DD HH24:MI:SS') + interval '1 day'
                        $sql$;
                    END IF;

                    IF $8 IS NOT NULL THEN
                    q := q || $sql$
                        AND P.tiles <@$8 AND P.tiles!='{}'
                        $sql$;
                    END IF;

                    q := q || $sql$
                        ORDER BY S.row, PT.row, P.name
                        )
                    --         select * from data;
                        SELECT array_to_json(array_agg(row_to_json(data)), true) FROM data;
                        $sql$;

                        raise notice '%', q;

                        RETURN QUERY
                        EXECUTE q
                        USING _user_name, _site_id, _product_type_id, _season_id, _satellit_id, _since_timestamp, _until_timestamp, _tiles, _get_nodes;
                    END
                    $BODY$
                  LANGUAGE plpgsql STABLE
                  COST 100
                  ROWS 1000;
                ALTER FUNCTION sp_get_dashboard_products_nodes(character varying, integer[], integer[], smallint, integer[], timestamp with time zone, timestamp with time zone, character varying[], boolean)
                  OWNER TO admin;
            $str$;
            raise notice '%', _statement;
            execute _statement;              
        
            IF to_regclass('public.ix_downloader_history_product_name') IS NULL THEN
                 _statement := $str$   
                    create index ix_downloader_history_product_name on downloader_history(product_name);  
                $str$;
                raise notice '%', _statement;
                execute _statement; 
            END IF;
            IF to_regclass('public.ix_product_name') IS NULL THEN
                 _statement := $str$   
                    create index ix_product_name on product(name);                
                $str$;
                raise notice '%', _statement;
                execute _statement; 
            END IF;
            
            ALTER TABLE product_provenance DROP CONSTRAINT IF EXISTS fk_product_provenance_product_id;
            alter table product_provenance add constraint fk_product_provenance_product_id foreign key(product_id) references product(id) on delete cascade;
            ALTER TABLE product_provenance DROP CONSTRAINT IF EXISTS fk_product_provenance_parent_product_id;
            alter table product_provenance add constraint fk_product_provenance_parent_product_id foreign key(parent_product_id) references product(id) on delete cascade;

-- Update functions for updating the end timestamp of the job (also to correctly display the end date of the job in the monitoring tab)
            _statement := $str$                        
                CREATE OR REPLACE FUNCTION sp_mark_job_finished(
                IN _job_id int
                ) RETURNS void AS $$
                BEGIN

                    UPDATE job
                    SET status_id = 6, --Finished
                    status_timestamp = now(),
                    end_timestamp = now()
                    WHERE id = _job_id; 

                END;
                $$ LANGUAGE plpgsql;
            $str$;
            raise notice '%', _statement;
            execute _statement;                

            _statement := $str$                        
                CREATE OR REPLACE FUNCTION sp_mark_job_cancelled(
                IN _job_id int
                ) RETURNS void AS $$
                BEGIN

                    IF (SELECT current_setting('transaction_isolation') NOT ILIKE 'REPEATABLE READ') THEN
                        RAISE EXCEPTION 'The transaction isolation level has not been set to REPEATABLE READ as expected.' USING ERRCODE = 'UE001';
                    END IF;

                    UPDATE step
                    SET status_id = 7, --Cancelled
                    status_timestamp = now()
                    FROM task
                    WHERE task.id = step.task_id AND task.job_id = _job_id
                    AND step.status_id NOT IN (6, 8) -- Finished or failed steps can't be cancelled
                    AND step.status_id != 7; -- Prevent resetting the status on serialization error retries.

                    UPDATE task
                    SET status_id = 7, --Cancelled
                    status_timestamp = now()
                    WHERE job_id = _job_id
                    AND status_id NOT IN (6, 8) -- Finished or failed tasks can't be cancelled
                    AND status_id != 7; -- Prevent resetting the status on serialization error retries.

                    UPDATE job
                    SET status_id = 7, --Cancelled
                    status_timestamp = now(),
                    end_timestamp = now()
                    WHERE id = _job_id
                    AND status_id NOT IN (6, 7, 8); -- Finished or failed jobs can't be cancelled
                END;
                $$ LANGUAGE plpgsql;
            $str$;
            raise notice '%', _statement;
            execute _statement;                

            _statement := $str$                        
                CREATE OR REPLACE FUNCTION sp_mark_job_failed(
                IN _job_id int
                ) RETURNS void AS $$
                BEGIN
                    -- Remaining tasks should be cancelled; the task that has failed has already been marked as failed.
                    UPDATE task
                    SET status_id = 7, -- Cancelled
                    status_timestamp = now()
                    WHERE job_id = _job_id
                    AND status_id NOT IN (6, 7, 8); -- Finished, cancelled or failed tasks can't be cancelled

                    UPDATE job
                    SET status_id = 8, -- Error
                    status_timestamp = now(),
                    end_timestamp = now()
                    WHERE id = _job_id;

                END;
                $$ LANGUAGE plpgsql;
            $str$;
            raise notice '%', _statement;
            execute _statement;                

            _statement := $str$
                create or replace function sp_insert_default_scheduled_tasks(
                    _season_id season.id%type,
                    _processor_id processor.id%type default null
                )
                returns void as
                $$
                declare _site_id site.id%type;
                declare _site_name site.short_name%type;
                declare _processor_name processor.short_name%type;
                declare _season_name season.name%type;
                declare _start_date season.start_date%type;
                declare _mid_date season.start_date%type;
                begin
                    select site.short_name
                    into _site_name
                    from season
                    inner join site on site.id = season.site_id
                    where season.id = _season_id;

                    select processor.short_name
                    into _processor_name
                    from processor
                    where id = _processor_id;

                    if not found then
                        raise exception 'Invalid season id %', _season_id;
                    end if;

                    select site_id,
                           name,
                           start_date,
                           mid_date
                    into _site_id,
                         _season_name,
                         _start_date,
                         _mid_date
                    from season
                    where id = _season_id;

                    if _processor_id is null or (_processor_id = 2 and _processor_name = 'l3a') then
                        perform sp_insert_scheduled_task(
                                    _site_name || '_' || _season_name || '_L3A' :: character varying,
                                    2,
                                    _site_id :: int,
                                    _season_id :: int,
                                    2::smallint,
                                    0::smallint,
                                    31::smallint,
                                    cast((select date_trunc('month', _start_date) + interval '1 month' - interval '1 day') as character varying),
                                    60,
                                    1 :: smallint,
                                    '{}' :: json);
                    end if;

                    if _processor_id is null or (_processor_id = 3 and _processor_name = 'l3b_lai')  then
                        perform sp_insert_scheduled_task(
                                    _site_name || '_' || _season_name || '_L3B' :: character varying,
                                    3,
                                    _site_id :: int,
                                    _season_id :: int,
                                    1::smallint,
                                    1::smallint,
                                    0::smallint,
                                    cast((_start_date + 1) as character varying),
                                    60,
                                    1 :: smallint,
                                    '{"general_params":{"product_type":"L3B"}}' :: json);
                    end if;

                    if _processor_id is null or (_processor_id = 5 and _processor_name = 'l4a') then
                        perform sp_insert_scheduled_task(
                                    _site_name || '_' || _season_name || '_L4A' :: character varying,
                                    5,
                                    _site_id :: int,
                                    _season_id :: int,
                                    2::smallint,
                                    0::smallint,
                                    31::smallint,
                                    cast(_mid_date as character varying),
                                    60,
                                    1 :: smallint,
                                    '{}' :: json);
                    end if;

                    if _processor_id is null or (_processor_id = 6 and _processor_name = 'l4b') then
                        perform sp_insert_scheduled_task(
                                    _site_name || '_' || _season_name || '_L4B' :: character varying,
                                    6,
                                    _site_id :: int,
                                    _season_id :: int,
                                    2::smallint,
                                    0::smallint,
                                    31::smallint,
                                    cast(_mid_date as character varying),
                                    60,
                                    1 :: smallint,
                                    '{}' :: json);
                    end if;
                    
                    if _processor_id is null or _processor_name = 's4c_l4a' then
                        perform sp_insert_scheduled_task(
                                    _site_name || '_' || _season_name || '_S4C_L4A' :: character varying,
                                    4,
                                    _site_id :: int,
                                    _season_id :: int,
                                    2::smallint,
                                    0::smallint,
                                    31::smallint,
                                    cast(_mid_date as character varying),
                                    60,
                                    1 :: smallint,
                                    '{}' :: json);
                    end if;

                    if _processor_id is null or _processor_name = 's4c_l4b' then
                        perform sp_insert_scheduled_task(
                                    _site_name || '_' || _season_name || '_S4C_L4B' :: character varying,
                                    5,
                                    _site_id :: int,
                                    _season_id :: int,
                                    2::smallint,
                                    0::smallint,
                                    31::smallint,
                                    cast((_start_date + 31) as character varying),
                                    60,
                                    1 :: smallint,
                                    '{}' :: json);
                    end if;

                    if _processor_id is null or _processor_name = 's4c_l4c' then
                        perform sp_insert_scheduled_task(
                                    _site_name || '_' || _season_name || '_S4C_L4C' :: character varying,
                                    6,
                                    _site_id :: int,
                                    _season_id :: int,
                                    1::smallint,
                                    7::smallint,
                                    0::smallint,
                                    cast((_start_date + 7) as character varying),
                                    60,
                                    1 :: smallint,
                                    '{}' :: json);
                    end if;


                    if _processor_id is not null and (_processor_id not in (2, 3, 5, 6) and _processor_name not in ('s4c_l4a', 's4c_l4b', 's4c_l4c')) then
                        raise exception 'No default jobs defined for processor id %', _processor_id;
                    end if;

                end;
                $$
                    language plpgsql volatile;
            $str$;
            raise notice '%', _statement;
            execute _statement;

            _statement := $str$                                        
                DROP FUNCTION sp_mark_step_failed(integer,character varying,character varying,integer,bigint,bigint,bigint,integer,integer,bigint,bigint,character varying,character varying);
                CREATE OR REPLACE FUNCTION sp_mark_step_failed(
                IN _task_id int,
                IN _step_name character varying,
                IN _node_name character varying,
                IN _exit_code int,
                IN _user_cpu_ms bigint,
                IN _system_cpu_ms bigint,
                IN _duration_ms bigint,
                IN _max_rss_kb int,
                IN _max_vm_size_kb int,
                IN _disk_read_b bigint,
                IN _disk_write_b bigint,
                IN _stdout_text CHARACTER VARYING,
                IN _stderr_text CHARACTER VARYING
                ) RETURNS void AS $$
                DECLARE job_id int;
                BEGIN

                    IF (SELECT current_setting('transaction_isolation') NOT ILIKE 'REPEATABLE READ') THEN
                        RAISE EXCEPTION 'The transaction isolation level has not been set to REPEATABLE READ as expected.' USING ERRCODE = 'UE001';
                    END IF;

                    UPDATE step
                    SET status_id = CASE status_id
                                        WHEN 1 THEN 8 -- Submitted -> Error
                                        WHEN 2 THEN 8 -- PendingStart -> Error
                                        WHEN 4 THEN 8 -- Running -> Error
                                        ELSE status_id
                                    END,
                    end_timestamp = now(),
                    status_timestamp = CASE status_id
                                           WHEN 1 THEN now()
                                           WHEN 2 THEN now()
                                           WHEN 4 THEN now()
                                           ELSE status_timestamp
                                       END,
                    exit_code = _exit_code
                    WHERE name = _step_name AND task_id = _task_id 
                    AND status_id != 8; -- Prevent resetting the status on serialization error retries.

                    UPDATE task
                    SET status_id = CASE status_id
                                        WHEN 1 THEN 8 -- Submitted -> Error
                                        WHEN 4 THEN 8 -- Running -> Error
                                        ELSE status_id
                                    END,
                    status_timestamp = CASE status_id
                                           WHEN 1 THEN now()
                                           WHEN 4 THEN now()
                                           ELSE status_timestamp
                                       END
                    WHERE id = _task_id
                    AND status_id != 8; -- Prevent resetting the status on serialization error retries.

                    -- Make sure the statistics are inserted only once.
                    IF NOT EXISTS (SELECT * FROM step_resource_log WHERE step_name = _step_name AND task_id = _task_id) THEN
                        INSERT INTO step_resource_log(
                        step_name, 
                        task_id, 
                        node_name, 
                        entry_timestamp, 
                        duration_ms, 
                        user_cpu_ms, 
                        system_cpu_ms, 
                        max_rss_kb, 
                        max_vm_size_kb, 
                        disk_read_b, 
                        disk_write_b,
                        stdout_text,
                        stderr_text)
                        VALUES (
                        _step_name, 
                        _task_id, 
                        _node_name, 
                        now(), 
                        _duration_ms, 
                        _user_cpu_ms, 
                        _system_cpu_ms, 
                        _max_rss_kb, 
                        _max_vm_size_kb, 
                        _disk_read_b, 
                        _disk_write_b,
                        _stdout_text,
                        _stderr_text);
                    END IF;
                    
                    IF EXISTS (SELECT * FROM step WHERE task_id = _task_id AND name = _step_name AND status_id = 8) THEN
                        
                        SELECT task.job_id INTO job_id FROM task WHERE task.id = _task_id;
                    
                        INSERT INTO event(
                        type_id, 
                        data, 
                        submitted_timestamp)
                        VALUES (
                        8, -- StepFailed
                        ('{"job_id":' || job_id || ',"task_id":' || _task_id || ',"step_name":"' || _step_name || '"}') :: json,
                        now()
                        );
                    END IF;

                END;
                $$ LANGUAGE plpgsql;
            $str$;
            raise notice '%', _statement;
            execute _statement;
            
            _statement := $str$  
                CREATE TABLE IF NOT EXISTS fmask_history(
                    satellite_id smallint not null references satellite(id),
                    downloader_history_id int not null references downloader_history(id),
                    status_id int not null references l1_tile_status(id),
                    status_timestamp timestamp with time zone not null default now(),
                    retry_count int not null default 0,
                    failed_reason text,
                    primary key (downloader_history_id)
                );
                
                -- GRANT PRIVILEGES
                GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE fmask_history TO admin;
                GRANT SELECT,INSERT,DELETE,UPDATE ON TABLE fmask_history TO "sen2agri-service";
            
                CREATE OR REPLACE FUNCTION sp_start_fmask_l1_tile_processing()
                returns table (
                    site_id int,
                    satellite_id smallint,
                    downloader_history_id int,
                    path text) as
                $$
                declare _satellite_id smallint;
                declare _downloader_history_id int;
                declare _path text;
                declare _site_id int;
                declare _product_date timestamp;
                begin
                    if (select current_setting('transaction_isolation') not ilike 'serializable') then
                        raise exception 'Please set the transaction isolation level to serializable.' using errcode = 'UE001';
                    end if;

                    select fmask_history.satellite_id,
                           fmask_history.downloader_history_id
                    into _satellite_id,
                         _downloader_history_id
                    from fmask_history
                    where status_id = 2 -- failed
                      and retry_count < 3
                      and status_timestamp < now() - interval '1 day'
                    order by status_timestamp
                    limit 1;

                    if found then
                        select downloader_history.product_date,
                               downloader_history.full_path,
                               downloader_history.site_id
                        into _product_date,
                             _path,
                             _site_id
                        from downloader_history
                        where id = _downloader_history_id;

                        update fmask_history
                        set status_id = 1, -- processing
                            status_timestamp = now()
                        where (fmask_history.downloader_history_id) = (_downloader_history_id);
                    else
                        select distinct
                            downloader_history.satellite_id,
                            downloader_history.id,
                            downloader_history.product_date,
                            downloader_history.full_path,
                            downloader_history.site_id
                        into _satellite_id,
                            _downloader_history_id,
                            _product_date,
                            _path,
                            _site_id
                        from downloader_history
                        inner join site on site.id = downloader_history.site_id
                        where not exists (
                            select *
                            from fmask_history
                            where (fmask_history.satellite_id) = (downloader_history.satellite_id)
                              and (status_id = 1 or -- processing
                                   retry_count < 3 and status_id = 2 -- failed
                              )
                              or (fmask_history.downloader_history_id) = (downloader_history.id)
                        ) and downloader_history.status_id in (2, 5, 7) -- downloaded, processing
                        and site.enabled
                        and downloader_history.satellite_id in (1, 2) -- sentinel2, landsat8
                        order by satellite_id, product_date
                        limit 1;

                        if found then
                            insert into fmask_history (
                                satellite_id,
                                downloader_history_id,
                                status_id
                            ) values (
                                _satellite_id,
                                _downloader_history_id,
                                1 -- processing
                            );
                        end if;
                    end if;

                    if _downloader_history_id is not null then
                        return query
                            select _site_id,
                                _satellite_id,
                                _downloader_history_id,
                                _path;
                    end if;
                end;
                $$ language plpgsql volatile;
                
                CREATE OR REPLACE FUNCTION sp_mark_fmask_l1_tile_done(
                    _downloader_history_id int
                )
                returns boolean
                as
                $$
                begin
                    if (select current_setting('transaction_isolation') not ilike 'serializable') then
                        raise exception 'Please set the transaction isolation level to serializable.' using errcode = 'UE001';
                    end if;

                    update fmask_history
                    set status_id = 3, -- done
                        status_timestamp = now(),
                        failed_reason = null
                    where (downloader_history_id) = (_downloader_history_id);

                    return true;
                end;
                $$ language plpgsql volatile;
                
                CREATE OR REPLACE FUNCTION sp_mark_fmask_l1_tile_failed(
                    _downloader_history_id int,
                    _reason text,
                    _should_retry boolean
                )
                returns boolean
                as
                $$
                begin
                    if (select current_setting('transaction_isolation') not ilike 'serializable') then
                        raise exception 'Please set the transaction isolation level to serializable.' using errcode = 'UE001';
                    end if;

                    update fmask_history
                    set status_id = 2, -- failed
                        status_timestamp = now(),
                        retry_count = case _should_retry
                            when true then retry_count + 1
                            else 3
                        end,
                        failed_reason = _reason
                    where (downloader_history_id) = (_downloader_history_id);

                    return true;
                end;
                $$ language plpgsql volatile;

                CREATE OR REPLACE FUNCTION sp_clear_pending_fmask_tiles()
                returns void
                as
                $$
                begin
                    delete
                    from fmask_history
                    where status_id = 1; -- processing
                end;
                $$ language plpgsql volatile;
            $str$;
            raise notice '%', _statement;
            execute _statement;                
                
            _statement := 'update meta set version = ''2.0.3'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;
    raise notice 'complete';
end;
$migration$;

commit;


