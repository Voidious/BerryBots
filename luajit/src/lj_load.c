/*
** Load and dump code.
** Copyright (C) 2005-2015 Mike Pall. See Copyright Notice in luajit.h
*/

#include <errno.h>
#include <stdio.h>

#define lj_load_c
#define LUA_CORE

#include "lua.h"
#include "lauxlib.h"

#include "lj_obj.h"
#include "lj_gc.h"
#include "lj_err.h"
#include "lj_str.h"
#include "lj_func.h"
#include "lj_frame.h"
#include "lj_vm.h"
#include "lj_lex.h"
#include "lj_bcdump.h"
#include "lj_parse.h"

/* -- Load Lua source code and bytecode ----------------------------------- */

static TValue *cpparser(lua_State *L, lua_CFunction dummy, void *ud)
{
  LexState *ls = (LexState *)ud;
  GCproto *pt;
  GCfunc *fn;
  int bc;
  UNUSED(dummy);
  cframe_errfunc(L->cframe) = -1;  /* Inherit error function. */
  bc = lj_lex_setup(L, ls);
  // @Voidious: Totally disable loading bytecode for BerryBots.
  if (bc || (ls->mode && !strchr(ls->mode, bc ? 'b' : 't'))) {
    setstrV(L, L->top++, lj_err_str(L, LJ_ERR_XMODE));
    lj_err_throw(L, LUA_ERRSYNTAX);
  }
  pt = bc ? lj_bcread(ls) : lj_parse(ls);
  fn = lj_func_newL_empty(L, pt, tabref(L->env));
  /* Don't combine above/below into one statement. */
  setfuncV(L, L->top++, fn);
  return NULL;
}

LUA_API int lua_loadx(lua_State *L, lua_Reader reader, void *data,
		      const char *chunkname, const char *mode)
{
  LexState ls;
  int status;
  ls.rfunc = reader;
  ls.rdata = data;
  ls.chunkarg = chunkname ? chunkname : "?";
  ls.mode = mode;
  lj_str_initbuf(&ls.sb);
  status = lj_vm_cpcall(L, NULL, &ls, cpparser);
  lj_lex_cleanup(L, &ls);
  lj_gc_check(L);
  return status;
}

LUA_API int lua_load(lua_State *L, lua_Reader reader, void *data,
		     const char *chunkname)
{
  return lua_loadx(L, reader, data, chunkname, NULL);
}

typedef struct FileReaderCtx {
  FILE *fp;
  char buf[LUAL_BUFFERSIZE];
} FileReaderCtx;

static const char *reader_file(lua_State *L, void *ud, size_t *size)
{
  FileReaderCtx *ctx = (FileReaderCtx *)ud;
  UNUSED(L);
  if (feof(ctx->fp)) return NULL;
  *size = fread(ctx->buf, 1, sizeof(ctx->buf), ctx->fp);
  return *size > 0 ? ctx->buf : NULL;
}

// @Voidious: Removes the middle of a string. Leaves sliced string on top of
//            stack and removes old one from stack.
const char *sliceString(
    lua_State *L, const char *string, int start, int rest) {
  int oldStringIndex = lua_gettop(L);
  int stringLen = strlen(string);
  lua_pushlstring(L, string, start);
  lua_pushlstring(L, &(string[rest]), stringLen - rest);
  lua_concat(L, 2);
  lua_remove(L, oldStringIndex);
  return lua_tostring(L, -1);
}

// @Voidious: Figure out base directory (Lua working directory + path to
//            currently running file) so we can load relative paths with
//            dofile/loadfile. Leaves new string on top of stack.
//            Lightly based on luaL_where.
const char *getBaseDir(lua_State *L) {
  const char *luaCwd = lua_getcwd(L);
  lua_Debug ar;
  if (lua_getstack(L, 1, &ar)) {
    lua_getinfo(L, "Sl", &ar);
    if (ar.currentline > 0) {
      char *fromFinalSlash = strrchr(ar.source, '/');
      if (fromFinalSlash != NULL) {
        lua_pushstring(L, luaCwd);
        lua_pushstring(L, "/");
        // skip leading @ and go up to final slash
        lua_pushlstring(L, &(ar.source[1]), fromFinalSlash - ar.source - 1);
        lua_concat(L, 3);
        return lua_tostring(L, -1);
      }
    }
  }
  lua_pushstring(L, luaCwd);
  return lua_tostring(L, -1);
}

// @Voidious: Given a base dir and filename, combine them and parse out
//            ./ and ../ appropriately. Leaves new string on top of stack.
const char *getAbsoluteFilename(
    lua_State *L, const char *dir, const char *filename) {
  int pathFromRoot = 0;
#if defined(_WIN32)
  if ((strlen(filename) >= 3 && filename[1] == ':'
           && filename[2] == LUA_DIRSEP[0])
      || filename[0] == LUA_DIRSEP[0]) {
    pathFromRoot = 1;
  }
  char dotSlash[] = ".\\";
  char dotDotSlash[] = "..\\";
  char slashDot[] = "\\.";
#else
  if (filename[0] == LUA_DIRSEP[0]) {
    pathFromRoot = 1;
  }
  char dotSlash[] = "./";
  char dotDotSlash[] = "../";
  char slashDot[] = "/.";
#endif
  if (pathFromRoot || dir == 0) {
    lua_pushstring(L, filename);
    return lua_tostring(L, -1);
  }

  const char *absFilename =
      lua_pushfstring(L, "%s%s%s", dir, LUA_DIRSEP, filename);
  char *dots;
  int i = 0;
  while ((dots = strstr(&(absFilename[i]), dotSlash)) != NULL) {
    int offset = dots - absFilename;
    if (offset == 0 || absFilename[offset - 1] == LUA_DIRSEP[0]) {
      absFilename = sliceString(L, absFilename, offset, offset + 2);
      i = offset;
    } else {
      i = offset + 2;
    }
  }
  int filenameLen = strlen(absFilename);
  if (filenameLen >= 2
      && strcmp(&(absFilename[filenameLen - 2]), slashDot) == 0) {
    lua_pushlstring(L, absFilename, filenameLen - 2);
    absFilename = lua_tostring(L, -1);
    lua_remove(L, -2);
  }
  while ((dots = strstr(absFilename, dotDotSlash)) != NULL) {
    int prevSlash = -1;
    int x;
    for (x = dots - absFilename - 2; x > 0; x--) {
      if (absFilename[x] == LUA_DIRSEP[0]) {
        prevSlash = x;
        break;
      }
    }
    if (prevSlash == -1) {
      return filename;
    }
    absFilename =
        sliceString(L, absFilename, prevSlash, dots - absFilename + 2);
  }
  return absFilename;
}

