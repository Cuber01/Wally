#ifndef WALLY_ARRAY_H
#define WALLY_ARRAY_H

#define DECLARE_ARRAY(name, type) \
typedef struct name {             \
    unsigned int capacity; \
    unsigned int count;  \
    type* values; \
} name;

#define DEFINE_ARRAY(name, snake_case_name, type) \
	void init##name(name* array)    \
    { \
		array->values = NULL; \
		array->capacity = 0; \
		array->count = 0; \
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
			unsigned int oldCapacity = array->capacity; \
			array->capacity = GROW_CAPACITY(oldCapacity); \
			array->values = GROW_ARRAY(type, array->values, oldCapacity, array->capacity); \
		} \
		\
		array->values[array->count] = value; \
		array->count++; \
	}

#endif //WALLY_ARRAY_H
