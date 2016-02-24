CREATE INDEX ix_shape_tiles_s2_geog ON shape_tiles_s2 USING gist(geog);
