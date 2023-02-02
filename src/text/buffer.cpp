#include "buffer.h"
#include "cursorops.h"

void Buffer::pushBack(FString string) {
    insert(end(), std::move(string));
}
