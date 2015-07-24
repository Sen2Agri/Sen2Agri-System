CREATE OR REPLACE FUNCTION sp_get_dashboard_system_overview() 
RETURNS json AS $$
BEGIN

	RETURN '{
		"current_jobs": [{
			"id": 7894,
			"processor": "L2A Atmospheric Corrections",
			"site": "Argentina, San Antonio de Areco",
			"triggered_by": "Available Product",
			"triggered_on": "2015-07-13 11:47:58",
			"status": "Running",
			"tasks_completed": 0,
			"tasks_remaining": 4,
			"current_tasks": [{
				"current_task_module": "Test module 3",
				"current_task_steps_completed": 17,
				"current_task_steps_remaining": 144
				},{
				"current_task_module": "Test module 3",
				"current_task_steps_completed": 17,
				"current_task_steps_remaining": 144}
				,{
				"current_task_module": "Test module 3",
				"current_task_steps_completed": 17,
				"current_task_steps_remaining": 144}],
			"actions": [1, 3, 4]
		}, {
			"id": 96345,
			"processor": "L3B Vegetation Status",
			"site": "Madagascar, Antsirabe",
			"triggered_by": "Scheduler",
			"triggered_on": "2015-07-13 12:23:45",
			"status": "Needs Input",
			"tasks_completed": 5,
			"tasks_remaining": 15,
			"current_tasks": [{
				"current_task_module": "Test module with long name",
				"current_task_steps_completed": 0,
				"current_task_steps_remaining": 1
				},{
				"current_task_module": "Test module with long name",
				"current_task_steps_completed": 0,
				"current_task_steps_remaining": 1}
				,{
				"current_task_module": "Test module with long name",
				"current_task_steps_completed": 0,
				"current_task_steps_remaining": 1}],
			"actions": [2, 3, 4]
		}, {
			"id": 96345,
			"processor": "L3B Vegetation Status",
			"site": "Madagascar, Antsirabe",
			"triggered_by": "Scheduler",
			"triggered_on": "2015-07-13 12:23:45",
			"status": "Needs Input",
			"tasks_completed": 5,
			"tasks_remaining": 15,
			"current_tasks": [{
				"current_task_module": "Test module with long name",
				"current_task_steps_completed": 0,
				"current_task_steps_remaining": 1
				},{
				"current_task_module": "Test module with long name",
				"current_task_steps_completed": 0,
				"current_task_steps_remaining": 1}
				,{
				"current_task_module": "Test module with long name",
				"current_task_steps_completed": 0,
				"current_task_steps_remaining": 1}],
			"actions": [2, 3, 4]
		}, {
			"id": 96345,
			"processor": "L3B Vegetation Status",
			"site": "Madagascar, Antsirabe",
			"triggered_by": "Scheduler",
			"triggered_on": "2015-07-13 12:23:45",
			"status": "Needs Input",
			"tasks_completed": 5,
			"tasks_remaining": 15,
			"current_tasks": [{
				"current_task_module": "Test module with long name",
				"current_task_steps_completed": 0,
				"current_task_steps_remaining": 1
				},{
				"current_task_module": "Test module with long name",
				"current_task_steps_completed": 0,
				"current_task_steps_remaining": 1}
				,{
				"current_task_module": "Test module with long name",
				"current_task_steps_completed": 0,
				"current_task_steps_remaining": 1}],
			"actions": [2, 3, 4]
		}],
		"server_resources" : [{
			"name": "Node 1",
			"cpu_now": 78,
			"cpu_history": [[' || extract(epoch from '2015-07-16 14:29:30'::timestamp) || ',22], [' || extract(epoch from '2015-07-16 14:29:35'::timestamp) || ',43], [' || extract(epoch from '2015-07-16 14:29:40'::timestamp) || ',27], [' || extract(epoch from '2015-07-16 14:29:45'::timestamp) || ',56], [' || extract(epoch from '2015-07-16 14:29:50'::timestamp) || ',28]],
			"ram_now": 2.34,
			"ram_available": 4,
			"ram_history": [[' || extract(epoch from '2015-07-16 14:29:30'::timestamp) || ',2.02], [' || extract(epoch from '2015-07-16 14:29:35'::timestamp) || ',1.56], [' || extract(epoch from '2015-07-16 14:29:40'::timestamp) || ',3.98], [' || extract(epoch from '2015-07-16 14:29:45'::timestamp) || ',1.19], [' || extract(epoch from '2015-07-16 14:29:50'::timestamp) || ',2.32]],
			"swap_now": 0.72,
			"swap_available": 16,
			"swap_history": [[' || extract(epoch from '2015-07-16 14:29:30'::timestamp) || ',1.02], [' || extract(epoch from '2015-07-16 14:29:35'::timestamp) || ',4.56], [' || extract(epoch from '2015-07-16 14:29:40'::timestamp) || ',2.98], [' || extract(epoch from '2015-07-16 14:29:45'::timestamp) || ',6.19], [' || extract(epoch from '2015-07-16 14:29:50'::timestamp) || ',4.32]],
			"disk_used": 1.67,
			"disk_available": 3,
			"load_1min": 2.23,
			"load_5min": 1.34,
			"load_15min": 2.42,
			"load_1min_history": [[' || extract(epoch from '2015-07-16 14:29:30'::timestamp) || ',1.23], [' || extract(epoch from '2015-07-16 14:29:35'::timestamp) || ',5.67], [' || extract(epoch from '2015-07-16 14:29:40'::timestamp) || ',2.34], [' || extract(epoch from '2015-07-16 14:29:45'::timestamp) || ',0.14], [' || extract(epoch from '2015-07-16 14:29:50'::timestamp) || ',2.45]],
			"load_5min_history": [[' || extract(epoch from '2015-07-16 14:29:30'::timestamp) || ',4.23], [' || extract(epoch from '2015-07-16 14:29:35'::timestamp) || ',1.67], [' || extract(epoch from '2015-07-16 14:29:40'::timestamp) || ',5.34], [' || extract(epoch from '2015-07-16 14:29:45'::timestamp) || ',0.94], [' || extract(epoch from '2015-07-16 14:29:50'::timestamp) || ',4.45]],
			"load_15min_history": [[' || extract(epoch from '2015-07-16 14:29:30'::timestamp) || ',2.23], [' || extract(epoch from '2015-07-16 14:29:35'::timestamp) || ',3.67], [' || extract(epoch from '2015-07-16 14:29:40'::timestamp) || ',1.34], [' || extract(epoch from '2015-07-16 14:29:45'::timestamp) || ',0.84], [' || extract(epoch from '2015-07-16 14:29:50'::timestamp) || ',3.45]]
		}, {
			"name": "Node 2",
			"cpu_now": 23,
			"cpu_history": [[' || extract(epoch from '2015-07-16 14:29:30'::timestamp) || ',22], [' || extract(epoch from '2015-07-16 14:29:35'::timestamp) || ',43], [' || extract(epoch from '2015-07-16 14:29:40'::timestamp) || ',27], [' || extract(epoch from '2015-07-16 14:29:45'::timestamp) || ',56], [' || extract(epoch from '2015-07-16 14:29:50'::timestamp) || ',98]],
			"ram_now": 2.34,
			"ram_available": 4,
			"ram_history": [[' || extract(epoch from '2015-07-16 14:29:30'::timestamp) || ',2.02], [' || extract(epoch from '2015-07-16 14:29:35'::timestamp) || ',1.56], [' || extract(epoch from '2015-07-16 14:29:40'::timestamp) || ',3.98], [' || extract(epoch from '2015-07-16 14:29:45'::timestamp) || ',1.19], [' || extract(epoch from '2015-07-16 14:29:50'::timestamp) || ',2.32]],
			"swap_now": 0.72,
			"swap_available": 16,
			"swap_history": [[' || extract(epoch from '2015-07-16 14:29:30'::timestamp) || ',1.02], [' || extract(epoch from '2015-07-16 14:29:35'::timestamp) || ',4.56], [' || extract(epoch from '2015-07-16 14:29:40'::timestamp) || ',2.98], [' || extract(epoch from '2015-07-16 14:29:45'::timestamp) || ',6.19], [' || extract(epoch from '2015-07-16 14:29:50'::timestamp) || ',4.32]],
			"disk_used": 1.67,
			"disk_available": 3,
			"load_1min": 2.23,
			"load_5min": 1.34,
			"load_15min": 2.42,
			"load_1min_history": [[' || extract(epoch from '2015-07-16 14:29:30'::timestamp) || ',1.23], [' || extract(epoch from '2015-07-16 14:29:35'::timestamp) || ',5.67], [' || extract(epoch from '2015-07-16 14:29:40'::timestamp) || ',2.34], [' || extract(epoch from '2015-07-16 14:29:45'::timestamp) || ',0.14], [' || extract(epoch from '2015-07-16 14:29:50'::timestamp) || ',2.45]],
			"load_5min_history": [[' || extract(epoch from '2015-07-16 14:29:30'::timestamp) || ',4.23], [' || extract(epoch from '2015-07-16 14:29:35'::timestamp) || ',1.67], [' || extract(epoch from '2015-07-16 14:29:40'::timestamp) || ',5.34], [' || extract(epoch from '2015-07-16 14:29:45'::timestamp) || ',0.94], [' || extract(epoch from '2015-07-16 14:29:50'::timestamp) || ',4.45]],
			"load_15min_history": [[' || extract(epoch from '2015-07-16 14:29:30'::timestamp) || ',2.23], [' || extract(epoch from '2015-07-16 14:29:35'::timestamp) || ',3.67], [' || extract(epoch from '2015-07-16 14:29:40'::timestamp) || ',1.34], [' || extract(epoch from '2015-07-16 14:29:45'::timestamp) || ',0.84], [' || extract(epoch from '2015-07-16 14:29:50'::timestamp) || ',3.45]]
		}]
		}';

END;
$$ LANGUAGE plpgsql;

