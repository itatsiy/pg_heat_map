#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/geo_decls.h"
#include "utils/array.h"
#include "utils/arrayaccess.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(build_heat_map);

typedef struct {
    int current;
} build_heat_map_func_struct;

Point* defineMapLimits(Point *array, int size, Point *leftTop, Point *rightBottom) {
    Point lt = *leftTop;
    Point rb = *rightBottom;
    int i;
    for (i = 0; i < size; i++) {
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
    leftTop -> x = lt.x;
    leftTop -> y = lt.y;
    rightBottom -> x = rb.x;
    rightBottom -> y = rb.y;
    Point * result = (Point *) palloc(sizeof(Point));
    result -> x = rb.x - lt.x;
    result -> y = lt.y - rb.y;
    return result;
}

Point * createPoint(Point *source) {
    Point * result = (Point *) palloc(sizeof(Point));
    result -> x = source -> x;
    result -> y = source -> y;
    return result;
}

Datum
build_heat_map(PG_FUNCTION_ARGS) {
    // Define input
    AnyArrayType *arg1AsArrayType = PG_GETARG_ANY_ARRAY_P(0);
    // Define variables
    Point *array;
    int arraySize;
    Point *leftTop;
    Point *rightBottom;
    Point *first;
    Point *diff;
    // Dimension has to be equal 1
    if (AARR_NDIM(arg1AsArrayType) != 1)
        PG_RETURN_NULL();
    // Assign array params after the check
    arraySize = AARR_DIMS(arg1AsArrayType)[0];
    if (arraySize == 0)
        PG_RETURN_NULL();
    array = (Point *) ARR_DATA_PTR(&(arg1AsArrayType->flt));
    first = &array[0];
    leftTop = createPoint(first);
    rightBottom = createPoint(first);
    diff = defineMapLimits(array, arraySize, leftTop, rightBottom);

    PG_RETURN_POINT_P(diff);
}