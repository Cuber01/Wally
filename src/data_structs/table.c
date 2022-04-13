#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table* table)
{
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table* table)
{
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initTable(table);
}

static Entry* findEntry(Entry* entries, int capacity, ObjString* key)
{
    // Unhash the index
    uint32_t index = key->hash % capacity;
    Entry* tombstone = NULL;

    for (;;)
    {
        Entry* entry = &entries[index];
        if (entry->key == NULL)
        {

            if (IS_NULL(entry->value))
            {
                // Empty entry.
                return tombstone != NULL ? tombstone : entry;
            }
            else
            {
                // We found a tombstone.
                if (tombstone == NULL) tombstone = entry;
            }

        } else if (entry->key == key) {
            // We found the key.
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

static void adjustCapacity(Table* table, int capacity)
{
    // Basically rewrite the array
    Entry* entries = ALLOCATE(Entry, capacity);
    table->count = 0;

    for (int i = 0; i < capacity; i++)
    {
        entries[i].key = NULL;
        entries[i].value = NULL_VAL;
    }

    for (int i = 0; i < table->capacity; i++)
    {
        Entry* entry = &table->entries[i];
        if (entry->key == NULL) continue;

        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++; // Non tombstone entry
    }

    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

void tableAddAll(Table* from, Table* to)
{
    for (int i = 0; i < from->capacity; i++)
    {
        Entry* entry = &from->entries[i];

        if (entry->key != NULL)
        {
            tableSet(to, entry->key, entry->value);
        }
    }
}

ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash)
{
    if (table->count == 0) return NULL;

    uint32_t index = hash % table->capacity;


    for (;;)
    {
        Entry* entry = &table->entries[index];

        if (entry->key == NULL)
        {
            // Stop if we find an empty non-tombstone entry
            if (IS_NULL(entry->value)) return NULL;
        }
        else if (entry->key->length == length &&
                 entry->key->hash == hash &&
                 memcmp(entry->key->chars, chars, length) == 0)
        {
            // We found it.
            return entry->key;
        }

        index = (index + 1) % table->capacity;
    }
}

bool tableSetNoOverwrite(Table* table, ObjString* key, Value value)
{
    // Grow the table if it is at least 75% full
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD)
    {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    // See if an entry already exists, if not, make a new one and increase table count,
    // if yes, set the existing one
    Entry* entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;
    // IS_NULL check makes sure that we're not examining a tombstone
    if (!isNewKey || !IS_NULL(entry->value)) return false;

    table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableSet(Table* table, ObjString* key, Value value)
{
    // Grow the table if it is at least 75% full
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD)
    {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    // See if an entry already exists, if not, make a new one and increase table count,
    // if yes, set the existing one
    Entry* entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;
    // IS_NULL check makes sure that we're not examining a tombstone
    if (isNewKey && IS_NULL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableGet(Table* table, ObjString* key, Value* value)
{
    if (table->count == 0) return false;

    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

bool tableDelete(Table* table, ObjString* key)
{
    if (table->count == 0) return false;

    // Find the entry
    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    // Place a tombstone in the entry

    // Tombstone is an empty (NULL key and value of true) entry but still
    // contains the pointer to the next entry with the same hash,
    // just like it did before its terrible demise. This is needed to probe
    // the entry.
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}





#include "table.h"
