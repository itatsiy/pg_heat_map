#! /bin/bash
psql -c 'drop extension if exists pg_heat_map'
make
make install
make installcheck
psql -c 'create extension pg_heat_map'
psql -c 'SELECT heat_map_agg(point[0], point[1], box(point(37, 158), point(70, 19)), 10, 5) FROM points'