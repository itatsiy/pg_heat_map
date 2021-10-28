CREATE FUNCTION heat_map_agg_func(INT [], FLOAT, FLOAT, BOX, INT, INT)
    RETURNS INT[]
    AS 'MODULE_PATHNAME'
    LANGUAGE C;

CREATE OR REPLACE FUNCTION heat_map_agg_finalize(arg INT [])
    RETURNS INTEGER[] AS $$
BEGIN
    RETURN arg;
END;
$$ LANGUAGE plpgsql;

CREATE
AGGREGATE heat_map_agg (FLOAT8, FLOAT8, BOX, INT, INT)
(
    SFUNC = heat_map_agg_func,
    STYPE = INT[],
    FINALFUNC = heat_map_agg_finalize
);