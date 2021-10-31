#! /bin/bash
psql -c 'drop extension if exists pg_heat_map'
make
make install
make installcheck
psql -c 'create extension pg_heat_map'
psql -c 'SELECT bitset_heat_map_agg(point[0], point[1], box(point(0, 0), point(200, 200)), 10, 10) FROM points'