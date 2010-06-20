-------------------------------------------------------------------------------------
--
--   LucED - The Lucid Editor
--
--   Copyright (C) 2005-2010 Oliver Schmidt, oliver at luced dot de
--
--   This program is free software; you can redistribute it and/or modify it
--   under the terms of the GNU General Public License Version 2 as published
--   by the Free Software Foundation in June 1991.
--
--   This program is distributed in the hope that it will be useful, but WITHOUT
--   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
--   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
--   more details.
--
--   You should have received a copy of the GNU General Public License along with 
--   this program; if not, write to the Free Software Foundation, Inc., 
--   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
--
-------------------------------------------------------------------------------------

print("Content-type: text/html\n\n")

local getenv = os.getenv
local concat = table.concat
local substr = string.sub

local BASE_DIR      = getenv("BASE_DIR")
local BASE_URI      = getenv("BASE_URI")

local REQUEST_URI   = getenv("REQUEST_URI")
local QUERY_STRING  = os.getenv("QUERY_STRING")

local function append(list, element)
    list[#list + 1] = element
end

--[[
print("BASE_DIR:",    BASE_DIR,   "<br>",
      "BASE_URI",     BASE_URI,   "<br>", 
      "REQUEST_URI:", REQUEST_URI)
--]]

local REQUEST_PATH = ""
local REQUEST_PATH_FOR_MESSAGES  = ""
do
    local function splitFrags(arg)
        local rslt = {}
        for p in string.gmatch(arg or "", "([^/]+)") do
            append(rslt, p)
        end
        return rslt
    end
    
    local requestFrags = splitFrags(REQUEST_URI)
    local baseFrags    = splitFrags(BASE_URI)
    
    local i = 1
    while i <= #requestFrags and requestFrags[i] == baseFrags[i] do
        i = i + 1
    end
    local j = #requestFrags
    if j > 0 and substr(requestFrags[j], 1, 1) == '?' then
        j = j - 1
    end
    if i <= j then
        REQUEST_PATH = concat(requestFrags, "/", i, j)
        REQUEST_PATH_FOR_MESSAGES  = concat(requestFrags, "/", 1, j)
    end
end
---[==[

--[[
print("<br>REQUEST_PATH:",REQUEST_PATH)
print("<br>REQUEST_PATH_FOR_MESSAGES:",REQUEST_PATH_FOR_MESSAGES)
--]]

QUERY_PARAM = {}
do
    if QUERY_STRING then
        for k,v in string.gmatch(QUERY_STRING, "(%w+)=(%w+)%&?") do
            QUERY_PARAM[k] = v
        end
    end
end
--[[
print("<br>","QUERY_PARAM[page]:",QUERY_PARAM.page)
print("<br>","QUERY_PARAM[xx]:",QUERY_PARAM.xx)
--]]

-------------------------------------------------------------------------------------
-- global Functions
--

function quoteHtmlCharacters(verbatimString)
    verbatimString = string.gsub(verbatimString, "&", "&amp;")
    verbatimString = string.gsub(verbatimString, "<", "&lt;") 
    verbatimString = string.gsub(verbatimString, ">", "&gt;")
    return verbatimString
end

function getAbsoluteFileName(relativeFilename)
    return BASE_DIR.."/"..REQUEST_PATH.."/"..relativeFilename 
end
function getAbsoluteFileNameForMessages(relativePageName)
    return REQUEST_PATH_FOR_MESSAGES.."/"..relativePageName 
end

local function quoteString(s)
    local rslt
    rslt = string.gsub(s,    '(["\\])', '\\%1')
    rslt = string.gsub(rslt, '\n',      '\\n')
    return '"'..rslt..'"'
end

local function quoteExpression(e)
    e = string.gsub(e,    '(["\'\\])', '\\%1')
    return e
end

function preprocessFile(relativeFilename)
    local filenameForMessages  = getAbsoluteFileNameForMessages(relativeFilename)
    local fullFilename = getAbsoluteFileName(relativeFilename)
    local chunks = {n=0}
    append(chunks, "local rsltChunks = {}; ")
    local file = io.open(fullFilename)
    if not file then
        return ""
    end
    local lineNumber = 1

    for line in file:lines() do
      if string.find(line, "^@[^(]") or string.find(line, "^@$") then
        append(chunks, string.sub(line, 2) .. "\n")
      else
        local last = 1
        for text, expr, index in string.gfind(line, "(.-)@(%b())()") do
          last = index
          if text ~= "" then
            append(chunks, string.format('rsltChunks[#rsltChunks+1] = %s; ', quoteString(text)))
          end
  --        append(chunks, string.format('io.write%s ', expr))
          -- probehalber loadstring, um Syntaxfehler in expr zu testen
          local func, msg = loadstring("return "..expr)
          if not func then
              error(string.format("\nFile %s, line %d: Error in expression @%s: %s\n", filenameForMessages, lineNumber, quoteExpression(expr), msg))
          end
          local expr2 = string.sub(expr, 2, -2) -- alles innerhalb (...)
          append(chunks, string.format('do '))
          append(chunks, string.format('  local function exprFunc()'))
          append(chunks, string.format('      return %s;', expr))
          append(chunks, string.format('  end '))
          append(chunks, string.format('  local callok, rslt;'))
          append(chunks, string.format('  callok, rslt = pcall(exprFunc);'))

          append(chunks, string.format('  if not callok then '))
          append(chunks, string.format('      error(string.format("\\nFile %s, line %d: Error in expression @%s: %%s\\n", rslt));', 
                                            filenameForMessages, lineNumber, quoteExpression(expr)))
          append(chunks, string.format('  elseif not rslt then '))
          if templateModeFlag and not string.match(expr2,'[^%w_]') then
              append(chunks, string.format('   rsltChunks[#rsltChunks+1] = "<<"..%s..">>";', quoteString(expr2)))
          else
              append(chunks, string.format('   error("\\nFile %s, line %d: Expression @%s is undefined.\\n");', filenameForMessages, lineNumber, quoteExpression(expr)))
          end
          append(chunks, string.format('  else '))
          append(chunks, string.format('      rsltChunks[#rsltChunks+1] = rslt;'))
          append(chunks, string.format('  end '))
          append(chunks, string.format('end '))
        end
        append(chunks, string.format('rsltChunks[#rsltChunks+1] = %s\n',
                                     quoteString(string.sub(line, last).."\n")))
      end
      lineNumber = lineNumber + 1
    end

    append(chunks, "return table.concat(rsltChunks)")

    local commandString = table.concat(chunks)
--local debugFile = io.open("/home/luced.de/htdocs/debug.out","w")
--debugFile:write(commandString)
--debugFile:close()
    ---print("XXXX", commandString, "YYY")
    local rsltFunction, errorMessage = loadstring(commandString)
    if not rsltFunction then
        error("Fehler: "..errorMessage)
    else
        return rsltFunction()
    end
end


local TEMPLATE_FILENAME = "template.lhtml"

local wasPreprocessOk, preprocessedPage = pcall(preprocessFile, TEMPLATE_FILENAME)

if not wasPreprocessOk  then
    print("<h1>Internal Server Error</h1>\n")
    print("<p><pre><code>"..quoteHtmlCharacters(preprocessedPage).."</code></pre>")
elseif preprocessedPage == "" then
    print("<h1>Internal Server Error</h1>\n")
    print("<p><pre><code>mising file '"..getAbsoluteFileNameForMessages(TEMPLATE_FILENAME).."'</code></pre>")
else
    print(preprocessedPage)
end

--]==]
