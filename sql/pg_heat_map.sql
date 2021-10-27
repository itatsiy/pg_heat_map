CREATE EXTENSION pg_heat_map;
SELECT build_heat_map(ARRAY [point(1, 2), point(2, 3)], 1);
