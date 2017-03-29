create table season(
    id smallserial not null primary key,
    site_id smallserial not null,
    name text not null,
    start_date date not null,
    end_date date not null,
    mid_date date not null,
    enabled boolean not null,
    unique (site_id, name)
);
