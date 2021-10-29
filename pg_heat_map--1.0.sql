CREATE FUNCTION int4_heat_map_agg_func(INT[], FLOAT, FLOAT, BOX, INT, INT)
    RETURNS INT[]
AS
'MODULE_PATHNAME'
    LANGUAGE C;

CREATE OR REPLACE FUNCTION int4_heat_map_agg_finalize(arg INT[])
    RETURNS INTEGER[] AS
$$
BEGIN
    RETURN arg;
END;
$$ LANGUAGE plpgsql;

CREATE
    AGGREGATE int4_heat_map_agg (FLOAT8, FLOAT8, BOX, INT, INT)
    (
    SFUNC = int4_heat_map_agg_func,
    STYPE = INT[],
    FINALFUNC = int4_heat_map_agg_finalize
    );

CREATE FUNCTION bitset_heat_map_agg_func(BIGINT[], FLOAT, FLOAT, BOX, INT, INT)
    RETURNS BIGINT[]
AS
'MODULE_PATHNAME'
    LANGUAGE C;

CREATE OR REPLACE FUNCTION bitset_heat_map_agg_finalize(arg BIGINT[])
    RETURNS BIGINT[] AS
$$
BEGIN
    RETURN arg;
END;
$$ LANGUAGE plpgsql;

CREATE
    AGGREGATE bitset_heat_map_agg (FLOAT8, FLOAT8, BOX, INT, INT)
    (
    SFUNC = bitset_heat_map_agg_func,
    STYPE = BIGINT[],
    FINALFUNC = bitset_heat_map_agg_finalize
    );