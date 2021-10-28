#include "postgres.h"

#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/geo_decls.h"
#include "utils/arrayaccess.h"
#include "utils/float.h"
#include "utils/lsyscache.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(heat_map_agg_func);

Datum
heat_map_agg_func(PG_FUNCTION_ARGS) {
    float8 x = PG_GETARG_FLOAT8(1);
    float8 y = PG_GETARG_FLOAT8(2);
    BOX b = *PG_GETARG_BOX_P(3);
    int32 xBucket = PG_GETARG_INT32(4);
    int32 yBucket = PG_GETARG_INT32(5);
    if(PG_ARGISNULL(0)) {
        int len = (xBucket * yBucket) + 1;
        Datum * result = (Datum *) palloc(len * sizeof(Datum));
        int i;
        for (i = 0; i < len; i++) {
            result[i] = Int32GetDatum(0);
        }
        int16 typlen;
        bool typbyval;
        char typalign;
        get_typlenbyvalalign(INT4OID, &typlen, &typbyval, &typalign);
        PG_RETURN_ARRAYTYPE_P(construct_array(result, len, INT4OID, typlen, typbyval, typalign));
    }
    if (b.low.x > x || b.high.x < x || b.high.y < y || b.low.y > y) {
        PG_RETURN_ARRAYTYPE_P(PG_GETARG_ARRAYTYPE_P(0));
    }
    ArrayType *result = PG_GETARG_ARRAYTYPE_P(0);
    int * array = (int *) ARR_DATA_PTR(result);
    int xIndex = (x - b.low.x) / ((b.high.x - b.low.x) / xBucket);
    int yIndex = (y - b.high.y) / ((b.low.y - b.high.y) / yBucket);
    int index = ((yIndex * yBucket) + xIndex) + 1;
    int value = ++array[index];
    if (array[0] < value) {
        array[0] = value;
    }
    PG_RETURN_ARRAYTYPE_P(result);
}