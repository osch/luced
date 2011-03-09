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

return
{
    {
        name     = "LucED",
        class    = "LucedLuaInterface",

        ptrType  = "Singleton",
        
        methods  = {
                     { name = "getModulePrefix"
                     },
                     { name = "resetModules"
                     },
                     { name = "getCurrentView"
                     },
                     { name = "getLoadedPackageModules"
                     },
                     { name = "toUpper"
                     },
                     { name = "toLower"
                     },
                     { name = "openFile"
                     },
                   }
    },
    {
        name     = "View",
        class    = "ViewLuaInterface",

        ptrType  = "WeakPtr",
        
        methods  = {
                     { name = "getFileName"
                     },
                     { name = "getCursorPosition"
                     },
                     { name = "setCursorPosition"
                     },
                     { name = "getCursorLine"
                     },
                     { name = "getCursorColumn"
                     },
                     { name = "getColumn"
                     },
                     { name = "getCharAtCursor"
                     },
                     { name = "getByteAtCursor"
                     },
                     { name = "getBytesAtCursor"
                     },
                     { name = "getBytes"
                     },
                     { name = "insertAtCursor"
                     },
                     { name = "insert"
                     },
                     { name = "find"
                     },
                     { name = "findMatch"
                     },
                     { name = "match"
                     },
                     { name = "getMatchedBytes"
                     },
                     { name = "executeAction"
                     },
                     { name = "hasPrimarySelection"
                     },
                     { name = "hasPseudoSelection"
                     },
                     { name = "getSelection"
                     },
                     { name = "replaceSelection"
                     },
                     { name = "releaseSelection"
                     },
                     { name = "removeSelection"
                     },
                     { name = "remove"
                     },
                     { name = "assureCursorVisible"
                     },
                     { name = "setCurrentActionCategory"
                     },
                   }
    },
}
