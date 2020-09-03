create table fmask_history(
    satellite_id smallint not null references satellite(id),
    downloader_history_id int not null references downloader_history(id),
    status_id int not null references l1_tile_status(id),
    status_timestamp timestamp with time zone not null default now(),
    retry_count int not null default 0,
    failed_reason text,
    primary key (downloader_history_id)
);
