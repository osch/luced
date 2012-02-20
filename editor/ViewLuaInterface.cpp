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
#include "MatchLuaInterface.hpp"

using namespace LucED;


LuaCFunctionResult ViewLuaInterface::isFile(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    return LuaCFunctionResult(luaAccess) << !textData->isFileNamePseudo();
}

LuaCFunctionResult ViewLuaInterface::getFileName(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    if (!textData->isFileNamePseudo()) {
        return LuaCFunctionResult(luaAccess) << textData->getFileName();
    } else {
        return LuaCFunctionResult(luaAccess);
    }
}

LuaCFunctionResult ViewLuaInterface::getDisplayFileName(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    return LuaCFunctionResult(luaAccess) << textData->getFileName();
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
    
    long newPosition = args[0].toLong();
    
    if (newPosition < 0) {
        newPosition = 0;
    } else if (newPosition > textData->getLength()) {
        newPosition = textData->getLength();
    }
    newPosition = textData->getBeginOfWChar(newPosition);

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
    
    return LuaCFunctionResult(luaAccess) << e->getOpticalColumn(args[0].toLong());
}


LuaCFunctionResult ViewLuaInterface::insertAtCursor(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();

    if (e->hasSelection())
    {
        if (   e->getCursorTextPosition() < e->getBeginSelectionPos()
            || e->getCursorTextPosition() > e->getEndSelectionPos())
        {
            e->releaseSelection();
        }
    }
    
    TextData::TextMark m = e->createNewMarkFromCursor();
    
    long totalInsertedLength = 0;
    
    for (int i = 0; i < args.getLength(); ++i)
    {
        if (args[i].isString()) 
        {
            long pos = m.getPos();
            
            long insertedLength = textData->insertAtMark(m, (const byte*)
                                                            args[i].getStringPtr(),
                                                            args[i].getStringLength());
            m.moveToPos(textData->getEndOfWChar(pos + insertedLength));
            totalInsertedLength += insertedLength;
        }
        else if (args[i].isTable()) {
        
        }
    } 
    if (e->hasSelection())
    {
        if (e->getCursorTextPosition() == e->getEndSelectionPos()) {
            e->moveSelectionEndTo(m.getPos());
        }
    }
    e->moveCursorToTextMark(m);
    
    return LuaCFunctionResult(luaAccess) << totalInsertedLength;
}

LuaCFunctionResult ViewLuaInterface::insert(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    LuaCMethodArgChecker<long, String>::check(args);
    
    long  insertPos = args[0].toLong();

    m.moveToPos(insertPos);

    long insertedLength = textData->insertAtMark(m, args[1].toString());
    
    return LuaCFunctionResult(luaAccess) << insertedLength;
}


LuaCFunctionResult ViewLuaInterface::getCharAtCursor(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    long pos = e->getCursorTextPosition();
    
    LuaVar rslt(luaAccess);
    
    if (0 <= pos && pos < textData->getLength())
    {
        long spos = textData->getBeginOfWChar(pos);
        long epos = textData->getNextBeginOfWChar(spos);
        long len  = epos - spos;
        
        rslt = luaAccess.toLua((const char*)(textData->getAmount(spos, len)), len);
    }
    else {
        rslt = "";
    }
    return LuaCFunctionResult(luaAccess) << rslt;
}

LuaCFunctionResult ViewLuaInterface::getByteAtCursor(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    long pos = e->getCursorTextPosition();
    
    LuaVar rslt(luaAccess);
    
    if (0 <= pos && pos < textData->getLength())
    {
        rslt = luaAccess.toLua((const char*)(textData->getAmount(pos, 1)), 1);
    }
    else {
        rslt = "";
    }
    return LuaCFunctionResult(luaAccess) << rslt;
}


LuaCFunctionResult ViewLuaInterface::getBytesAtCursor(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    long pos    = e->getCursorTextPosition();
    long length = textData->getLength();
    long amount = 1;
    
    if (args.getLength() >= 1 && args[0].isNumber()) {
        amount = args[0].toLong();
    }
    if (pos + amount > length) {
        amount = length - pos;
    }
    
    LuaVar rslt(luaAccess);
    
    if (amount > 0)
    {
        rslt = luaAccess.toLua((const char*)(textData->getAmount(pos, amount)),
                               amount);
    }
    else {
        rslt = "";
    }
    return LuaCFunctionResult(luaAccess) << rslt;
}


LuaCFunctionResult ViewLuaInterface::getBytes(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    LuaCMethodArgChecker<long,long>::check(args);

    long pos    = args[0].toLong();
    long end    = args[1].toLong();
    long length = textData->getLength();
    
    long amount = end - pos;
    
    if (pos + amount > length) {
        amount = length - pos;
    }
    
    LuaVar rslt(luaAccess);
    
    if (pos >= 0 && amount > 0)
    {
        rslt = luaAccess.toLua((const char*)(textData->getAmount(pos, amount)),
                               amount);
    }
    else {
        rslt = "";
    }
    return LuaCFunctionResult(luaAccess) << rslt;
}

