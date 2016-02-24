CREATE OR REPLACE FUNCTION sp_get_dashboard_server_resource_data() 
RETURNS json AS $$
DECLARE current_node RECORD;
DECLARE temp_json json;
DECLARE temp_json2 json;
DECLARE cpu_user_history_json json;
DECLARE cpu_system_history_json json;
DECLARE ram_history_json json;
DECLARE swap_history_json json;
DECLARE load_1min_history_json json;
DECLARE load_5min_history_json json;
DECLARE load_15min_history_json json;

DECLARE since timestamp;
BEGIN

	CREATE TEMP TABLE current_nodes (
		name character varying,
		cpu_user_now smallint,
		cpu_user_history json,
		cpu_system_now smallint,
		cpu_system_history json,
		ram_now real,
		ram_available real,
		ram_unit character varying,
		ram_history json,
		swap_now real,
		swap_available real,
		swap_unit character varying,
		swap_history json,
		disk_used real,
		disk_available real,
		disk_unit character varying,
		load_1min real,
		load_5min real,
		load_15min real,
		load_1min_history json,
		load_5min_history json,
		load_15min_history json
		) ON COMMIT DROP;

	-- Get the list of nodes to return the resources for
	INSERT INTO current_nodes (name)
	SELECT DISTINCT	node_name
	FROM node_resource_log ORDER BY node_resource_log.node_name;

	-- Ensure that default values are set for some of the fields
	UPDATE current_nodes
	SET 
		cpu_user_now = 0,
		cpu_system_now = 0,
		ram_now = 0,
		ram_available = 0,
		ram_unit = 'GB',
		swap_now = 0,
		swap_available = 0,
		swap_unit = 'GB',
		disk_used = 0,
		disk_available = 0,
		disk_unit = 'GB',
		load_1min = 0,
		load_5min = 0,
		load_15min = 0;

	-- Go through the nodes and compute their data
	FOR current_node IN SELECT * FROM current_nodes ORDER BY name LOOP

		-- First, get the NOW data
		UPDATE current_nodes
		SET 
			cpu_user_now = coalesce(current_node_now.cpu_user,0) / 10,
			cpu_system_now = coalesce(current_node_now.cpu_system,0) / 10,
			ram_now = round(coalesce(current_node_now.mem_used_kb,0)::numeric / 1048576::numeric, 2),	-- Convert to GB
			ram_available = round(coalesce(current_node_now.mem_total_kb,0)::numeric / 1048576::numeric, 2),	-- Convert to GB
			ram_unit = 'GB',
			swap_now = round(coalesce(current_node_now.swap_used_kb,0)::numeric / 1048576::numeric, 2),	-- Convert to GB
			swap_available = round(coalesce(current_node_now.swap_total_kb,0)::numeric / 1048576::numeric, 2),	-- Convert to GB
			swap_unit = 'GB',
			disk_used = round(coalesce(current_node_now.disk_used_bytes,0)::numeric / 1073741824::numeric, 2),	-- Convert to GB
			disk_available = round(coalesce(current_node_now.disk_total_bytes,0)::numeric / 1073741824::numeric, 2),	-- Convert to GB
			disk_unit = 'GB',
			load_1min = coalesce(current_node_now.load_avg_1m,0),
			load_5min = coalesce(current_node_now.load_avg_5m,0),
			load_15min = coalesce(current_node_now.load_avg_15m,0)
		FROM (SELECT * FROM node_resource_log WHERE node_resource_log.node_name = current_node.name
		AND timestamp >= now() - '1 minute'::interval
		ORDER BY timestamp DESC LIMIT 1) AS current_node_now
		WHERE current_nodes.name = current_node.name;

		-- The history will be shown since:
		since := now() - '15 minutes'::interval;
		
		-- Next, get the HISTORY data
		SELECT  
			array_to_json(array_agg( json_build_array(extract(epoch from resource_history.timestamp)::bigint * 1000, resource_history.cpu_user / 10))),
			array_to_json(array_agg( json_build_array(extract(epoch from resource_history.timestamp)::bigint * 1000, resource_history.cpu_system / 10))),
			array_to_json(array_agg( json_build_array(extract(epoch from resource_history.timestamp)::bigint * 1000, round(resource_history.mem_used_kb::numeric / 1048576::numeric, 2)))),	-- Convert to GB
			array_to_json(array_agg( json_build_array(extract(epoch from resource_history.timestamp)::bigint * 1000, round(resource_history.swap_used_kb::numeric / 1048576::numeric, 2)))),	-- Convert to GB
			array_to_json(array_agg( json_build_array(extract(epoch from resource_history.timestamp)::bigint * 1000, resource_history.load_avg_1m))),
			array_to_json(array_agg( json_build_array(extract(epoch from resource_history.timestamp)::bigint * 1000, resource_history.load_avg_5m))),
			array_to_json(array_agg( json_build_array(extract(epoch from resource_history.timestamp)::bigint * 1000, resource_history.load_avg_15m))) 
		INTO
			cpu_user_history_json,
			cpu_system_history_json,
			ram_history_json,
			swap_history_json,
			load_1min_history_json,
			load_5min_history_json,
			load_15min_history_json
		FROM (
			SELECT 
			timestamp,
			cpu_user,
			cpu_system,
			mem_used_kb,
			swap_used_kb,
			load_avg_1m,
			load_avg_5m,
			load_avg_15m
			FROM node_resource_log 
			WHERE node_resource_log.node_name = current_node.name 
			AND node_resource_log.timestamp >= since
			ORDER BY timestamp DESC) resource_history;

		-- Make sure that there are enough entries in the arrays so that the graph is shown as coming from right to left in the first 15 minutes
		cpu_user_history_json := sp_pad_left_json_history_array(cpu_user_history_json, since, '1 minute');
		cpu_system_history_json := sp_pad_left_json_history_array(cpu_system_history_json, since, '1 minute');
		ram_history_json := sp_pad_left_json_history_array(ram_history_json, since, '1 minute');
		swap_history_json := sp_pad_left_json_history_array(swap_history_json, since, '1 minute');
		load_1min_history_json := sp_pad_left_json_history_array(load_1min_history_json, since, '1 minute');
		load_5min_history_json := sp_pad_left_json_history_array(load_5min_history_json, since, '1 minute');
		load_15min_history_json := sp_pad_left_json_history_array(load_15min_history_json, since, '1 minute');

		-- Make sure that there are entries added in the arrays even if there isn't data up to now
		cpu_user_history_json := sp_pad_right_json_history_array(cpu_user_history_json, since, '1 minute');
		cpu_system_history_json := sp_pad_right_json_history_array(cpu_system_history_json, since, '1 minute');
		ram_history_json := sp_pad_right_json_history_array(ram_history_json, since, '1 minute');
		swap_history_json := sp_pad_right_json_history_array(swap_history_json, since, '1 minute');
		load_1min_history_json := sp_pad_right_json_history_array(load_1min_history_json, since, '1 minute');
		load_5min_history_json := sp_pad_right_json_history_array(load_5min_history_json, since, '1 minute');
		load_15min_history_json := sp_pad_right_json_history_array(load_15min_history_json, since, '1 minute');

		UPDATE current_nodes
		SET 
			cpu_user_history = cpu_user_history_json,
			cpu_system_history = cpu_system_history_json,
			ram_history = ram_history_json,
			swap_history = swap_history_json,
			load_1min_history = load_1min_history_json,
			load_5min_history = load_5min_history_json,
			load_15min_history = load_15min_history_json
		WHERE current_nodes.name = current_node.name;
		
	END LOOP;

	SELECT array_to_json(array_agg(row_to_json(current_nodes_details, true))) INTO temp_json
	FROM (SELECT * FROM current_nodes) AS current_nodes_details;

	temp_json2 := json_build_object('server_resources', temp_json);

	RETURN temp_json2;


END;
$$ LANGUAGE plpgsql;