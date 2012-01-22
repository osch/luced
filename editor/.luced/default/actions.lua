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

local append     = table.insert
local toUpper    = luced.toUpper
local toLower    = luced.toLower
local existsFile = luced.existsFile
local openFile   = luced.openFile

local wrapLineLength = 80
local marginChars = '[*-/|+ \\t]*'
local marginRegex = '['..marginChars..']*'

local emptyLineRegex = '^['..marginChars..']*(?:\\<[/\\w]+\\>)?['..marginChars..']*$'

local function formatParagraph(view)
    local v         = view
    local cursorPos = v:getCursorPosition()
    local m
    
    m = v:findMatch([[^]], cursorPos, "b")
    
    local cursorLineStart = m and m:getBeginPos() or 0
    
    m = v:findMatch(emptyLineRegex, cursorLineStart, "b")
    
    local firstParLineStart = m and (m:getEndPos() + 1) or 0
    
    m = v:match(''..marginRegex..'', firstParLineStart)

    local firstMargin = m:getMatchedBytes() or ""
    

    m = v:findMatch('$', firstParLineStart, "f")

    local secondParLineStart = m and (m:getEndPos() + 1) or 0

    m = v:match(''..marginRegex..'', secondParLineStart)

    local secondMargin = m:getMatchedBytes() or ""
    
    m = v:findMatch('[ \\t]*$', firstParLineStart, "f")
    
    if m then
        m:remove()
        local p = m:getBeginPos()
        
        while true do
            m = v:match('(\\n'..emptyLineRegex..')|(?:(\\n'..marginRegex..')([^\\n]*)([ \\t]*)$)', p)
            if m and not m:getBeginPos(1) and m:getBeginPos(3) ~= m:getEndPos(3) then
                m:replace(2, " ")
                m:remove(4)
                p = m:getEndPos()
                --io.stderr:write(tostring(p).."\n")
            else
                break
            end
        end
    end
    local p  = firstParLineStart
    local p1 = firstParLineStart + #firstMargin
    
    local wrapColumn = 80
    while true do
        m = v:match('[^\\n]{'..tostring(wrapColumn)..'}', p)
        if m then
            p = m:getEndPos()
            m = v:findMatch('[ \\t\\n]', p, "b")
            if m then
                if m:getMatchedBytes() == '\n' or m:getBeginPos() <= p1 then
                    m = v:findMatch('[ \\t\\n]', p + 1, "f")
                    if not m then 
                        break
                    end
                end
                if m:getMatchedBytes() == '\n' then
                    break
                end
                m:replace("\n"..secondMargin)
                p  = m:getBeginPos() + 1
                p1 = m:getEndPos()
            else
                break
            end
        else
            break
        end
    end

    --v:insertAtCursor(secondMargin)
end

local correspondingFileExtensions =
{
    { { "c", "cpp", "m", "mm" }, { "h", "hpp", "h", "hpp" } }
}

