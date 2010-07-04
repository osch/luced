local gsub = string.gsub

local function stringLiteral(value)
  return '"'..gsub(value, '"', '\\"')..'"'
end

local function noopLiteral(value)
  return tostring(value)
end

local function converterMethod(methodName)
    return function(luaVarName)
        return luaVarName.."."..methodName.."()"
    end
end

local function nullableConverterMethod(typeName, methodName)
    return function(luaVarName)
        return "("..luaVarName..".isNil() ? Nullable<"..typeName..">()"
                                      .." : Nullable<"..typeName..">("..luaVarName.."."..methodName.."()))"
    end
end

local function basicRegexConverter(luaVarName)
    return "BasicRegex("..luaVarName..".toString())"
end

local typeInfos =
{
  ["bool"]        = { convert = converterMethod("toBoolean"), literal = noopLiteral   },
  ["int"]         = { convert = converterMethod("toInt"),     literal = noopLiteral   },
  ["long"]        = { convert = converterMethod("toLong"),    literal = noopLiteral   },
  ["double"]      = { convert = converterMethod("toNumber"),  literal = noopLiteral   },
  ["String"]      = { convert = converterMethod("toString"),  literal = stringLiteral, defaultValue = "" },

  ["BasicRegex"]  = { convert = basicRegexConverter,
                      convertException = "RegexException",    literal = stringLiteral, defaultValue = "" },

  ["Nullable<bool>"] 
                  = { convert = nullableConverterMethod("bool", "toBoolean"), literal = noopLiteral, defaultValue = "Null",
                                                                      isNullable = true },

  ["map"]         = { isStructure = true },
  ["list"]        = { isStructure = true },
  ["alternative"] = { isStructure = true },
}

return 
{
    typeInfos = typeInfos
}
