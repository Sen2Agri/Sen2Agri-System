CREATE OR REPLACE FUNCTION sp_insert_node_statistics(
    _node_name node_resource_log.node_name%TYPE,
    _cpu_user node_resource_log.cpu_user%TYPE,
    _cpu_system node_resource_log.cpu_system%TYPE,
    _mem_total_kb node_resource_log.mem_total_kb%TYPE,
    _mem_used_kb node_resource_log.mem_used_kb%TYPE,
    _swap_total_kb node_resource_log.swap_total_kb%TYPE,
    _swap_used_kb node_resource_log.swap_used_kb%TYPE,
    _load_avg_1m node_resource_log.load_avg_1m%TYPE,
    _load_avg_5m node_resource_log.load_avg_5m%TYPE,
    _load_avg_15m node_resource_log.load_avg_15m%TYPE,
    _disk_total_bytes node_resource_log.disk_total_bytes%TYPE,
    _disk_used_bytes node_resource_log.disk_used_bytes%TYPE,
    _timestamp node_resource_log.timestamp%TYPE
)
RETURNS VOID
AS $$
BEGIN
    INSERT INTO node_resource_log(
        node_name,
        cpu_user,
        cpu_system,
        mem_total_kb,
        mem_used_kb,
        swap_total_kb,
        swap_used_kb,
        load_avg_1m,
        load_avg_5m,
        load_avg_15m,
        disk_total_bytes,
        disk_used_bytes,
        "timestamp"
    )
    VALUES (
        _node_name,
        _cpu_user,
        _cpu_system,
        _mem_total_kb,
        _mem_used_kb,
        _swap_total_kb,
        _swap_used_kb,
        _load_avg_1m,
        _load_avg_5m,
        _load_avg_15m,
        _disk_total_bytes,
        _disk_used_bytes,
        now()
    );
END;
$$
LANGUAGE plpgsql
VOLATILE;
