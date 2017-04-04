alter table scheduled_task_status add constraint fk_scheduled_task_status_scheduled_task foreign key(task_id) references scheduled_task(id) on delete cascade;
