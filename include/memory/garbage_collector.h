#ifndef WALLY_GARBAGE_COLLECTOR_H
#define WALLY_GARBAGE_COLLECTOR_H

#include "value.h"

#define GC_HEAP_GROW_FACTOR 2

void collectGarbage();
void markValue(Value value);
void markObject(Obj* object);

extern bool gcStarted;

#endif //WALLY_GARBAGE_COLLECTOR_H