void ViewLuaInterface::parseAndSetFindUtilOptions(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();

    if (!LuaCMethodArgChecker<String,long,MoreArgs>::isValid(args))
    {
        throw LuaArgException(luaAccess);
    }
    String searchString  = args[0].toString();
    long   startPosition = args[1].toLong();
    long   endPosition   = -1;
    String optionsString = "";

    int index = 2;

    if (index < args.getLength() && args[index].isNumber()) {
        endPosition = args[index].toLong();
        ++index;
    }
    if (index < args.getLength() && args[index].isString()) {
        optionsString = args[index].toString();
        ++index;
    }
    if (index < args.getLength()) {
        throw LuaArgException(luaAccess);
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
            default:  throw LuaArgException(luaAccess);
        }
    }

    long   textLength    = textData->getLength();

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
    LuaAccess luaAccess = args.getLuaAccess();

    try {
        parseAndSetFindUtilOptions(args);
        
        findUtil.findNext();
    
        if (findUtil.wasFound())
        {
            return LuaCFunctionResult(luaAccess) << findUtil.getMatchBeginPos()
                                                 << findUtil.getMatchEndPos();
        }
        else {
            return LuaCFunctionResult(luaAccess);
        }
    } catch (RegexException& ex) {
        throw LuaException(luaAccess,
                           String() << "Invalid Regex '" << findUtil.getFindString() 
                                    << "': " << ex.getMessage()); 
    }
}


LuaCFunctionResult ViewLuaInterface::findMatch(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    try {
        parseAndSetFindUtilOptions(args);
        
        findUtil.setRegexFlag(true);
        
        findUtil.findNext();
    
        if (findUtil.wasFound())
        {
            return LuaCFunctionResult(luaAccess) << MatchLuaInterface::create(e,
                                                                              findUtil.getBasicRegex(),
                                                                              findUtil.getOvector());
        }
        else {
            return LuaCFunctionResult(luaAccess) ;
        }
    } catch (RegexException& ex) {
        throw LuaException(luaAccess,
                           String() << "Invalid Regex '" << findUtil.getFindString() 
                                    << "': " << ex.getMessage()); 
    }
}

LuaCFunctionResult ViewLuaInterface::match(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    try {
        parseAndSetFindUtilOptions(args);
        
        findUtil.setRegexFlag(true);
        
        if (findUtil.doesMatch())
        {
            return LuaCFunctionResult(luaAccess) << MatchLuaInterface::create(e,
                                                                              findUtil.getBasicRegex(),
                                                                              findUtil.getOvector());
        }
        else {
            return LuaCFunctionResult(luaAccess);
        }
    } catch (RegexException& ex) {
        throw LuaException(luaAccess,
                           String() << "Invalid Regex '" << findUtil.getFindString() 
                                    << "': " << ex.getMessage()); 
    }
}

LuaCFunctionResult ViewLuaInterface::executeAction(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();

    LuaCMethodArgChecker<String>::check(args);

    String   actionName = args[0].toString();
    ActionId actionId   = ActionIdRegistry::getInstance()->getActionId(actionName);

    bool rslt = e->getKeyActionHandler()->invokeActionMethod(actionId);
    
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


LuaCFunctionResult ViewLuaInterface::getSelection(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    String rslt;
    
    if (e->hasPrimarySelection() || e->hasPseudoSelection())
    {
        rslt = textData->getSubstring(Pos(e->getBeginSelectionPos()),
                                      Pos(e->getEndSelectionPos()));

    }
    return LuaCFunctionResult(luaAccess) << rslt;
}

LuaCFunctionResult ViewLuaInterface::replaceSelection(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    if (args.getLength() != 1 || !args[0].isString()) {
        throw LuaArgException(luaAccess);
    }
    
    String arg = args[0].toString();

    TextData::TextMark m = e->createNewMarkFromCursor();
    
    if (e->hasPrimarySelection() || e->hasPseudoSelection())
    {
        long spos = e->getBeginSelectionPos();
        long epos = e->getEndSelectionPos();
        
        m.moveToPos(spos);

        textData->insertAtMark(m, arg);
        
        m.moveToPos(spos + arg.getLength());
        
        textData->removeAtMark(m, epos - spos);
    }
    else {
        textData->insertAtMark(m, arg);
    }
    return LuaCFunctionResult(luaAccess);
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

LuaCFunctionResult ViewLuaInterface::remove(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();

    LuaCMethodArgChecker<long,long>::check(args);
    
    long spos = args[0].toLong();
    long epos = args[1].toLong();
    
    if (epos > textData->getLength()) {
        epos = textData->getLength();
    }
    if (spos < 0) {
        spos = 0;
    }
    
    if (epos > spos)
    {
        TextData::TextMark m = e->createNewMarkFromCursor();
        m.moveToPos(spos);
        textData->removeAtMark(m, epos - spos);
    }
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

    String category = args[0].toString();

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
