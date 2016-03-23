CREATE OR REPLACE FUNCTION sp_get_products_area(VARIADIC products character varying[])
  RETURNS double precision AS
$BODY$DECLARE surf double precision;
BEGIN
	surf := 0;
	IF $1 IS NOT NULL THEN
		FOR i IN 1 .. array_upper($1, 1)
		LOOP
			surf := surf + st_area(geog) FROM product WHERE name = $1[i];
		END LOOP;
	END IF;
	RETURN surf;
END$BODY$
  LANGUAGE plpgsql VOLATILE
