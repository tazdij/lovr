#include "data/blob.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

Blob* lovrBlobCreate(void* data, size_t size, const char* name) {
  Blob* blob = lovrCalloc(sizeof(Blob));
  blob->ref = 1;
  blob->data = data;
  blob->size = size;
  blob->name = lovrStrdup(name);
  return blob;
}

void lovrBlobDestroy(void* ref) {
  Blob* blob = ref;
  lovrFree(blob->data);
  lovrFree(blob->name);
  lovrFree(blob);
}
