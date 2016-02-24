CREATE OR REPLACE FUNCTION sp_get_dashboard_data(
IN _since_date date DEFAULT null
) RETURNS json AS $$
BEGIN

	RETURN '{"menu": {
		  "id": "file",
		  "value": "File",
		  "popup": {
		    "menuitem": [
		      {"value": "New", "onclick": "CreateNewDoc()"},
		      {"value": "Open", "onclick": "OpenDoc()"},
		      {"value": "Close", "onclick": "CloseDoc()"}
		    ]
		  }
		}}';

END;
$$ LANGUAGE plpgsql;

