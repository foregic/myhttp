--[[
Author       : foregic
Date         : 2021-12-23 20:50:15
LastEditors  : foregic
LastEditTime : 2021-12-23 20:50:15
FilePath     : /httpserver/include/post.lua
Description  : 
--]]


function getPostResponse(...)
	local a = ""
	-- print("getPostResponse")
	for i = 1, select("#",...), 2 do
		a = (a .. "<li>" .. select(i,...) .. "=" ..  select(i + 1,...)    .. "</li>\n")
	end

	return a

end
 
