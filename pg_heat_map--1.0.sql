CREATE FUNCTION build_heat_map(POINT[])
    RETURNS POINT
    AS 'MODULE_PATHNAME'
    LANGUAGE C;

