#!/usr/bin/env lua
-------------------------------------------------------------------------------
--
-- Preprocessor for embedded lua code within arbitrary text files. This
-- is used for code generation templates by the LucED Makefile.
--
-- This code is a modified version of
--
--                              http://lua-users.org/wiki/SimpleLuaPreprocessor
--
-------------------------------------------------------------------------------

local format = string.format
local insert = table.insert

local function append(list, toBeAppended)
    insert(list, toBeAppended)
end

local function quoteString(s)
    local rslt
    rslt = string.gsub(s,    '([\'"\\])', '\\%1')
    rslt = string.gsub(rslt, '\n',      '\\n')
    return rslt;
end

local function quoteAsString(s)
    return '"'..quoteString(s)..'"'
end

function emluaErrorHandler(msg)
    io.stderr:write(format("%s\n", debug.traceback("", 2)))
    return msg
end

local indentLevel = 0
local indentString = ""

function emluaSetIndent(newIndentLevel)
    indentLevel = newIndentLevel
    indentString = string.rep(" ", indentLevel)
end
function emluaAddIndent(plusAmount)
    emluaSetIndent(indentLevel + plusAmount)
end
function emluaGetIndentString()
    return indentString
end

local function preprocess(filename)

  local file = io.open(filename)
  local lineNumber = 1
  local chunk = { "local format = string.format;",
                  "local write  = io.write; " }

  for line in file:lines() do
    if string.find(line, "^@[^(]") or string.find(line, "^@$") then
      append(chunk, string.sub(line, 2) .. "\n")
    else
      append(chunk, format('write(emluaGetIndentString()); '))
      local last = 1
      for text, expr, index in string.gfind(line, "(.-)@(%b())()") do
        last = index
        if text ~= "" then
          append(chunk, format('write(%s); ', quoteAsString(text)))
        end
        local func, msg = loadstring("return "..expr)
        if not func then
            io.stderr:write(format("\nFile %s, line %d: Error in expression @%s: %s\n\n", filename, lineNumber, expr, msg))
            os.exit(10)
        end
        local expr2 = string.sub(expr, 2, -2)
        append(chunk, format('do '))
        append(chunk, format('  local function exprFunc()'))
        append(chunk, format('      return %s;', expr))
        append(chunk, format('  end '))
        append(chunk, format('  local callok, rslt;'))
        append(chunk, format('  callok, rslt = xpcall(exprFunc, emluaErrorHandler);'))
        
        append(chunk, format('  if not callok then '))
        append(chunk, format('      io.stderr:write(format("\\nFile %s, line %d: Error in expression @%s: %%s\\n\\n", rslt));', 
                                          filename, lineNumber, quoteString(expr)))
        append(chunk, format('      os.exit(10);'))
        append(chunk, format('  elseif rslt then '))
        append(chunk, format('      write(rslt);'))
        append(chunk, format('  end '))
        append(chunk, format('end '))
      end
      append(chunk, format('write(%s)\n',
                                         quoteAsString(string.sub(line, last).."\n")))
    end
    lineNumber = lineNumber + 1
  end
  return table.concat(chunk)
end


for _, filename in ipairs({...}) do
    local code = preprocess(filename)
    local func, msg = loadstring(code, filename)
    if not func then
       io.stderr:write("Error: " .. msg .. "\n")
       os.exit(111)
    end
    local status, msg = xpcall(func, emluaErrorHandler)
    if not status then
      io.stderr:write("Error in file "..filename..": "..msg.."\n")
      os.exit(112)
    end
end


