begin transaction;

do $migration$
declare _statement text;
begin
    raise notice 'running migrations';

    if exists (select * from information_schema.tables where table_schema = 'public' and table_name = 'meta') then
        if exists (select * from meta where version in ('1.2', '1.3')) then
            _statement := $str$    
                INSERT INTO processor (id, name, short_name, label) VALUES (3, 'L3B Vegetation Status','l3b', 'L3B &mdash; LAI/FAPAR/FCOVER/NDVI') on conflict(id) DO UPDATE SET name = 'L3B Vegetation Status', short_name = 'l3b', label = 'L3B &mdash; LAI/FAPAR/FCOVER/NDVI';

                DELETE FROM config WHERE key = 'processor.l3b.lai.link_l3c_to_l3b';
                DELETE FROM config WHERE key = 'processor.l3b_lai.sub_products';

                DELETE FROM config WHERE key = 'executor.processor.l3b_lai.slurm_qos';
                DELETE FROM config WHERE key = 'general.scratch-path.l3b_lai';
                INSERT INTO config(key, site_id, value) VALUES ('executor.processor.l3b.slurm_qos', NULL, 'qoslai') on conflict DO nothing;
                INSERT INTO config(key, site_id, value) VALUES ('general.scratch-path.l3b', NULL, '/mnt/archive/orchestrator_temp/l3b/{job_id}/{task_id}-{module}') on conflict DO nothing;

                INSERT INTO config_category VALUES (2, 'L2A Atmospheric Correction', 1, false) on conflict(id) DO UPDATE SET name = 'L2A Atmospheric Correction',               allow_per_site_customization = false;

                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l2a.s2.implementation', NULL, 'maja', '2020-09-07 14:17:52.846794+03') on conflict DO nothing;
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l2a.s2.retry-interval', NULL, '1 day', '2020-09-07 14:36:37.906825+03') on conflict DO nothing;
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l2a.optical.num-workers', NULL, '4', '2020-09-07 14:36:37.906825+03') on conflict DO nothing;
            
                INSERT INTO config_metadata VALUES ('processor.l2a.s2.implementation', 'L2A processor to use for Sentinel-2 products (`maja` or `sen2cor`)', 'string', false, 2) on conflict DO nothing;
                INSERT INTO config_metadata VALUES ('processor.l2a.s2.retry-interval', 'Retry interval for the L2A processor', 'string', false, 2) on conflict DO nothing;
                INSERT INTO config_metadata VALUES ('processor.l2a.optical.num-workers', 'Parallelism degree of the L2A processor', 'int', false, 2) on conflict DO nothing;
            
                DELETE FROM config WHERE key = 'executor.module.path.bands-extractor';
                DELETE FROM config WHERE key = 'executor.module.path.data-smoothing';
                DELETE FROM config WHERE key = 'executor.module.path.feature-extraction';
                DELETE FROM config WHERE key = 'executor.module.path.lai-bv-err-image-invertion';
                DELETE FROM config WHERE key = 'executor.module.path.lai-bv-image-invertion';
                DELETE FROM config WHERE key = 'executor.module.path.lai-bv-input-variable-generation';
                DELETE FROM config WHERE key = 'executor.module.path.lai-err-time-series-builder';
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
                DELETE FROM config WHERE key = 'executor.module.path.product-formatter';
                DELETE FROM config WHERE key = 'executor.module.path.xml-statistics';
                DELETE FROM config WHERE key = 'executor.module.path.dummy-module';
            
                DELETE FROM config_metadata WHERE key = 'executor.module.path.bands-extractor';
                DELETE FROM config_metadata WHERE key = 'executor.module.path.data-smoothing';
                DELETE FROM config_metadata WHERE key = 'executor.module.path.feature-extraction';
                DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-bv-err-image-invertion';
                DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-bv-image-invertion';
                DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-bv-input-variable-generation';
                DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-err-time-series-builder';
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
                DELETE FROM config_metadata WHERE key = 'executor.module.path.product-formatter';
                DELETE FROM config_metadata WHERE key = 'executor.module.path.xml-statistics';
                DELETE FROM config_metadata WHERE key = 'executor.module.path.dummy-module';
            
            
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

                DELETE FROM config_metadata WHERE key = 'processor.l3b.lai.link_l3c_to_l3b';
                DELETE FROM config_metadata WHERE key = 'processor.l3b.mono_date_ndvi_only';
                DELETE FROM config_metadata WHERE key = 'processor.l3b.ndvi.tiles_filter';
                
                DELETE FROM config_metadata WHERE key = 'processor.l3b.lai.produce_ndvi';
                DELETE FROM config_metadata WHERE key = 'processor.l3b.lai.produce_lai';
                DELETE FROM config_metadata WHERE key = 'processor.l3b.lai.produce_fapar';
                DELETE FROM config_metadata WHERE key = 'processor.l3b.lai.produce_fcover';
                
                DELETE FROM config_metadata WHERE key = 'executor.processor.l3b_lai.slurm_qos';
                DELETE FROM config_metadata WHERE key = 'general.scratch-path.l3b_lai';
                DELETE FROM config_metadata WHERE key = 'processor.l3b.fitted';
                DELETE FROM config_metadata WHERE key = 'processor.l3b.lai.localwnd.fwr';
                DELETE FROM config_metadata WHERE key = 'processor.l3b.mono_date_lai';
                DELETE FROM config_metadata WHERE key = 'processor.l3b.reprocess';

                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.filter.produce_ndvi', NULL, '1', '2017-10-24 14:56:57.501918+02') on conflict DO nothing;
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.filter.produce_lai', NULL, '1', '2017-10-24 14:56:57.501918+02') on conflict DO nothing;
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.filter.produce_fapar', NULL, '1', '2017-10-24 14:56:57.501918+02') on conflict DO nothing;
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.filter.produce_fcover', NULL, '1', '2017-10-24 14:56:57.501918+02') on conflict DO nothing;
                INSERT INTO config(key, site_id, value, last_updated) VALUES ('processor.l3b.filter.produce_in_domain_flags', NULL, '0', '2017-10-24 14:56:57.501918+02') on conflict DO nothing;

                INSERT INTO config_metadata VALUES ('processor.l3b.filter.produce_ndvi', 'L3B LAI processor will produce NDVI', 'int', false, 4) on conflict DO nothing;
                INSERT INTO config_metadata VALUES ('processor.l3b.filter.produce_lai', 'L3B LAI processor will produce LAI', 'int', false, 4) on conflict DO nothing;
                INSERT INTO config_metadata VALUES ('processor.l3b.filter.produce_fapar', 'L3B LAI processor will produce FAPAR', 'int', false, 4) on conflict DO nothing;
                INSERT INTO config_metadata VALUES ('processor.l3b.filter.produce_fcover', 'L3B LAI processor will produce FCOVER', 'int', false, 4) on conflict DO nothing;
                INSERT INTO config_metadata VALUES ('processor.l3b.filter.produce_in_domain_flags', 'L3B processor will input domain flags', 'int', false, 4) on conflict DO nothing;
                
                INSERT INTO config_metadata VALUES ('executor.processor.l3b.slurm_qos', 'Slurm QOS for LAI processor', 'string', true, 8) on conflict DO nothing;
                INSERT INTO config_metadata VALUES ('general.scratch-path.l3b', 'Path for L3B temporary files', 'string', false, 1) on conflict DO nothing;

                DELETE FROM config WHERE key = 'executor.module.path.lai-end-of-job';
                DELETE FROM config_metadata WHERE key = 'executor.module.path.lai-end-of-job';

                INSERT INTO config(key, site_id, value, last_updated) VALUES ('executor.module.path.end-of-job', NULL, '/usr/bin/true', '2016-01-12 14:56:57.501918+02') on conflict DO nothing;
                INSERT INTO config_metadata VALUES ('executor.module.path.end-of-job', 'End of a multi root steps job', 'file', true, 8) on conflict DO nothing;
            
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
                
           _statement := 'update meta set version = ''1.3'';';
            raise notice '%', _statement;
            execute _statement;
        end if;
    end if;

    raise notice 'complete';
end;
$migration$;

commit;

