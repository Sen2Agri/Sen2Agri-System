create table product_details_l4a(
    product_id int not null references product(id) on delete cascade,
    "NewID" int not null,
    "CT_decl" int,
    "CT_pred_1" int,
    "CT_conf_1" real,
    "CT_pred_2" int,
    "CT_conf_2" real,
    constraint product_details_l4a_pkey primary key(product_id, "NewID")
);
