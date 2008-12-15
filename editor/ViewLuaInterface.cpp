/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
//
//   This program is free software; you can redistribute it and/or modify it
//   under the terms of the GNU General Public License Version 2 as published
//   by the Free Software Foundation in June 1991.
//
//   This program is distributed in the hope that it will be useful, but WITHOUT
//   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//   more details.
//
//   You should have received a copy of the GNU General Public License along with 
//   this program; if not, write to the Free Software Foundation, Inc., 
//   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
/////////////////////////////////////////////////////////////////////////////////////

#include "ViewLuaInterface.hpp"
#include "ActionIdRegistry.hpp"
#include "LuaCMethodArgChecker.hpp"
#include "LuaArgException.hpp"
#include "RegexException.hpp"

using namespace LucED;


LuaCFunctionResult ViewLuaInterface::getFileName(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    return LuaCFunctionResult(luaAccess) << e->getTextData()->getFileName();
}


LuaCFunctionResult ViewLuaInterface::getCursorPosition(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    return LuaCFunctionResult(luaAccess) << e->getCursorTextPosition();
}

LuaCFunctionResult ViewLuaInterface::setCursorPosition(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    LuaCMethodArgChecker<long>::check(args);
    
    long newPosition = args[1].toLong();
    
    if (newPosition < 0) {
        newPosition = 0;
    } else if (newPosition > e->getTextData()->getLength()) {
        newPosition = e->getTextData()->getLength();
    }
    
    e->moveCursorToTextPosition(newPosition);
    
    return LuaCFunctionResult(luaAccess);
}


LuaCFunctionResult ViewLuaInterface::getCursorLine(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    return LuaCFunctionResult(luaAccess) << e->getCursorLineNumber() + 1;
}


LuaCFunctionResult ViewLuaInterface::getCursorColumn(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    return LuaCFunctionResult(luaAccess) << e->getOpticalCursorColumn();
}

LuaCFunctionResult ViewLuaInterface::getColumn(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    LuaCMethodArgChecker<long>::check(args);
    
    return LuaCFunctionResult(luaAccess) << e->getOpticalColumn(args[1].toLong());
}


LuaCFunctionResult ViewLuaInterface::insertAtCursor(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    TextData::TextMark m = e->createNewMarkFromCursor();
    
    long totalInsertedLength = 0;
    
    for (int i = 1; i < args.getLength(); ++i)
    {
        if (args[i].isString()) {
            long insertedLength = e->getTextData()->insertAtMark(m, (const byte*)
                                                                    args[i].getStringPtr(),
                                                                    args[i].getStringLength());
            m.moveForwardToPos(m.getPos() + insertedLength);
            totalInsertedLength += insertedLength;
        }
        else if (args[i].isTable()) {
        
        }
    } 
    
    e->moveCursorToTextMark(m);
    
    return LuaCFunctionResult(luaAccess) << totalInsertedLength;
}

LuaCFunctionResult ViewLuaInterface::insert(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    LuaCMethodArgChecker<long, String>::check(args);
    
    long  insertPos = args[1].toLong();

    m.moveToPos(insertPos);

    long insertedLength = e->getTextData()->insertAtMark(m, (const byte*) args[2].getStringPtr(),
                                                                          args[2].getStringLength());
    return LuaCFunctionResult(luaAccess) << insertedLength;
}


LuaCFunctionResult ViewLuaInterface::getCharAtCursor(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    long pos = e->getCursorTextPosition();
    LuaVar rslt(luaAccess);
    
    if (pos < e->getTextData()->getLength())
    {
        rslt.assign(luaAccess.toLua((const char*)(e->getTextData()->getAmount(pos, 1)),
                                    1));
    }
    else {
        rslt.assign("");
    }
    return LuaCFunctionResult(luaAccess) << rslt;
}


