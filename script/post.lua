--[[
Author       : foregic
Date         : 2021-12-23 20:50:15
LastEditors  : foregic
LastEditTime : 2021-12-23 20:50:15
FilePath     : /httpserver/include/post.lua
Description  : 
--]]


xmlHead = '<?xml version="1.0" encoding="utf-8" ?>\n'



function add(num1, num2)
    return num1 + num2
end

 
-- Open note file to wriet.
function openNoteFile(fileName)
	return io.open(fileName, "w")
end
 
 
-- Close writed note file.
function closeNoteFile(noteFile)
	noteFile:close()
end
 
 
function writeNestedLabel(ioChanel, label, nestCnt)
	if nestCnt == 0 then
		ioChanel:write(label)
		return
	end
 
	for i = 1, nestCnt do
		ioChanel:write("\t")
	end
 
	ioChanel:write(label)
end

function getPostResponse(...)
	a = ""
	print("getPostResponse")
	for i = 1, select("#",...), 2 do
		a = (a .. "<li>" .. select(i,...) .. "=" ..  select(i + 1,...)    .. "</li>\n")
	end

	return a

end
 
 
function generateNoteXML(fromName, toName, msgContent)
	local noteFile = openNoteFile(fromName .. "_" .. toName .. ".xml")
	if not noteFile then
		return false
	end
 
	noteFile:write(xmlHead)
	noteFile:write("<note>\n")
 
	local currNestCnt = 1
	writeNestedLabel(noteFile, "<fromName>", currNestCnt)
	noteFile:write(fromName)
	writeNestedLabel(noteFile, "</fromName>\n", 0)
 
	writeNestedLabel(noteFile, "<toName>", currNestCnt)
	noteFile:write(toName)
	writeNestedLabel(noteFile, "</toName>\n", 0)
 
	local sendTime = os.time()
	writeNestedLabel(noteFile, "<sendTime>", currNestCnt)
	noteFile:write(sendTime)
	writeNestedLabel(noteFile, "</sendTime>\n", 0)
 
	writeNestedLabel(noteFile, "<msgContent>", currNestCnt)
	noteFile:write(msgContent)
	writeNestedLabel(noteFile, "</msgContent>\n", 0)
 
	noteFile:write("</note>\n")
	closeNoteFile(noteFile)
 
	return true
end