CREATE OR REPLACE FUNCTION sp_get_dashboard_product_availability(
IN _since_timestamp timestamp without time zone DEFAULT null) 
RETURNS json AS $$
BEGIN

	RETURN '{
		"products" : [{
			"key": "Products",
			"_values": [
			  {
				"key": "L2A Products",
				"_values": [
				  {
					"key": "Product 1",
					"date": gd(2015,07,16,14,29,30),
					"info": ["Info 1", "Info 2", "Info 3"]
				  },
				  {
					"key": "Product 2",
					"date": gd(2015,07,16,14,29,30),
					"info": ["Info 1", "Info 2", "Info 3"] 
				  }]
			  },
			  {
			   "key": "L3A Products",
				"_values": [
				  {
					"key": "Product 3",
					"date": gd(2015,07,16,14,29,30),
					"info": ["Info 1", "Info 2", "Info 3"]
				  },
				  {
					"key": "Product 4",
					"date": gd(2015,07,16,14,29,30),
					"info": ["Info 1", "Info 2", "Info 3"] 
				  }]
			  }]
		}]
		}';

END;
$$ LANGUAGE plpgsql;

