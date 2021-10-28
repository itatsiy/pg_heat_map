#include "postgres.h"
#include "postgres_ext.h"

#include <limits.h>

#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/geo_decls.h"
#include "utils/array.h"
#include "utils/arrayaccess.h"
#include "utils/float.h"
#include "utils/lsyscache.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(build_heat_map);

typedef struct {
    float8 xMin;
    float8 yMin;
    int xBucketSize;
    int yBucketSize;
} PrebuildDesc;

typedef struct {
    Datum *array;
    int len;
} Result;

PrebuildDesc
defineMapLimits(Point *array, int size, float8 resolution) {
    Point first = *DatumGetPointP(&array[0]);
    Point lt = (Point) {.x = first.x, .y = first.y};
    Point rb = (Point) {.x = first.x, .y = first.y};
    int i;
    for (i = 1; i < size; i++) {
        Point value = *DatumGetPointP(&array[i]);
        if (lt.x > value.x) {
            lt.x = value.x;
        } else if (rb.x < value.x) {
            rb.x = value.x;
        }
        if (lt.y < value.y) {
            lt.y = value.y;
        } else if (rb.y > value.y) {
            rb.y = value.y;
        }
    }
    return (PrebuildDesc) {
            .xMin = lt.x,
            .yMin = rb.y,
            .xBucketSize = (int) ((rb.x - lt.x) / resolution),
            .yBucketSize = (int) ((lt.y - rb.y) / resolution),
    };
}

Result
distribute(Point *array, int size, float resolution, PrebuildDesc prebuildDesc) {
    int bucketsArrayLen = prebuildDesc.xBucketSize * prebuildDesc.yBucketSize;
    int *buckets = (int *) palloc(bucketsArrayLen * sizeof(int));
    memset(buckets, 0, bucketsArrayLen * sizeof(int));
    int resultLen = 0;
    int max = 0;
    int i;
    for (i = 0; i < size; i++) {
        Point value = *DatumGetPointP(&array[i]);
        int x = (value.x - prebuildDesc.xMin) / resolution;
        int y = (value.y - prebuildDesc.yMin) / resolution;
        int current = ++buckets[y * prebuildDesc.yBucketSize + x];
        if (max < current) {
            max = current;
        }
        if (current == 1) {
            resultLen++;
        }
    }
    resultLen *= 3;
    Datum * result = (Datum *) palloc(resultLen * sizeof(Datum));
    float8 level = max / 1.0;
    int x = 0;
    int y = 0;
    i = 0;
    float halfResolution = resolution / 2;
    for (y = 0; y < prebuildDesc.yBucketSize; y++) {
        int h = y * prebuildDesc.yBucketSize;
        for (x = 0; x < prebuildDesc.xBucketSize; x++, h++) {
            int value = buckets[h];
            if (value != 0) {
                result[i++] = Float8GetDatum(prebuildDesc.xMin + (x * resolution) + halfResolution);
                result[i++] = Float8GetDatum(prebuildDesc.yMin + (y * resolution) + halfResolution);
                result[i++] = Float8GetDatum(value / level);
            }
        }
    }
    pfree(buckets);
    return (Result) {
            .array = result,
            .len = resultLen
    };
}

Datum
build_heat_map(PG_FUNCTION_ARGS) {
    // Define input
    AnyArrayType *arg1AsArrayType = PG_GETARG_ANY_ARRAY_P(0);
    float8 resolution = PG_GETARG_FLOAT8(1);

    // Define variables
    Point *array;
    int arraySize;
    PrebuildDesc prebuildDesc;

    // Validation
    if (isinf(resolution) || AARR_NDIM(arg1AsArrayType) != 1)
        PG_RETURN_NULL();
    arraySize = AARR_DIMS(arg1AsArrayType)[0];
    if (arraySize == 0)
        PG_RETURN_NULL();

    array = (Point *) ARR_DATA_PTR(&(arg1AsArrayType->flt));
    prebuildDesc = defineMapLimits(array, arraySize, resolution);
    Result result = distribute(array, arraySize, resolution, prebuildDesc);
    int16 typlen;
    bool typbyval;
    char typalign;
    get_typlenbyvalalign(FLOAT8OID, &typlen, &typbyval, &typalign);
    PG_RETURN_ARRAYTYPE_P(construct_array(result.array, result.len, FLOAT8OID, typlen, typbyval, typalign));
}