LuaCFunctionResult ViewLuaInterface::getCharsAtCursor(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    long pos    = e->getCursorTextPosition();
    long length = e->getTextData()->getLength();
    long amount = 1;
    
    if (args.getLength() >= 2 && args[1].isNumber()) {
        amount = args[1].toLong();
    }
    if (pos + amount > length) {
        amount = length - pos;
    }
    
    LuaVar rslt(luaAccess);
    
    if (amount > 0)
    {
        rslt.assign(luaAccess.toLua((const char*)(e->getTextData()->getAmount(pos, amount)),
                                    amount));
    }
    else {
        rslt.assign("");
    }
    return LuaCFunctionResult(luaAccess) << rslt;
}


LuaCFunctionResult ViewLuaInterface::getChars(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    LuaCMethodArgChecker<long,long>::check(args);

    long pos    = args[1].toLong();
    long end    = args[2].toLong();
    long length = e->getTextData()->getLength();
    
    long amount = end - pos;
    
    if (pos + amount > length) {
        amount = length - pos;
    }
    
    LuaVar rslt(luaAccess);
    
    if (pos >= 0 && amount > 0)
    {
        rslt.assign(luaAccess.toLua((const char*)(e->getTextData()->getAmount(pos, amount)),
                                    amount));
    }
    else {
        rslt.assign("");
    }
    return LuaCFunctionResult(luaAccess) << rslt;
}


void ViewLuaInterface::parseAndSetFindUtilOptions(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();

    if (!LuaCMethodArgChecker<String,long,MoreArgs>::isValid(args))
    {
        throw LuaArgException();
    }
    String searchString  = args[1].toString();
    long   startPosition = args[2].toLong();
    long   endPosition   = -1;
    String optionsString = "";

    int index = 3;

    if (index < args.getLength() && args[index].isNumber()) {
        endPosition = args[index].toLong();
        ++index;
    }
    if (index < args.getLength() && args[index].isString()) {
        optionsString = args[index].toString();
        ++index;
    }
    if (index < args.getLength()) {
        throw LuaArgException();
    }
    
    FindUtil::Options options;
    
    for (int i = 0, n = optionsString.getLength(); i < n; ++i)
    {
        char c = optionsString[i];
        switch (c)
        {
            case 'f': options.clear(FindUtil::BACKWARD);    break;
            case 'b': options.set  (FindUtil::BACKWARD);    break;
            case 'i': options.set  (FindUtil::IGNORE_CASE); break;
            case 'r': options.set  (FindUtil::REGEX);       break;
            default:  throw LuaArgException();
        }
    }

    long   textLength    = e->getTextData()->getLength();

    if (startPosition < 0)          { startPosition = 0; }
    if (startPosition > textLength) { startPosition = startPosition; }

    findUtil.setFindString  (searchString);
    findUtil.setTextPosition(startPosition);
    findUtil.setOptions     (options);

    if (endPosition >= 0) {
        if (findUtil.getSearchForwardFlag() == true)
        {
            findUtil.setNoMatchBeforePosition(-1);
            findUtil.setMaximalEndOfMatchPosition(endPosition);
            findUtil.setMaxForwardAssertionLength(0);
        } else {
            findUtil.setNoMatchBeforePosition(endPosition);
            findUtil.setMaximalEndOfMatchPosition(-1);
            findUtil.setMaxBackwardAssertionLength(0);
        }
    }
    else {
        findUtil.setNoMatchBeforePosition(-1);
        findUtil.setMaximalEndOfMatchPosition(-1);
    }     
}


LuaCFunctionResult ViewLuaInterface::find(const LuaCFunctionArguments& args)
{
    try
    {
        LuaAccess luaAccess = args.getLuaAccess();
        
        parseAndSetFindUtilOptions(args);
        
        findUtil.findNext();
    
        if (findUtil.wasFound())
        {
            return LuaCFunctionResult(luaAccess) << findUtil.getMatchBeginPos()
                                                 << findUtil.getMatchEndPos();
        }
        else {
            return LuaCFunctionResult(luaAccess) << false;
        }
    } catch (RegexException& ex) {
        throw LuaException(String() << "Invalid Regex '" << findUtil.getFindString() 
                                    << "': " << ex.getMessage()); 
    }
}


