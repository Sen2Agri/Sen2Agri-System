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
