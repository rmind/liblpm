/*
 * Copyright (c) 2016 Mindaugas Rasiukevicius <rmind at noxt eu>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#include <inttypes.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "lpm.h"

int		luaopen_lpm(lua_State *);
static int	lua_lpm_new(lua_State *);
static int	lua_lpm_tobin(lua_State *);
static int	lua_lpm_insert(lua_State *);
static int	lua_lpm_remove(lua_State *);
static int	lua_lpm_lookup(lua_State *);
static int	lua_lpm_flush(lua_State *);
static int	lua_lpm_gc(lua_State *);

static const struct luaL_reg lpm_lib_methods[] = {
	{ "new",	lua_lpm_new	},
	{ "tobin",	lua_lpm_tobin	},
	{ NULL,		NULL		}
};

static const struct luaL_reg lpm_methods[] = {
	{ "insert",	lua_lpm_insert	},
	{ "remove",	lua_lpm_remove	},
	{ "lookup",	lua_lpm_lookup	},
	{ "flush",	lua_lpm_flush	},
	{ "__gc",	lua_lpm_gc	},
	{ NULL,		NULL		}
};

#define	LPM_METATABLE	"lpm-obj-methods"

typedef struct {
	lpm_t *		lpm;
} lpm_lua_t;

int
luaopen_lpm(lua_State *L)
{
	if (luaL_newmetatable(L, LPM_METATABLE)) {
#if LUA_VERSION_NUM >= 502
		luaL_setfuncs(L, lpm_methods, 0);
#else
		luaL_register(L, NULL, lpm_methods);
#endif
		lua_pushliteral(L, "__index");
		lua_pushvalue(L, -2);
		lua_settable(L, -3);

		lua_pushliteral(L, "__metatable");
		lua_pushliteral(L, "must not access this metatable");
		lua_settable(L, -3);
	}
	lua_pop(L, 1);

#if LUA_VERSION_NUM >= 502
	luaL_newlib(L, lpm_lib_methods);
#else
	luaL_register(L, "lpm", lpm_lib_methods);
#endif
	return 1;
}

static int
lua_lpm_new(lua_State *L)
{
	unsigned type = lua_tointeger(L, 1);
	lpm_lua_t *lctx;
	lpm_t *lpm;

	if ((lpm = lpm_create()) == NULL) {
		luaL_error(L, "OOM");
		return 0;
	}
	lctx = (lpm_lua_t *)lua_newuserdata(L, sizeof(lpm_lua_t));
	if (lctx == NULL) {
		lpm_destroy(lpm);
		luaL_error(L, "OOM");
		return 0;
	}
	lctx->lpm = lpm;
	luaL_getmetatable(L, LPM_METATABLE);
	lua_setmetatable(L, -2);
	return 1;
}

static int
lua_lpm_tobin(lua_State *L)
{
	uint8_t addr[16];
	const char *cidr;
	unsigned preflen;
	size_t len;

	cidr = lua_tolstring(L, 2, &len);
	if (!cidr || lpm_strtobin(cidr, addr, &len, &preflen) == -1) {
		return 0;
	}
	lua_pushlstring(L, addr, len);
	lua_pushinteger(L, preflen);
	return 2;
}

static lpm_lua_t *
lua_lpm_getctx(lua_State *L)
{
	void *ud = luaL_checkudata(L, 1, LPM_METATABLE);
	luaL_argcheck(L, ud != NULL, 1, "`" LPM_METATABLE "' expected");
	return (lpm_lua_t *)ud;
}

static int
lua_lpm_gc(lua_State *L)
{
	lpm_lua_t *lctx = lua_lpm_getctx(L);
	/* FIXME release the Lua references */
	lpm_destroy(lctx->lpm);
	return 0;
}

static int
lua_lpm_insert(lua_State *L)
{
	lpm_lua_t *lctx = lua_lpm_getctx(L);
	const uint8_t *addr;
	unsigned preflen;
	size_t len;
	void *ref;

	addr = lua_tolstring(L, 2, &len);
	luaL_argcheck(L, addr, 2,
	    "`addr' binary string expected");

	preflen = lua_tointeger(L, 3);
	luaL_argcheck(L, preflen <= 128, 3,
	    "`preflen' cannot be greater than 128");

	lua_pushvalue(L, 4);
	ref = (void *)(intptr_t)luaL_ref(L, LUA_REGISTRYINDEX);

	if (lpm_insert(lctx->lpm, addr, len, preflen, ref) == 0) {
		lua_pushboolean(L, 1);
		return 1;
	}
	return 0;
}

static int
lua_lpm_remove(lua_State *L)
{
	lpm_lua_t *lctx = lua_lpm_getctx(L);
	const uint8_t *addr;
	unsigned preflen;
	size_t len;
	void *ref;

	addr = lua_tolstring(L, 2, &len);
	luaL_argcheck(L, addr, 2,
	    "`addr' binary string expected");

	preflen = lua_tointeger(L, 3);
	luaL_argcheck(L, preflen <= 128, 3,
	    "`preflen' cannot be greater than 128");

	if ((ref = lpm_lookup(lctx->lpm, addr, len)) != NULL) {
		luaL_unref(L, LUA_REGISTRYINDEX, (intptr_t)ref);
	}
	if (lpm_remove(lctx->lpm, addr, len, preflen) == 0) {
		lua_pushboolean(L, 1);
		return 1;
	}
	return 0;
}

static int
lua_lpm_lookup(lua_State *L)
{
	lpm_lua_t *lctx = lua_lpm_getctx(L);
	const uint8_t *addr;
	size_t len;
	void *ref;

	addr = lua_tolstring(L, 2, &len);
	luaL_argcheck(L, addr, 2, "`addr' binary string expected");

	if ((ref = lpm_lookup(lctx->lpm, addr, len)) != NULL) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, (intptr_t)ref);
		return 1;
	}
	return 0;
}

static int
lua_lpm_flush(lua_State *L)
{
	lpm_lua_t *lctx = lua_lpm_getctx(L);
	lpm_flush(lctx->lpm);
	return 0;
}
