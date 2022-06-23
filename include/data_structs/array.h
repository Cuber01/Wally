#ifndef WALLY_ARRAY_H
#define WALLY_ARRAY_H

#include "common.h"

#define DECLARE_ARRAY(name, type) \
typedef struct name {             \
    uint capacity; \
    uint count;  \
    type* values; \
} name;

#define DEFINE_ARRAY_FUNCTIONS(name, snake_case_name, type) \
	name* init##name(name* array)    \
    {                                                       \
        array = reallocate(array, 0, sizeof(name));         \
                                                            \
		array->values = NULL; \
		array->capacity = 0; \
		array->count = 0;                                         \
                                                            \
        return array;                                       \
	} \
	\
	void free##name(name* array) \
    { \
		FREE_ARRAY(type, array->values, array->capacity); \
		init##name(array); \
	} \
	\
	void snake_case_name##Write(name* array, type value) \
    { \
		if (array->capacity < array->count + 1) { \
			uint oldCapacity = array->capacity; \
			array->capacity = GROW_CAPACITY(oldCapacity); \
			array->values = GROW_ARRAY(type, array->values, oldCapacity, array->capacity); \
		} \
		\
		array->values[array->count] = value; \
		array->count++; \
	}

#define DEFINE_ARRAY_FUNCTION_PREDECLARATIONS(name, snake_case_name, type) \
    name* init##name(name* array); \
    void free##name(name* array); \
    void snake_case_name##Write(name* array, type value); \

DECLARE_ARRAY(UInts, uint)
DEFINE_ARRAY_FUNCTION_PREDECLARATIONS(UInts, uints, uint)



#endif //WALLY_ARRAY_H