LuaCFunctionResult ViewLuaInterface::findMatch(const LuaCFunctionArguments& args)
{
    try
    {
        LuaAccess luaAccess = args.getLuaAccess();
        
        parseAndSetFindUtilOptions(args);
        
        findUtil.setRegexFlag(true);
        
        findUtil.findNext();
    
        if (findUtil.wasFound())
        {
            LuaVar rslt  = luaAccess.newTable();
            LuaVar start = luaAccess.newTable();
            LuaVar end   = luaAccess.newTable();
            LuaVar match = luaAccess.newTable();
            
            rslt["beginPos"] = start;
            rslt["endPos"]   = end;
            rslt["match"]    = match;
            
            for (int i = 0, n = findUtil.getNumberOfCapturingSubpatterns(); i <= n; ++i)
            {
                int beg = findUtil.getCapturedSubpatternBeginPos(i);
                int len = findUtil.getCapturedSubpatternLength(i);
                if (beg != -1) {
                    start[i] = beg; 
                    end  [i] = beg + len;
                    match[i] = e->getTextData()->getSubstring(beg, len);
                } else {
                    start[i] = false; 
                    end  [i] = false;
                    match[i] = false;
                }
            }
            return LuaCFunctionResult(luaAccess) << rslt;
        }
        else {
            return LuaCFunctionResult(luaAccess) << false;
        }
    } catch (RegexException& ex) {
        throw LuaException(String() << "Invalid Regex '" << findUtil.getFindString() 
                                    << "': " << ex.getMessage()); 
    }
}

LuaCFunctionResult ViewLuaInterface::executeAction(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();

    LuaCMethodArgChecker<String>::check(args);

    String   actionName = args[1].toString();
    ActionId actionId   = ActionIdRegistry::getInstance()->getActionId(actionName);

    bool rslt = e->invokeActionMethod(actionId);
    
    return LuaCFunctionResult(luaAccess) << rslt;
}


LuaCFunctionResult ViewLuaInterface::hasPrimarySelection(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    return LuaCFunctionResult(luaAccess) << e->hasPrimarySelection();
}

LuaCFunctionResult ViewLuaInterface::hasPseudoSelection(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    return LuaCFunctionResult(luaAccess) << e->hasPseudoSelection();
}

LuaCFunctionResult ViewLuaInterface::releaseSelection(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    e->releaseSelection();
    return LuaCFunctionResult(luaAccess);
}

LuaCFunctionResult ViewLuaInterface::removeSelection(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    long selBegin  = e->getBeginSelectionPos();
    long selLength = e->getEndSelectionPos() - selBegin;
    e->moveCursorToTextPosition(selBegin);
    e->removeAtCursor(selLength);
    e->releaseSelection();
    return LuaCFunctionResult(luaAccess);
}


LuaCFunctionResult ViewLuaInterface::assureCursorVisible(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();

    e->assureCursorVisible();
    
    return LuaCFunctionResult(luaAccess);
}


LuaCFunctionResult ViewLuaInterface::setCurrentActionCategory(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();

    LuaCMethodArgChecker<String>::check(args);

    String category = args[1].toString();

    if      (category == "NEWLINE") {
        e->setCurrentActionCategory(TextEditorWidget::ACTION_NEWLINE);
    }
    else if (category == "TABULATOR") {
        e->setCurrentActionCategory(TextEditorWidget::ACTION_TABULATOR);
    }
    else if (category == "INPUT") {
        e->setCurrentActionCategory(TextEditorWidget::ACTION_KEYBOARD_INPUT);
    }
    else {
        e->setCurrentActionCategory(TextEditorWidget::ACTION_UNSPECIFIED);
    }
    
    return LuaCFunctionResult(luaAccess);
 
}