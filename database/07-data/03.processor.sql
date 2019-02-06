INSERT INTO processor
(id, name, short_name, label)
VALUES
(1, 'L2A Atmospheric Corrections','l2a');

INSERT INTO processor
(id, name, short_name, label)
VALUES
(2, 'L3A Composite','l3a', 'L3A &mdash; Cloud-free Composite');

INSERT INTO processor
(id, name, short_name, label)
VALUES
(3, 'L3B Vegetation Status','l3b_lai', 'L3B &mdash; LAI/NDVI');

INSERT INTO processor
(id, name, short_name, label)
VALUES
(4, 'L3E Pheno NDVI metrics','l3e_pheno', 'L3E &mdash; Phenology Indices');

INSERT INTO processor
(id, name, short_name, label)
VALUES
(5, 'L4A Crop Mask','l4a', 'L4A &mdash; Cropland Mask');

INSERT INTO processor
(id, name, short_name, label)
VALUES
(6, 'L4B Crop Type','l4b', 'L4B &mdash; Crop Type Map');

INSERT INTO processor
(id, name, short_name)
VALUES
(7, 'L2 SAR Amplitude','l2-amp');

INSERT INTO processor
(id, name, short_name)
VALUES
(8, 'L2 SAR Coherence','l2-cohe');

INSERT INTO processor
(id, name, short_name, label)
VALUES
(9, 'S4C L4C Agricultural Practices','s4c_l4c', 'S4C L4C &mdash; Agricultural Practices');