// @Voidious: BerryBots file security enforcement.
int allowFilename(lua_State *L, const char *luaCwd, const char *absFilename) {
  if (luaCwd == 0) {
    // @Voidious: I don't think this is possible within BerryBots, but if
    //            someone does manage to create a new global_State with no cwd,
    //            or clear the cwd, don't allow them to do anything.
    return 1;
              
  } else {
    return strncmp(luaCwd, absFilename, strlen(luaCwd));
  }
}

LUALIB_API int luaL_loadfilex(lua_State *L, const char *filename,
            const char *mode)
{
   // @Voidious: BerryBots programs can only read from below working directory.
  if (filename == NULL) {
    return luaL_error(L, "not allowed to read from stdin");
  }

  const char *luaCwd = lua_getcwd(L);
  const char *baseDir = getBaseDir(L);
  const char *absFilename = getAbsoluteFilename(L, baseDir, filename);
  L->top--; // baseDir
  if (allowFilename(L, luaCwd, absFilename) != 0) {
    return luaL_error(L, "not allowed to open %s from %s", absFilename, luaCwd);
  }

  const char *scopedFilename;
  if (luaCwd == 0) {
    scopedFilename = filename;
  } else {
    scopedFilename = &(absFilename[strlen(luaCwd) + 1]);
  }

  FileReaderCtx ctx;
  int status;
  const char *chunkname;
  ctx.fp = fopen(absFilename, "rb");
  if (ctx.fp == NULL) {
    lua_pushfstring(L, "cannot open %s: %s", absFilename, strerror(errno));
    return LUA_ERRFILE;
  }
  chunkname = lua_pushfstring(L, "@%s", scopedFilename);
  lua_remove(L, -2);

  status = lua_loadx(L, reader_file, &ctx, chunkname, mode);
  if (ferror(ctx.fp)) {
    L->top -= 2;
    lua_pushfstring(L, "cannot read %s: %s", chunkname+1, strerror(errno));
    fclose(ctx.fp);
    return LUA_ERRFILE;
  }

  // @Voidious: Track list of files loaded into this Lua state. Note that this
  //            field is not hidden from the user program, so you cannot trust
  //            this list to be secure. BerryBots uses this for packaging a
  //            stage or bot, where there should be no concern about a user
  //            hacking himself.
  lua_getfield(L, LUA_REGISTRYINDEX, "__FILES");
  int new = 1;
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "__FILES");
    lua_getfield(L, LUA_REGISTRYINDEX, "__FILES");
  } else {
    int numFiles = lua_objlen(L, -1);
    int x;
    for (x = 1; x <= numFiles && new == 1; x++) {
      lua_pushinteger(L, x);
      lua_gettable(L, -2);
      const char *loadedFilename = lua_tostring(L, -1);
      if (strcmp(loadedFilename, filename) == 0) {
        new = 0;
      }
      lua_pop(L, 1);
    }
  }
  if (new) {
    lua_pushnumber(L, lua_objlen(L, -1) + 1);
    lua_pushstring(L, filename);
    lua_settable(L, -3);
  }
  lua_pop(L, 1);

  L->top--;
  copyTV(L, L->top-1, L->top);
  fclose(ctx.fp);

  return status;
}

LUALIB_API int luaL_loadfile(lua_State *L, const char *filename)
{
  return luaL_loadfilex(L, filename, NULL);
}

typedef struct StringReaderCtx {
  const char *str;
  size_t size;
} StringReaderCtx;

static const char *reader_string(lua_State *L, void *ud, size_t *size)
{
  StringReaderCtx *ctx = (StringReaderCtx *)ud;
  UNUSED(L);
  if (ctx->size == 0) return NULL;
  *size = ctx->size;
  ctx->size = 0;
  return ctx->str;
}

LUALIB_API int luaL_loadbufferx(lua_State *L, const char *buf, size_t size,
				const char *name, const char *mode)
{
  StringReaderCtx ctx;
  ctx.str = buf;
  ctx.size = size;
  return lua_loadx(L, reader_string, &ctx, name, mode);
}

LUALIB_API int luaL_loadbuffer(lua_State *L, const char *buf, size_t size,
			       const char *name)
{
  return luaL_loadbufferx(L, buf, size, name, NULL);
}

LUALIB_API int luaL_loadstring(lua_State *L, const char *s)
{
  return luaL_loadbuffer(L, s, strlen(s), s);
}

/* -- Dump bytecode ------------------------------------------------------- */

LUA_API int lua_dump(lua_State *L, lua_Writer writer, void *data)
{
  cTValue *o = L->top-1;
  api_check(L, L->top > L->base);
  if (tvisfunc(o) && isluafunc(funcV(o)))
    return lj_bcwrite(L, funcproto(funcV(o)), writer, data, 0);
  else
    return 1;
}

