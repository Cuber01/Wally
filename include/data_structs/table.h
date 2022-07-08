#ifndef WALLY_TABLE_H
#define WALLY_TABLE_H

#include "common.h"
#include "value.h"

#define TABLE_SUCCESS 0
#define TABLE_ERROR_REDEFINE 1
#define TABLE_ERROR_FUNCTION_SET 2
#define TABLE_ERROR_UNDEFINED_SET 3

#define ALLOCATE_TABLE() \
    reallocate(NULL, 0, sizeof(Table))

typedef struct {
    ObjString* key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
    ObjString** keys;
} Table;

void initTable(Table* table);
void freeTable(Table* table);

// Create new entry. Error if given key already exists.
uint8_t tableDefineEntry(Table* table, ObjString* key, Value value);
// Set an entry. Error if given key doesn't exist.
uint8_t tableSetExistingEntry(Table* table, ObjString* key, Value value);
// Set or create an entry if it doesn't exist.
uint8_t tableSet(Table* table, ObjString* key, Value value);

bool tableGet(Table* table, ObjString* key, Value* value);
bool tableDelete(Table* table, ObjString* key);

ObjString* tableFindString(Table* table, const char* chars, uint length, uint32_t hash);
void tableAddAll(Table* from, Table* to);

void markTable(Table* table);
void tableRemoveWhite(Table* table);


#endif //WALLY_TABLE_H
