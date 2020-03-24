CREATE SCHEMA IF NOT EXISTS reports;

CREATE TABLE IF NOT EXISTS reports.s1_report
(
  downloader_history_id integer,
  orbit_id smallint,
  acquisition_date date,
  product_name character varying,
  status_description character varying,
  intersection_date date,
  intersected_status_id smallint,
  intersection numeric(5,2),
  polarisation character varying,
  l2_product character varying,
  l2_coverage numeric(5,2),
  status_reason character varying,
  intersected_product character varying,
  site_id smallint
)
WITH (
  OIDS=FALSE
);
ALTER TABLE reports.s1_report
  OWNER TO postgres;

CREATE TABLE IF NOT EXISTS reports.s2_report
(
  site_id smallint,
  downloader_history_id integer,
  orbit_id smallint,
  acquisition_date date,
  product_name character varying,
  status_description character varying,
  status_reason character varying,
  l2_product character varying,
  clouds integer
)
WITH (
  OIDS=FALSE
);
ALTER TABLE reports.s2_report
  OWNER TO postgres;

CREATE TABLE IF NOT EXISTS reports.l8_report
(
  site_id smallint,
  downloader_history_id integer,
  orbit_id character varying,
  acquisition_date date,
  product_name character varying,
  status_description character varying,
  status_reason character varying,
  l2_product character varying,
  clouds integer
)
WITH (
  OIDS=FALSE
);
ALTER TABLE reports.l8_report
  OWNER TO postgres;
