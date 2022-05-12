#ifndef WALLY_ALLOCATION_LOGGER_H
#define WALLY_ALLOCATION_LOGGER_H

void logAllocation(void* pointer, size_t oldSize, size_t newSize);

#endif //WALLY_ALLOCATION_LOGGER_H
