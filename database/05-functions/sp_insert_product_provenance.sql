create or replace function sp_insert_product_provenance(
    _product_id int,
    _parent_product_id int,
    _parent_product_date int
)
returns void
as $$
begin
    insert into product_provenance(product_id, parent_product_id, parent_product_date)
    values (_product_id, _parent_product_id, _parent_product_date);
end;
$$
language plpgsql volatile;
