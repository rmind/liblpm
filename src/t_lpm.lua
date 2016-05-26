local lpm = require("lpm")

local acl = lpm.new()
local some_info = { val = "test" }

local addr, preflen = lpm:tobin("10.0.0.0/24")
local ok = acl:insert(addr, preflen, some_info)
assert(ok)

local ret = acl:lookup(lpm:tobin("10.0.0.100"))
assert(ret.val == "test")
