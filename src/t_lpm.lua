--
-- This file is in the Public Domain.
--

local lpm = require("lpm")

local acl = lpm.new()
local some_info = { val = "test" }
local some_more_info = { lav = "tset" }
local addr, preflen, ok, ret

addr, preflen = lpm.tobin("10.0.0.0/25")
ok = acl:insert(addr, preflen)
assert(ok)

ret = acl:lookup(lpm.tobin("10.0.0.1"))
assert(ret == true)

ret = acl:lookup(lpm.tobin("10.0.0.128"))
assert(ret == nil)

addr, preflen = lpm.tobin("10.1.1.0/24")
ok = acl:insert(addr, preflen, some_info)
assert(ok)

ret = acl:lookup(lpm.tobin("10.1.1.100"))
assert(ret.val == "test")

ret = acl:lookup(lpm.tobin("10.2.2.1"))
assert(ret == nil)


local function tracer(ind, tbl)
  local trc
  if _VERSION == "Lua 5.1" then
    -- Lua 5.1 only calls the __gc metamethod on userdata.  There is an
    -- undocumented function called 'newproxy' to create blank userdata.
    -- This function is deprecated and absent in later Lua versions.
    trc = newproxy(true)
    getmetatable(trc).__gc = function () table.insert(tbl, ind) end
  else
    trc = {}
    setmetatable(trc, {__gc = function () table.insert(tbl, ind) end})
  end
  return trc
end

-- test overwrite/remove
local gcl = {}
acl = lpm.new()
collectgarbage()    -- cleanup old lpm

addr, preflen = lpm.tobin("1.2.3.4/5")
ok = acl:insert(addr, preflen, tracer("i1", gcl))
assert(ok)
ok = acl:insert(addr, preflen, tracer("i2", gcl))
assert(ok)
collectgarbage()    -- tracer i1 should have been GC'd.
assert(table.remove(gcl) == "i1")

addr, preflen = lpm.tobin("84.42.21.0/28")
ok = acl:insert(addr, preflen, tracer("q1", gcl))
assert(ok)
addr, preflen = lpm.tobin("84.42.21.0/24")
ok = acl:insert(addr, preflen, tracer("q2", gcl))
assert(ok)
collectgarbage()    -- no tracers should have been removed
assert(#gcl == 0)

ok = acl:remove(addr, preflen)
assert(ok)
collectgarbage()    -- tracer q2 should have been GC'd, and NOT q1.
assert(table.remove(gcl) == "q2" and #gcl == 0)
ok = acl:remove(addr, preflen)
assert(not ok)
collectgarbage()    -- no tracers should have been removed
assert(#gcl == 0)

acl = nil
collectgarbage()    -- lpm collected
collectgarbage()    -- remaining tracers (i1, q1) should have been removed.
assert(#gcl == 2)

print("ok")
