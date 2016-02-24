CREATE OR REPLACE FUNCTION sp_get_processors()
RETURNS TABLE (
    id processor.id%TYPE,
    "short_name" processor."short_name"%TYPE,
    "name" processor."name"%TYPE
)
AS $$
BEGIN
    RETURN QUERY
        SELECT processor.id,
               processor.short_name,
               processor.name
        FROM processor
        ORDER BY processor.id;
END
$$
LANGUAGE plpgsql
STABLE;
