#ifndef WALLY_GARBAGE_COLLECTOR_H
#define WALLY_GARBAGE_COLLECTOR_H

#include "value.h"

void collectGarbage();
void markValue(Value value);
void markObject(Obj* object);

#endif //WALLY_GARBAGE_COLLECTOR_H
