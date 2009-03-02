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

#include "EditorTopWinActions.hpp"
#include "GlobalLuaInterpreter.hpp"

using namespace LucED;


void EditorTopWinActions::executeLuaScript()
{
    if (editorWidget->areCursorChangesDisabled())
    {
        return;
    }
    try
    {
        RawPtr<TextData> textData = editorWidget->getTextData();
        
        if (editorWidget->hasSelection())
        {
            TextData::TextMark selBegin  = editorWidget->getNewMarkToBeginOfSelection();
            TextData::TextMark selEnd    = editorWidget->getNewMarkToEndOfSelection();
            long               selLength = selEnd.getPos() - selBegin.getPos();
            
            LuaAccess         luaAccess    = GlobalLuaInterpreter::getInstance()->getCurrentLuaAccess();
            LuaAccess::Result scriptResult = luaAccess.executeScript((const char*) textData->getAmount(selBegin.getPos(), selLength),
                                                                     selLength);
            String output = scriptResult.output;
            
            TextData::HistorySection::Ptr historySectionHolder = textData->createHistorySection();
            
            editorWidget->hideCursor();
            editorWidget->moveCursorToTextMark(selEnd);
            if (output.getLength() > 0) {
                long insertedLength = editorWidget->insertAtCursor((const byte*) output.toCString(), output.getLength());
                editorWidget->setPrimarySelection(selEnd.getPos(), 
                                                  selEnd.getPos() + insertedLength);

                editorWidget->moveCursorToTextPosition(selEnd.getPos() + insertedLength);
                editorWidget->assureCursorVisible();
                editorWidget->moveCursorToTextMark(selEnd);
            } else {
                editorWidget->releaseSelection();
            }
            editorWidget->assureCursorVisible();
            editorWidget->rememberCursorPixX();
            editorWidget->showCursor();
        }
        else
        {
            long cursorPos = editorWidget->getCursorTextPosition();
            long spos = cursorPos;
            int parenCounter = 0;
            
            while (spos > 0)
            {
                byte c = textData->getChar(spos - 1);
                if (parenCounter > 0)
                {
                    if (c == '(') {
                        --parenCounter;
                    } else if (c == ')') {
                        ++parenCounter;
                    }
                    --spos;
                }
                else if (editorWidget->isWordCharacter(c) || c == '.') {
                    --spos;
                }
                else if (c == ')') {
                    ++parenCounter;
                    --spos;
                } else {
                    break;
                }
            }
            if (spos < cursorPos)
            {
                LuaAccess         luaAccess    = GlobalLuaInterpreter::getInstance()->getCurrentLuaAccess();
                LuaAccess::Result scriptResult = luaAccess.executeExpression((const char*) textData->getAmount(spos, cursorPos - spos),
                                                                             cursorPos - spos);
                String output = scriptResult.output;
                for (int i = 0, n = scriptResult.objects.getLength(); i < n; ++i) {
                    output << scriptResult.objects[i].toString();
                }
                if (output.getLength() > 0) 
                {
                    TextData::HistorySection::Ptr historySectionHolder = textData->createHistorySection();
                    
                    editorWidget->hideCursor();
                    editorWidget->moveCursorToTextPosition(spos);
                    editorWidget->removeAtCursor(cursorPos - spos);
                    long insertedLength = editorWidget->insertAtCursor((const byte*) output.toCString(), output.getLength());
                    editorWidget->moveCursorToTextPosition(spos + insertedLength);
                    editorWidget->assureCursorVisible();
                    editorWidget->rememberCursorPixX();
                    editorWidget->showCursor();
                }
            }
        }
    }
    catch (LuaException& ex)
    {
        messageBoxInvoker->call(MessageBoxParameter().setTitle("Lua Error")
                                                     .setMessage(ex.getMessage()));
    }
}

void EditorTopWinActions::resetLuaModules()
{
    GlobalLuaInterpreter::getInstance()->resetModules();
    GlobalConfig::getInstance()->readConfig();
}


