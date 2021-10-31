#include "postgres.h"

#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/geo_decls.h"
#include "utils/arrayaccess.h"
#include "utils/float.h"
#include "utils/lsyscache.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(int4_heat_map_agg_func);

PG_FUNCTION_INFO_V1(bitset_heat_map_agg_func);


const int ADDRESS_BITS_PER_WORD = 6;
const int BITS_PER_WORD = 1 << ADDRESS_BITS_PER_WORD;

Datum
int4_heat_map_agg_func(PG_FUNCTION_ARGS) {
    float8 x = PG_GETARG_FLOAT8(1);
    float8 y = PG_GETARG_FLOAT8(2);
    BOX b = *PG_GETARG_BOX_P(3);
    int32 xBucket = PG_GETARG_INT32(4);
    int32 yBucket = PG_GETARG_INT32(5);
    if (PG_ARGISNULL(0)) {
        int len = (xBucket * yBucket) + 2;
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
    float8 xDiff = b.high.x - b.low.x;
    float8 yDiff = b.high.y - b.low.y;
    if (xDiff < 0.0 || yDiff < 0.0) {
        PG_RETURN_ARRAYTYPE_P(PG_GETARG_ARRAYTYPE_P(0));
    }
    ArrayType *result = PG_GETARG_ARRAYTYPE_P(0);
    int *array = (int *) ARR_DATA_PTR(result);
    int xIndex = (x - b.low.x) / (xDiff / xBucket);
    int yIndex = (y - b.low.y) / (yDiff / yBucket);
    int index = ((yIndex * xBucket) + xIndex) + 2;
    int value = ++array[index];
    if (array[0] < value) {
        array[0] = value;
    }
    if (value == 1) {
        array[1]++;
    }
    PG_RETURN_ARRAYTYPE_P(result);
}

static Datum *
create_bitset(int len) {
    Datum * result = malloc(len * sizeof(Datum));
    int i;
    for (i = 0; i < len; i++) {
        result[i] = Int64GetDatum(0);
    }
    return result;
}

static void
set_true(long *bitset, int bitIndex) {
    int wordIndex = (bitIndex >> ADDRESS_BITS_PER_WORD) + 1;
    if ((bitset[wordIndex] & (1L << bitIndex)) == 0) {
        bitset[wordIndex] |= 1L << bitIndex;
        bitset[0]++;
    }
}

Datum
bitset_heat_map_agg_func(PG_FUNCTION_ARGS) {
    float8 x = PG_GETARG_FLOAT8(1);
    float8 y = PG_GETARG_FLOAT8(2);
    BOX b = *PG_GETARG_BOX_P(3);
    int32 xBucket = PG_GETARG_INT32(4);
    int32 yBucket = PG_GETARG_INT32(5);
    if (PG_ARGISNULL(0)) {
        int len = ((xBucket * yBucket) >> ADDRESS_BITS_PER_WORD) + 2;
        Datum * result = create_bitset(len);
        int16 typlen;
        bool typbyval;
        char typalign;
        get_typlenbyvalalign(INT8OID, &typlen, &typbyval, &typalign);
        PG_RETURN_ARRAYTYPE_P(construct_array(result, len, INT8OID, typlen, typbyval, typalign));
    }
    if (b.low.x > x || b.high.x < x || b.high.y < y || b.low.y > y) {
        PG_RETURN_ARRAYTYPE_P(PG_GETARG_ARRAYTYPE_P(0));
    }
    float8 xDiff = b.high.x - b.low.x;
    float8 yDiff = b.high.y - b.low.y;
    if (xDiff < 0.0 || yDiff < 0.0) {
        PG_RETURN_ARRAYTYPE_P(PG_GETARG_ARRAYTYPE_P(0));
    }
    ArrayType *result = PG_GETARG_ARRAYTYPE_P(0);
    long *array = (long *) ARR_DATA_PTR(result);
    int xIndex = (x - b.low.x) / (xDiff / xBucket);
    int yIndex = (y - b.low.y) / (yDiff / yBucket);
    int index = (yIndex * xBucket) + xIndex;
    set_true(array, index);
    PG_RETURN_ARRAYTYPE_P(result);
}