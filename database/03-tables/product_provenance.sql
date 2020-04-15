create table if not exists product_provenance(
    product_id int not null,
    parent_product_id int not null,
    parent_product_date timestamp with time zone not null,
    constraint product_provenance_pkey primary key (product_id, parent_product_id)
);
