INSERT INTO site(name, short_name, geog)
VALUES ('Ukraine', 'ukraine', ST_GeographyFromText('POLYGON((21.500 53.000, 41.000 53.000, 41.000 43.600, 21.500 43.600, 21.500 53.000))'));

INSERT INTO site(name, short_name, geog)
VALUES ('Mali', 'mali', ST_GeographyFromText('POLYGON((-13.000 17.400, 5.000 17.400, 5.000 9.600, -13.000 9.600, -13.000 17.400))'));

INSERT INTO site(name, short_name, geog)
VALUES ('South Africa', 'south_africa', ST_GeographyFromText('POLYGON((15.690 -21.500,  33.500 -21.500, 33.500 -35.000, 15.690 -35.000, 15.690 -21.500))'));

INSERT INTO site(name, short_name, geog)
VALUES ('Madagascar (Antsirabe)', 'madagascar_antisrabe', ST_GeographyFromText('POLYGON((46.899388 -18.969148, 47.959467 -18.969148, 47.959467 -19.974510, 46.899388 -19.974510, 46.899388 -18.969148))'));

INSERT INTO site(name, short_name, geog)
VALUES ('Russia (Tula)', 'russia_tula', ST_GeographyFromText('POLYGON((35.940406 54.140187, 37.651121 54.140187, 37.651121 53.123645, 35.940406 53.123645, 35.940406 54.140187))'));

INSERT INTO site(name, short_name, geog)
VALUES ('Morocco (Tensift)', 'morocco_tensift', ST_GeographyFromText('POLYGON((-7.946524 32.532847, -6.766447 32.532847, -6.766447 31.527674, -7.946524 31.527674, -7.946524 32.532847))'));

INSERT INTO site(name, short_name, geog)
VALUES ('China (Shandong)', 'china_shandong', ST_GeographyFromText('POLYGON((115.875065 37.046532, 117.109759 37.046532, 117.109759 36.051579, 115.875065 36.051579, 115.875065 37.046532))'));
