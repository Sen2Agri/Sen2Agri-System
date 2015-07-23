CREATE OR REPLACE FUNCTION sp_get_dashboard_processor_statistics() 
RETURNS json AS $$
BEGIN

	RETURN '{
		"l2a_statistics" : {
			"resources": [ ["Last Run On", "2015-07-20 16:11:23"], ["Average Duration", "03:12:22"], ["Average User CPU", "03:10:25"], ["Average System CPU", "01:10:25"], ["Average Maximum RSS", "893 MB"], ["Average Maximum VM", "1245 MB"], ["Average Disk Read", "563 MB"], ["Average Disk Write", "1236 MB"]],
			"output": [["Number Of Products", 23], ["Average Tile per Product", 6], ["Average Processing per Tile", "00:32:15"]],
			"configuration": [["Param 1", "Value 1"], ["Param 2", "Value 2"]]
		},
		"l3a_statistics" : {
			"resources": [ ["Last Run On", "2015-07-20 16:11:23"], ["Average Duration", "03:12:22"], ["Average User CPU", "03:10:25"], ["Average System CPU", "01:10:25"], ["Average Maximum RSS", "893 MB"], ["Average Maximum VM", "1245 MB"], ["Average Disk Read", "563 MB"], ["Average Disk Write", "1236 MB"]],
			"output": [["Number Of Products", 23], ["Average Tile per Product", 6], ["Average Processing per Tile", "00:32:15"]],
			"configuration": [["Param 1", "Value 1"], ["Param 2", "Value 2"]]
		},
		"l3b_statistics" : {
			"resources": [ ["Last Run On", "2015-07-20 16:11:23"], ["Average Duration", "03:12:22"], ["Average User CPU", "03:10:25"], ["Average System CPU", "01:10:25"], ["Average Maximum RSS", "893 MB"], ["Average Maximum VM", "1245 MB"], ["Average Disk Read", "563 MB"], ["Average Disk Write", "1236 MB"]],
			"output": [["Number Of Products", 23], ["Average Tile per Product", 6], ["Average Processing per Tile", "00:32:15"]],
			"configuration": [["Param 1", "Value 1"], ["Param 2", "Value 2"]]
		},
		"l4a_statistics" : {
			"resources": [ ["Last Run On", "2015-07-20 16:11:23"], ["Average Duration", "03:12:22"], ["Average User CPU", "03:10:25"], ["Average System CPU", "01:10:25"], ["Average Maximum RSS", "893 MB"], ["Average Maximum VM", "1245 MB"], ["Average Disk Read", "563 MB"], ["Average Disk Write", "1236 MB"]],
			"output": [["Number Of Products", 23], ["Average Tile per Product", 6], ["Average Processing per Tile", "00:32:15"]],
			"configuration": [["Param 1", "Value 1"], ["Param 2", "Value 2"]]
		},
		"l4b_statistics" : {
			"resources": [ ["Last Run On", "2015-07-20 16:11:23"], ["Average Duration", "03:12:22"], ["Average User CPU", "03:10:25"], ["Average System CPU", "01:10:25"], ["Average Maximum RSS", "893 MB"], ["Average Maximum VM", "1245 MB"], ["Average Disk Read", "563 MB"], ["Average Disk Write", "1236 MB"]],
			"output": [["Number Of Products", 23], ["Average Tile per Product", 6], ["Average Processing per Tile", "00:32:15"]],
			"configuration": [["Param 1", "Value 1"], ["Param 2", "Value 2"]]
		}
		}';

END;
$$ LANGUAGE plpgsql;

