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

local thisModuleName    = ...
local thisPackagePrefix = luced.getModulePrefix(thisModuleName)

local append = table.insert

local function smartNewline(view)
    local v = view
    if v:hasPrimarySelection() then
        v:removeSelection()
    end
    v:releaseSelection()

    local cursorPos   = v:getCursorPosition()
    local m           = v:findMatch([[^]], cursorPos, "b")
    local startOfLine = m.beginPos[0]

    m = v:findMatch([[(\,|\;|\<|\>)|(\&\&|\|\||\.|\<\<|\+)]], cursorPos, startOfLine, "b")

    local separatorChar
    local isOperator = false

    if m then
        if m.match[1] then
            separatorChar = m.match[1]
        elseif m.match[2] then
            separatorChar  = m.match[2]
            isOperator     = true
        end
    end
    if separatorChar then
        separatorChar = string.gsub(separatorChar, '(.)', '\\%1')
    end
    local indentSpace = ""
    local numberOfAdditionalSpaces = 0

    m = v:findMatch([[^([ \t]*)]], startOfLine)

    if m.endPos[0] > cursorPos then
        indentSpace = v:getChars(startOfLine, cursorPos)
    else
        indentSpace = m.match[0]
        local firstColumn = v:getColumn(m.endPos[0])
        local positions = {}
        local p = startOfLine
        if separatorChar then
            repeat
                m = v:findMatch([[([({])[ \t]*|([})])|(]]..separatorChar..[[[ \t]*)]], p, cursorPos)
                if m then
                    p = m.endPos[0]
                    if m.match[1] == "(" or m.match[1] == "{" then 
                        append(positions, {p})
                    elseif m.match[2] == ")" or m.match[2] == "}" then
                        if #positions >= 1 then
                            positions[#positions] = nil
                        end
                    elseif m.match[3] and p < cursorPos then
                        if #positions >= 1 then
                            if isOperator then
                                append(positions[#positions], m.beginPos[3])
                            else
                                append(positions[#positions], p)
                            end
                        end
                    end
                end
            until not m or p >= cursorPos
        end
        if #positions >= 1 then
            local lastPositions = positions[#positions]
            local column = v:getColumn(lastPositions[#lastPositions])
            numberOfAdditionalSpaces = column - firstColumn
        end
    end
    v:insertAtCursor("\n" .. indentSpace .. string.rep(" ", numberOfAdditionalSpaces))
    v:assureCursorVisible()
    v:setCurrentActionCategory("NEWLINE")
end


return
{
    cvsEdit =
    {
        shellScript = [[ set -e
                         file=`readlink -f $FILE`
                         cd "`dirname $file`"
                         fn="`basename $file`"
                         cvs update "$fn"
                         rev=`cat CVS/Entries | grep "/$fn/" | cut -d/ -f3` 
                         cvs editors "$fn" 
                         cvs edit -a none "$fn" ]],
    },
    cvsUnedit =
    {
        shellScript = [[ set -e
                         file=`readlink -f $FILE`
                         cd "`dirname $file`" 
                         fn="`basename $file`" 
                         rev="`cat CVS/Entries | grep /$fn/ | cut -d/ -f3`" 
                         (echo "yes"|cvs unedit "$fn" 1>/dev/null 2>&1) 
                         touch "$fn" ]],
    },
    cvsCommit =
    {
        shellScript = [[ set -e
                         file=`readlink -f $FILE`
                         cd `dirname $file`
                         
                         term="xterm"
                         if type rxvt 2>/dev/null 1>&2
                         then
                           term="rxvt"
                         fi
                         
                         if [ -e CVS ]
                         then
                           $term -e cvs commit `basename $file`
                         else
                           homedir=`cd "$HOME"; pwd`
                           gitdir=`while test ! "$homedir" = \`pwd\`   -a  ! -e .git 
                                   do 
                                     cd ..
                                   done
                                   pwd`
                           if [ ! -e "$gitdir"/.git ]
                           then
                             echo "error: neither Git nor CVS repository"
                             exit 1
                           fi
                           $term -e git commit `basename $file`
                         fi
                      ]],
    },
    cvsDiff =
    {
        shellScript = [[ set -e
                         file=`readlink -f $FILE`
                         cd `dirname $file` 
                         fn=`basename $file`
                         if [ -e CVS ]
                         then 
                           tkdiff $fn
                         else
                           homedir=`cd "$HOME"; pwd`
                           gitdir=`while test ! "$homedir" = \`pwd\`   -a  ! -e .git 
                                   do 
                                     cd ..
                                   done
                                   pwd`
                           if [ ! -e "$gitdir"/.git ]
                           then
                             echo "error: neither Git nor CVS repository"
                             exit 1
                           fi
                           fullname=`cd \`dirname $fn\`
                                     echo \`pwd\`/\`basename $fn\`
                                    `
                           relname=`echo $fullname|sed s~$gitdir/~~`
                           tag1="HEAD"
                           t1=`mktemp -t "old.$tag1.\`basename $fn\`.XXXXXX"`
                           git show $tag1:$relname > $t1
                           tkdiff "$t1" "$fn" 
                           rm "$t1"
                         fi
                      ]],
    },
    cvsLog =
    {
        shellScript = [[ set -e
                         file=`readlink -f $FILE`
                         cd `dirname $file` 
                         fn=`basename $file` 
                         cvs log $fn ]],
    },

    test1 = function(view)
               view:insertAtCursor("xx_2_yy")
            end,
    test2 =
    {
        shellScript = [[ echo "<$FILE>"; ls|head -n3 ]],
    },
    test3 =
    {
        shellScript = [[ echo "<$FILE>"
                         ls
                         echo -n "testeerituertiouertoiuretioreutoireutret ABC" ]],
    },
    smartNewline = smartNewline,
}

