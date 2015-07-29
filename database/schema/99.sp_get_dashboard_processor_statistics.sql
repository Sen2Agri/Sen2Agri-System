CREATE OR REPLACE FUNCTION sp_get_dashboard_processor_statistics() 
RETURNS json AS $$
DECLARE current_processor RECORD;
DECLARE temp_json json;
DECLARE temp_json2 json;
DECLARE temp_json3 json;
DECLARE temp_array json[];
DECLARE temp_array2 json[];
DECLARE return_string text;
BEGIN

	CREATE TEMP TABLE processors (
		id smallint,
		name character varying
		) ON COMMIT DROP;

	-- Get the list of processors to return the resources for
	INSERT INTO processors (id, name)
	SELECT id, short_name
	FROM processor ORDER BY name;

	return_string := '{';

	-- Go through the processors and compute their data
	FOR current_processor IN SELECT * FROM processors ORDER BY name LOOP

		IF return_string != '{' THEN
			return_string := return_string || ',';
		END IF;

		-- First compute the resource averages
		WITH job_resources AS(
		SELECT 
		max(entry_timestamp) AS last_run,
		sum(duration_ms) AS total_duration,
		sum(user_cpu_ms) AS total_user_cpu,
		sum(system_cpu_ms) AS total_system_cpu,
		sum(max_rss_kb) AS total_max_rss,
		sum(max_vm_size_kb) AS total_max_vm_size,
		sum(disk_read_b) AS total_disk_read,
		sum(disk_write_b) AS total_disk_write
		FROM step_resource_log 
		INNER JOIN task ON step_resource_log.task_id = task.id
		INNER JOIN job ON task.job_id = job.id AND job.processor_id = current_processor.id
		GROUP BY job.id)
		SELECT '[' ||
		'["Last Run On","' || to_char(max(last_run), 'YYYY-MM-DD HH:MI:SS') || '"],' ||
		'["Average Duration","' || to_char(avg(total_duration) / 1000 * INTERVAL '1 second', 'HH24:MI:SS.MS') || '"],' ||
		'["Average User CPU","' || to_char(avg(total_user_cpu) / 1000 * INTERVAL '1 second', 'HH24:MI:SS.MS') || '"],' ||
		'["Average System CPU","' || to_char(avg(total_system_cpu) / 1000 * INTERVAL '1 second', 'HH24:MI:SS.MS') || '"],' ||
		'["Average Max RSS","' || round(avg(total_max_rss)::numeric / 1024::numeric, 2)::varchar || ' MB' || '"],' ||
		'["Average Max VM","' || round(avg(total_max_vm_size)::numeric / 1024::numeric, 2)::varchar || ' MB' || '"],' ||
		'["Average Disk Read","' || round(avg(total_disk_read)::numeric / 1048576::numeric, 2)::varchar || ' MB' || '"],' ||
		'["Average Disk Write","' || round(avg(total_disk_write)::numeric / 1048576::numeric, 2)::varchar || ' MB' || '"]' ||
		']' INTO temp_json
		FROM job_resources;

		-- Next compute the output statistics
		temp_array := array[]::json[];
		SELECT json_build_array('Number of Products', count(*)) INTO temp_json2 FROM product WHERE processor_id = current_processor.id;
		temp_array := array_append(temp_array, temp_json2);

		WITH step_statistics AS(
		SELECT 
		count(*) AS no_of_tiles, 
		sum(duration_ms)/count(*) AS average_duration_per_tile
		FROM step_resource_log 
		INNER JOIN task ON step_resource_log.task_id = task.id
		INNER JOIN job ON task.job_id = job.id AND job.processor_id = current_processor.id
		GROUP BY job.id)
		SELECT array[json_build_array('Average Tiles per Product',round(avg(no_of_tiles),2)), json_build_array('Average Duration per Tile',round(avg(average_duration_per_tile),2))]
		INTO temp_array2
		FROM step_statistics;

		temp_array := array_cat(temp_array, temp_array2);
		temp_json2 := array_to_json(temp_array);

		-- Last get the configuration parameters
		WITH config_params AS (
		SELECT json_build_array(
		key || CASE coalesce(config.site_id,0) WHEN 0 THEN '' ELSE '(' || site.short_name || ')' END,
		value) AS param
		FROM config
		LEFT OUTER JOIN site ON config.site_id = site.id
		WHERE config.key ILIKE 'processor.' || current_processor.name)
		SELECT array_to_json(array_agg(config_params.param)) INTO temp_json3
		FROM config_params;

		-- Update the return json with the computed data
		return_string := return_string || '"' || current_processor.name || '_statistics" :' || json_build_object('resources', temp_json, 'output', temp_json2, 'configuration', temp_json3);
		
	END LOOP;

	return_string := return_string || '}';
	RETURN return_string::json;

END;
$$ LANGUAGE plpgsql;