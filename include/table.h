#ifndef WALLY_TABLE_H
#define WALLY_TABLE_H

#include "common.h"
#include "value.h"

typedef struct {
    ObjString* key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(Table* table);

bool tableSetNoOverwrite(Table* table, ObjString* key, Value value);
bool tableSet(Table* table, ObjString* key, Value value);
bool tableGet(Table* table, ObjString* key, Value* value);
bool tableDelete(Table* table, ObjString* key);

ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash);
void tableAddAll(Table* from, Table* to);


#endif //WALLY_TABLE_H
