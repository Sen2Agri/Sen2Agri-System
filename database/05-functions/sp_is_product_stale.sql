create or replace function sp_is_product_stale(
    _product_id int,
    _parent_products int[]
)
returns boolean
as $$
begin
    return exists (
        select pid
        from unnest(_parent_products) as pp(pid)
        where not exists (
            select *
            from product_provenance
            where (product_id, parent_product_id) = (_product_id, pid)
        )
    ) or exists (
        select *
        from product_provenance
        where exists (
            select *
            from product
            where product.id = product_provenance.parent_product_id
              and product.created_timestamp >= product_provenance.parent_product_date
        )
    );
end;
$$
language plpgsql stable;
