create or replace function sp_insert_default_scheduled_tasks(
    _site_id site.id%type,
    _season_id season.id%type
)
returns void as
$$
declare _site_name site.short_name%type;
declare _season_name season.name%type;
declare _start_date season.start_date%type;
declare _mid_date season.start_date%type;
begin
    select short_name
    into _site_name
    from site
    where id = _site_id;

    select name,
           start_date,
           mid_date
    into _season_name,
         _start_date,
         _mid_date
    from season
    where id = _season_id;

	perform sp_insert_scheduled_task(
                _site_name || '_' || _season_name || '_L3A' :: character varying,
                2,
                _site_id :: int,
                _season_id :: int,
                2::smallint,
                0::smallint,
                31::smallint,
                cast((select date_trunc('month', _start_date) + interval '1 month' - interval '1 day') as character varying),
                60,
                1 :: smallint,
                '{}' :: json);

	perform sp_insert_scheduled_task(
                _site_name || '_' || _season_name || '_L3B' :: character varying,
                3,
                _site_id :: int,
                _season_id :: int,
                1::smallint,
                10::smallint,
                0::smallint,
                cast((_start_date + 10) as character varying),
                60,
                1 :: smallint,
                '{"general_params":{"product_type":"L3B"}}' :: json);

	perform sp_insert_scheduled_task(
                _site_name || '_' || _season_name || '_L4A' :: character varying,
                5,
                _site_id :: int,
                _season_id :: int,
                2::smallint,
                0::smallint,
                31::smallint,
                cast(_mid_date as character varying),
                60,
                1 :: smallint,
                '{}' :: json);

	perform sp_insert_scheduled_task(
                _site_name || '_' || _season_name || '_L4B' :: character varying,
                6,
                _site_id :: int,
                _season_id :: int,
                2::smallint,
                0::smallint,
                31::smallint,
                cast(_mid_date as character varying),
                60,
                1 :: smallint,
                '{}' :: json);
end;
$$
    language plpgsql volatile;
