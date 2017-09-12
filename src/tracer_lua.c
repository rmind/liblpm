/* Elliot Thomas, public domain
 *
 * This is a simple module for tracing lua's garbage collection.
 * It returns a function that creates "tracers" -- bits of userdata
 * whose sole purpose is to append a value to a table when collected.
 *
 * The tracer function takes two arguments, a value followed by a table.
 *
 * Example:
 *
 *      tracer = require("tracer")
 *      coll = {}
 *      var = tracer("foo", coll)
 *      var = nil
 *      collectgarbage()
 *      assert(coll[1] == "foo")
 */

#include <lua.h>
#include <lauxlib.h>

int luaopen_tracer(lua_State *);
static int lua_tracer(lua_State *);
static int lua_tracer_gc(lua_State *);

struct tracer
{
    int ind_refidx;
    int tbl_refidx;
};

int luaopen_tracer(lua_State *L)
{
    lua_pushcfunction(L, lua_tracer);
    return 1;
}

static int lua_tracer(lua_State *L)
{
    /* indicator, table ->  tracer */
    struct tracer trc, *tracer;

    if(lua_type(L, 2) != LUA_TTABLE)
    {
        luaL_argerror(L, 2, "expected second argument to be table");
        return 0;
    }

    trc.tbl_refidx = luaL_ref(L, LUA_REGISTRYINDEX);
    trc.ind_refidx = luaL_ref(L, LUA_REGISTRYINDEX);
    tracer = lua_newuserdata(L, sizeof(struct tracer));
    if(!tracer)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, trc.ind_refidx);
        luaL_unref(L, LUA_REGISTRYINDEX, trc.tbl_refidx);
        luaL_argerror(L, 2, "OOM");
        return 0;
    }

    *tracer = trc;

    lua_newtable(L);
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, lua_tracer_gc);
    lua_settable(L, -3);
    lua_setmetatable(L, 1);

    return 1;
}

static int lua_tracer_gc(lua_State *L)
{
    struct tracer *tracer;
    if(lua_type(L, 1) != LUA_TUSERDATA)
    {
        luaL_argerror(L, 1, "expected a tracer");
        return 0;
    }

    tracer = lua_touserdata(L, 1);

    lua_rawgeti(L, LUA_REGISTRYINDEX, tracer->tbl_refidx);
    lua_pushinteger(L, lua_objlen(L, -1) + 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, tracer->ind_refidx);
    lua_settable(L, -3);

    luaL_unref(L, LUA_REGISTRYINDEX, tracer->ind_refidx);
    luaL_unref(L, LUA_REGISTRYINDEX, tracer->tbl_refidx);
    return 0;
}

