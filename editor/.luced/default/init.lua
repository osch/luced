-------------------------------------------------------------------------------------
--
--   LucED - The Lucid Editor
--
--   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
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


local cachedSyntaxDefinitions = {}

local cache = {}

local function getPackage(name)
    local rslt = cache[name]
    if not rslt then
        rslt = require(name)
        cache[name] = rslt
    end
    return rslt
end

return 
{
    -------------------------------------------------------------------------------
    getFonts             =  function()
                                return getPackage("this.fonts")
                            end,

    getTextStyles        =  function()
                                return getPackage("this.textStyles")
                            end,
                            
    getLanguageModes     =  function()
                                return getPackage("this.languageModes")
                            end,
                            
    getActionKeyBindings =  function()
                                return getPackage("this.actionKeyBindings")
                            end,
    -------------------------------------------------------------------------------
    
    getAction            =  function(actionName)
                                return getPackage("this.actions")[actionName]
                            end,
                        
    getSyntaxDefinition  =  function(syntaxName)
                                local rslt = cachedSyntaxDefinitions[syntaxName]
                                if rslt then
                                    return rslt
                                else
                                    local moduleName = "this."..syntaxName.."Syntax"
                                    local ok,rslt = pcall(require, moduleName)
                                    if ok then
                                        cachedSyntaxDefinitions[syntaxName] = rslt
                                        return rslt
                                    else
                                        return nil
                                    end
                                end
                            end
    -------------------------------------------------------------------------------
}
