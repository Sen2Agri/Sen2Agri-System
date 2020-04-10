alter table product_provenance add constraint fk_product_provenance_product_id foreign key(product_id) references product(id) on delete cascade;
alter table product_provenance add constraint fk_product_provenance_parent_product_id foreign key(parent_product_id) references product(id) on delete cascade;
