alter table scheduled_task add constraint fk_scheduled_task_season foreign key(season_id) references season(id) on delete cascade;
