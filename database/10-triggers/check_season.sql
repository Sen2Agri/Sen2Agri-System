CREATE TRIGGER check_season_dates 
	BEFORE INSERT OR UPDATE ON public.season
	FOR EACH ROW EXECUTE PROCEDURE public.check_season();
