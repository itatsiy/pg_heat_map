#! /bin/bash
psql -c 'drop extension if exists pg_heat_map'
make
make install
make installcheck
psql -c 'create extension pg_heat_map'
psql -c 'select build_heat_map(ARRAY [point(1, 2), point(2, 3)], 0.1)'