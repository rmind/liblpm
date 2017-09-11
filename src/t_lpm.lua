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


-- test overwrite/remove
acl = lpm.new()
addr, preflen = lpm.tobin("1.2.3.4/5")
ok = acl:insert(addr, preflen, some_info)
assert(ok)
ok = acl:insert(addr, preflen, some_more_info)
assert(ok)

addr, preflen = lpm.tobin("84.42.21.0/28")
ok = acl:insert(addr, preflen, some_info)
assert(ok)
addr, preflen = lpm.tobin("84.42.21.0/24")
ok = acl:insert(addr, preflen, some_info)
assert(ok)

ok = acl:remove(addr, preflen)
assert(ok)
ok = acl:remove(addr, preflen)
assert(not ok)


