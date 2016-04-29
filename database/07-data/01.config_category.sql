INSERT INTO config_category VALUES (8, 'Executor', 7, false);
INSERT INTO config_category VALUES (12, 'Dashboard', 8, false);
INSERT INTO config_category VALUES (1, 'General', 0, true);
INSERT INTO config_category VALUES (2, 'L2A Atmospheric Corrections', 1, true);
INSERT INTO config_category VALUES (3, 'L3A Composite', 2, true);
INSERT INTO config_category VALUES (4, 'L3B Vegetation Status', 3, true);
INSERT INTO config_category VALUES (5, 'L4A Crop Mask', 4, true);
INSERT INTO config_category VALUES (6, 'L4B Crop Type', 5, true);
INSERT INTO config_category VALUES (7, 'Archiver', 6, true);
INSERT INTO config_category VALUES (13, 'Monitoring Agent', 9, false);
INSERT INTO config_category VALUES (14, 'Resources', 10, false);
INSERT INTO config_category VALUES (16, 'Demmaccs', 12, false);
INSERT INTO config_category VALUES (15, 'Downloader', 11, true);
INSERT INTO config_category VALUES (17, 'Site', 13, false);

SELECT pg_catalog.setval('config_category_id_seq', 17, true);
