MODULES = pg_heat_map

EXTENSION = pg_heat_map
DATA = pg_heat_map--1.0.sql
PGFILEDESC = "pg_heat_map - heat map builder"

REGRESS = pg_heat_map

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
