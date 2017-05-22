CREATE OR REPLACE FUNCTION sp_dashboard_update_site(
    _id smallint,
    _enabled boolean)
  RETURNS void AS
$BODY$
BEGIN

IF _enabled IS NOT NULL THEN
    UPDATE site
    SET enabled = _enabled AND EXISTS(
                    SELECT *
                    FROM season
                    WHERE season.site_id = _id AND season.enabled)
    WHERE id = _id;
END IF;

END;
$BODY$
  LANGUAGE plpgsql;
