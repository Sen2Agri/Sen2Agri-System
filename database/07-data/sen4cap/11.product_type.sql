INSERT INTO product_type
(id, name, description, is_raster)
VALUES
(7, 'l1c','L1C product', true);

INSERT INTO product_type
(id, name, description, is_raster)
VALUES
(1, 'l2a','L2A Atmospheric correction', true);

INSERT INTO product_type
(id, name, description, is_raster)
VALUES
(3, 'l3b_lai_monodate','L3A LAI mono-date product', true);

INSERT INTO product_type
(id, name, description, is_raster)
VALUES
(4, 's4c_l4a','L4A Crop type product', false);

INSERT INTO product_type
(id, name, description, is_raster)
VALUES
(5, 's4c_l4b','L4B Grassland Mowing product', false);

INSERT INTO product_type
(id, name, description, is_raster)
VALUES
(6, 's4c_l4c','L4C Agricultural Practices product', false);

INSERT INTO product_type
(id, name, description, is_raster)
VALUES
(8, 'l3c_lai_reproc','L3C LAI Reprocessed product', true);

INSERT INTO product_type
(id, name, description, is_raster)
VALUES
(10, 's1_l2a_amp','Sentinel 1 L2 Amplitude product', true);

INSERT INTO product_type
(id, name, description, is_raster)
VALUES
(11, 's1_l2a_cohe','Sentinel 1 L2 Coherence product', true);


INSERT INTO product_type
(id, name, description)
VALUES
(14, 'lpis', 'LPIS raster product');
