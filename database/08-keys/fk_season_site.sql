alter table season add constraint fk_season_site foreign key(site_id) references site(id);