local function openCorrespondingFile(view)
    if view:isFile() then
        local currentFile = view:getFileName()
        local function opened(exts1, exts2)
            for i = 1, #exts1 do
                local e1 = exts1[i]
                if currentFile:sub(-#e1) == e1 then
                    for j = 0, #exts2 - 1 do
                        local e2 = exts2[1 + ((i + j - 1) % #exts2)]
                        local correspondingName = currentFile:sub(1, -#e1-1)..e2
                        if existsFile(correspondingName) then
                            openFile(correspondingName)
                            return true
                        end
                    end
                end
            end
            return false
        end
        for _, entry in ipairs(correspondingFileExtensions) do
            if opened(entry[1], entry[2]) or opened(entry[2], entry[1]) then
                return
            end
        end
    end
end

local function smartNewline(view)
    local v = view
    if v:hasPrimarySelection() then
        v:removeSelection()
    end
    v:releaseSelection()

    local cursorPos   = v:getCursorPosition()
    local m           = v:findMatch([[^]], cursorPos, "b")
    local startOfLine = m and m:getBeginPos() or 0

    m = v:findMatch([[(\,|\;|\<|\>)|(\&\&|\|\||\.|\<\<|\+)|([(){}])]], cursorPos, startOfLine, "b")

    local separatorChar
    local isOperator = false

    if m then
        if m.match[1] then
            separatorChar = m.match[1]
        elseif m.match[2] then
            separatorChar  = m.match[2]
            isOperator     = true
        elseif m.match[3] then
            separatorChar = nil
        end
    end
    if separatorChar then
        separatorChar = string.gsub(separatorChar, '(.)', '\\%1')
    end
    local indentSpace = ""
    local numberOfAdditionalSpaces = 0

    m = v:findMatch([[^([ \t]*)]], startOfLine)

    if not m or m:getEndPos() >= cursorPos then
        indentSpace = v:getChars(startOfLine, cursorPos)
    else
        indentSpace = m.match[0]
        local firstColumn = v:getColumn(m:getEndPos())
        local positions = {}
        local p = startOfLine
        if separatorChar then
            repeat
                m = v:findMatch([[([({])[ \t]*|([})])|(]]..separatorChar..[[[ \t]*)]], p, cursorPos)
                if m then
                    p = m:getEndPos()
                    if m.match[1] == "(" or m.match[1] == "{" then 
                        append(positions, {p})
                    elseif m.match[2] == ")" or m.match[2] == "}" then
                        if #positions >= 1 then
                            positions[#positions] = nil
                        end
                    elseif m.match[3] and p < cursorPos then
                        if #positions >= 1 then
                            if isOperator then
                                append(positions[#positions], m:getBeginPos(3))
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
                         file=`readlink -f $FILE 2>/dev/null|| echo $FILE`
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
                         file=`readlink -f $FILE 2>/dev/null|| echo $FILE`
                         cd "`dirname $file`" 
                         fn="`basename $file`" 
                         if [ -e .svn ]
                         then
                            svn revert "$fn"
                         elif [ -e CVS ]
                         then
                           rev="`cat CVS/Entries | grep /$fn/ | cut -d/ -f3`" 
                           (echo "yes"|cvs unedit "$fn" 1>/dev/null 2>&1) 
                           touch "$fn"
                         else
                           homedir=`cd "$HOME"; pwd`
                           gitdir=`while test ! "$homedir" = \`pwd\`   -a  ! -e .git 
                                   do 
                                     cd ..
                                   done
                                   pwd`
                           if [ ! -e "$gitdir"/.git ]
                           then
                             echo "error: neither Git nor CVS nor svn repository"
                             exit 1
                           fi
                           git checkout -- `basename $file`
                         fi ]],
    },
    cvsCommit =
    {
        shellScript = [[ set -e
                         file=`readlink -f $FILE 2>/dev/null|| echo $FILE`
                         cd `dirname $file`
                         
                         term="xterm"
                         if type rxvt 2>/dev/null 1>&2
                         then
                           term="rxvt"
                         fi
                         
                         if [ -e CVS ]
                         then
                           $term -e cvs commit `basename $file`
                         elif [ -e .svn ]
                         then
                           $term -e svn commit `basename $file`
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
                         file=`readlink -f $FILE 2>/dev/null|| echo $FILE`
                         cd `dirname $file` 
                         fn=`basename $file`
                         if [ -e CVS -o -e .svn ]
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
                         file=`readlink -f $FILE 2>/dev/null|| echo $FILE`
                         cd `dirname $file` 
                         fn=`basename $file`
                         if [ -e CVS ]
                         then 
                           cvs log "$fn"
                         elif [ -e .svn ]
                         then
                           svn log  "$file"
                         else
                           homedir=`cd "$HOME"; pwd`
                           gitdir=`while test ! "$homedir" = \`pwd\`   -a  ! -e .git 
                                   do 
                                     cd ..
                                   done
                                   pwd`
                           if [ ! -e "$gitdir"/.git ]
                           then
                             echo "error: neither Git nor SVN nor CVS repository"
                             exit 1
                           fi
                           fullname=`cd \`dirname $fn\`
                                     echo \`pwd\`/\`basename $fn\`
                                    `
                           relname=`echo $fullname|sed s~$gitdir/~~`
                           cd "$gitdir"
                           git log $relname 
                         fi
                      ]],
    },
    
    toUpper =   function(view)
                    local old = view:getSelection()
                    local new = toUpper(old)
                    if new ~= old then
                        view:replaceSelection(new)
                    end
                end,

    toLower =   function(view)
                    local old = view:getSelection()
                    local new = toLower(old)
                    if new ~= old then
                        view:replaceSelection(new)
                    end
                end,

    openCorrespondingFile = openCorrespondingFile,

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
    formatParagraph = formatParagraph,
}

