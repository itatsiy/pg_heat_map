CREATE OR REPLACE FUNCTION int4_heat_map_agg_func(INT[], FLOAT, FLOAT, BOX, INT, INT)
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

CREATE OR REPLACE FUNCTION bitset_heat_map_agg_func(BIGINT[], FLOAT, FLOAT, BOX, INT, INT)
    RETURNS BIGINT[]
AS
'MODULE_PATHNAME'
    LANGUAGE C;

CREATE OR REPLACE FUNCTION bitset_heat_map_agg_combinefunc(a BIGINT[], b BIGINT[])
    RETURNS BIGINT[] AS
$$
DECLARE
    i INTEGER;
BEGIN
    IF a IS NULL THEN
        RETURN b;
    END IF;
    FOR i IN 1 .. ARRAY_UPPER(a, 1)
        LOOP
            a[i] = a[i] | b[i];
        END LOOP;
    RETURN a;
END;
$$ LANGUAGE plpgsql;

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
    COMBINEFUNC = bitset_heat_map_agg_combinefunc,
    PARALLEL = SAFE,
    FINALFUNC = bitset_heat_map_agg_finalize
    );