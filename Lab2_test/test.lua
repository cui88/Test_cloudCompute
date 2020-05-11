-- 壓測支持HTTP pipeline的服務
-- 通過在init方法中將三個HTTP request請求拼接在一起，實現每次發送三個請求，以使用HTTP pipeline。
wrk = {
    scheme  = "http",
    host    = "localhost",
    port    = 8080,
    method  = "GET",
    path    = "/",
    headers = {},
    body    = nil,
    thread  = nil,
 }
--[[init = function(args)
   local r = {}
   r[1] = wrk.format(nil, "/?foo")
   r[2] = wrk.format(nil, "/?bar")
   r[3] = wrk.format(nil, "/?baz")

   req = table.concat(r)
end

request = function()
   return req
end--]]
