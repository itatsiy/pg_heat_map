#include "postgres.h"

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
    int * distributed;
    int len;
    int min;
    int max;
} DistributionDesc;

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

DistributionDesc
distribute(Point *array, int size, float resolution, PrebuildDesc prebuildDesc) {
    int resultArraySize = prebuildDesc.xBucketSize * prebuildDesc.yBucketSize;
    int *result = (int *) palloc(resultArraySize * sizeof(int));
    memset(result, 0, resultArraySize * sizeof(int));
    int max = 0;
    int min = INT_MAX;
    int i;
    for (i = 0; i < size; i++) {
        Point value = *DatumGetPointP(&array[i]);
        int x = (value.x - prebuildDesc.xMin) / resolution;
        int y = (value.y - prebuildDesc.yMin) / resolution;
        int current = ++result[y * prebuildDesc.yBucketSize + x];
        if (max < current) {
            max = current;
        }
        if (min > current) {
            min = current;
        }
    }
    return (DistributionDesc) {
        .distributed = result,
        .len = resultArraySize,
        .min = min,
        .max = max
    };
}

Datum *
normalize(DistributionDesc distributionDesc) {
    Datum *result = (Datum *) palloc(distributionDesc.len * sizeof(Datum));
    int i;
    for (i = 0; i < distributionDesc.len; i++) {
        int value = distributionDesc.distributed[i];
        result[i] = Int32GetDatum(value);
    }
    return result;
}

Datum
build_heat_map(PG_FUNCTION_ARGS) {
    ArrayType *result;
    Datum *resultDatum;
    // Define input
    AnyArrayType *arg1AsArrayType = PG_GETARG_ANY_ARRAY_P(0);
    float8 resolution = PG_GETARG_FLOAT8(1);

    // Define variables
    Point *array;
    int arraySize;
    PrebuildDesc prebuildDesc;
    DistributionDesc distributionDesc;

    // Validation
    if (isinf(resolution) || AARR_NDIM(arg1AsArrayType) != 1)
        PG_RETURN_NULL();
    arraySize = AARR_DIMS(arg1AsArrayType)[0];
    if (arraySize == 0)
        PG_RETURN_NULL();

    array = (Point *) ARR_DATA_PTR(&(arg1AsArrayType->flt));

    prebuildDesc = defineMapLimits(array, arraySize, resolution);

    distributionDesc = distribute(array, arraySize, resolution, prebuildDesc);
    resultDatum = normalize(distributionDesc);
    int16 typlen;
    bool typbyval;
    char typalign;
    get_typlenbyvalalign(INT4OID, &typlen, &typbyval, &typalign);
    result = construct_array(resultDatum, distributionDesc.len, INT4OID, typlen, typbyval, typalign);
    PG_RETURN_ARRAYTYPE_P(result);
}