#include "api.h"
#include "data/blob.h"
#include "util.h"

static int l_lovrBlobGetName(lua_State* L) {
  Blob* blob = luax_checktype(L, 1, Blob);
  lua_pushstring(L, blob->name);
  return 1;
}

static int l_lovrBlobGetPointer(lua_State* L) {
  Blob* blob = luax_checktype(L, 1, Blob);
  lua_pushlightuserdata(L, blob->data);
  return 1;
}

static int l_lovrBlobGetSize(lua_State* L) {
  Blob* blob = luax_checktype(L, 1, Blob);
  lua_pushinteger(L, blob->size);
  return 1;
}

static int l_lovrBlobGetString(lua_State* L) {
  Blob* blob = luax_checktype(L, 1, Blob);

  lua_Integer offset = luaL_optinteger(L, 2, 0);
  luax_check(L, offset >= 0, "Blob byte offset can not be negative");
  luax_check(L, (size_t) offset < blob->size, "Blob byte offset must be less than the size of the Blob");

  lua_Integer length = luaL_optinteger(L, 3, blob->size - offset);
  luax_check(L, length >= 0, "Length can not be negative");
  luax_check(L, (size_t) length <= blob->size - offset, "Blob:getString range overflows the size of the Blob");

  lua_pushlstring(L, (char*) blob->data + offset, (size_t) length);
  return 1;
}

#define l_lovrBlobGet(L, T)\
  Blob* blob = luax_checktype(L, 1, Blob);\
  lua_Integer offset = luaL_optinteger(L, 2, 0);\
  luax_check(L, offset >= 0, "Blob byte offset can not be negative");\
  luax_check(L, (size_t) offset < blob->size, "Blob byte offset must be less than the size of the Blob");\
  lua_Integer count = luaL_optinteger(L, 3, 1);\
  luax_check(L, count > 0, "Count must be greater than zero");\
  luax_check(L, (size_t) count * sizeof(T) <= blob->size - (size_t) offset, "Byte range overflows the size of the Blob");\
  const T* data = (const T*) ((char*) blob->data + offset);\
  for (lua_Integer i = 0; i < count; i++) lua_pushnumber(L, (lua_Number) data[i]);\
  return count;

static int l_lovrBlobGetI8(lua_State* L) { l_lovrBlobGet(L, int8_t); }
static int l_lovrBlobGetU8(lua_State* L) { l_lovrBlobGet(L, uint8_t); }
static int l_lovrBlobGetI16(lua_State* L) { l_lovrBlobGet(L, int16_t); }
static int l_lovrBlobGetU16(lua_State* L) { l_lovrBlobGet(L, uint16_t); }
static int l_lovrBlobGetI32(lua_State* L) { l_lovrBlobGet(L, int32_t); }
static int l_lovrBlobGetU32(lua_State* L) { l_lovrBlobGet(L, uint32_t); }
static int l_lovrBlobGetF32(lua_State* L) { l_lovrBlobGet(L, float); }
static int l_lovrBlobGetF64(lua_State* L) { l_lovrBlobGet(L, double); }

#define l_lovrBlobSet(L, T)\
  Blob* blob = luax_checktype(L, 1, Blob);\
  lua_Integer offset = luaL_checkinteger(L, 2);\
  luax_check(L, offset >= 0, "Blob byte offset can not be negative");\
  luax_check(L, (size_t) offset < blob->size, "Blob byte offset must be less than the size of the Blob");\
  bool table = lua_istable(L, 3);\
  int count = table ? luax_len(L, 3) : lua_gettop(L) - 2;\
  luax_check(L, (size_t) count * sizeof(T) <= blob->size - (size_t) offset, "Byte range overflows the size of the Blob");\
  T* data = (T*) ((char*) blob->data + offset);\
  if (table) {\
    for (int i = 0; i < count; i++) {\
      lua_rawgeti(L, 3, i + 1);\
      data[i] = (T) lua_tonumber(L, -1);\
      lua_pop(L, 1);\
    }\
  } else {\
    for (int i = 0; i < count; i++) {\
      data[i] = luaL_checknumber(L, i + 3);\
    }\
  }\
  return count;

static int l_lovrBlobSetI8(lua_State* L) { l_lovrBlobSet(L, int8_t); }
static int l_lovrBlobSetU8(lua_State* L) { l_lovrBlobSet(L, uint8_t); }
static int l_lovrBlobSetI16(lua_State* L) { l_lovrBlobSet(L, int16_t); }
static int l_lovrBlobSetU16(lua_State* L) { l_lovrBlobSet(L, uint16_t); }
static int l_lovrBlobSetI32(lua_State* L) { l_lovrBlobSet(L, int32_t); }
static int l_lovrBlobSetU32(lua_State* L) { l_lovrBlobSet(L, uint32_t); }
static int l_lovrBlobSetF32(lua_State* L) { l_lovrBlobSet(L, float); }
static int l_lovrBlobSetF64(lua_State* L) { l_lovrBlobSet(L, double); }

const luaL_Reg lovrBlob[] = {
  { "getName", l_lovrBlobGetName },
  { "getPointer", l_lovrBlobGetPointer },
  { "getSize", l_lovrBlobGetSize },
  { "getString", l_lovrBlobGetString },
  { "getI8", l_lovrBlobGetI8 },
  { "getU8", l_lovrBlobGetU8 },
  { "getI16", l_lovrBlobGetI16 },
  { "getU16", l_lovrBlobGetU16 },
  { "getI32", l_lovrBlobGetI32 },
  { "getU32", l_lovrBlobGetU32 },
  { "getF32", l_lovrBlobGetF32 },
  { "getF64", l_lovrBlobGetF64 },
  { "setI8", l_lovrBlobSetI8 },
  { "setU8", l_lovrBlobSetU8 },
  { "setI16", l_lovrBlobSetI16 },
  { "setU16", l_lovrBlobSetU16 },
  { "setI32", l_lovrBlobSetI32 },
  { "setU32", l_lovrBlobSetU32 },
  { "setF32", l_lovrBlobSetF32 },
  { "setF64", l_lovrBlobSetF64 },
  { NULL, NULL }
};
