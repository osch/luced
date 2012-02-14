/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2012 Oliver Schmidt, oliver at luced dot de
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

#ifndef LUA_ERROR_HANDLER_HPP
#define LUA_ERROR_HANDLER_HPP

#include "RunningComponent.hpp"
#include "OwningPtr.hpp"
#include "WeakPtr.hpp"
#include "ExceptionLuaInterface.hpp"
#include "EventDispatcher.hpp"
#include "Callback.hpp"
#include "MessageBoxParameter.hpp"
#include "FileOpener.hpp"
#include "System.hpp"
#include "TextData.hpp"
#include "LanguageModes.hpp"
#include "HilitedText.hpp"
#include "EditorTopWin.hpp"
#include "TopWinList.hpp"

namespace LucED
{

class LuaErrorHandler : public RunningComponent
{
public:
    typedef OwningPtr<LuaErrorHandler> Ptr;

    static WeakPtr<LuaErrorHandler> start(ExceptionLuaInterface::Ptr                luaException,
                                          Callback<const MessageBoxParameter&>::Ptr messageBoxInvoker)
    {
        Ptr ptr(new LuaErrorHandler(luaException, messageBoxInvoker));
        EventDispatcher::getInstance()->registerRunningComponent(ptr);
        ptr->startMessageBox();
        return ptr;
    }

private:
    LuaErrorHandler(ExceptionLuaInterface::Ptr                luaException,
                    Callback<const MessageBoxParameter&>::Ptr messageBoxInvoker)
        : luaException(luaException),
          messageBoxInvoker(messageBoxInvoker)
    {}
    
    void startMessageBox()
    {
        if (luaException->hasFileSource() && !luaException->isBuiltinFile())
        {
            messageBoxInvoker->call(MessageBoxParameter().setTitle("Lua Error")
                                                         .setMessage(String() << "Lua Error in file '" << luaException->getFileName() << "'")
                                                         .setDefaultButton("O]pen file", newCallback(this, &LuaErrorHandler::handleOpenFileButton))
                                                         .setCancelButton ("C]ancel",    newCallback(this, &LuaErrorHandler::handleAbortButton)));
        }
        else if (luaException->hasScriptBytes() && !luaException->isBuiltinFile())
        {
            messageBoxInvoker->call(MessageBoxParameter().setTitle("Lua Error")
                                                         .setMessage(luaException->getMessage())
                                                         .setDefaultButton("O]pen script", newCallback(this, &LuaErrorHandler::handleOpenStringButton))
                                                         .setCancelButton ("C]ancel",      newCallback(this, &LuaErrorHandler::handleAbortButton)));
        }
        else if (luaException->isBuiltinFile())
        {
            messageBoxInvoker->call(MessageBoxParameter().setTitle("Internal Lua Error")
                                                         .setMessage(String() << "Lue Error in internal file '"
                                                                              << luaException->getFileName()
                                                                              << "', line " << luaException->getFileLineNumber()
                                                                              << ": "
                                                                              << luaException->getMessage()));
        }
        else
        {
            messageBoxInvoker->call(MessageBoxParameter().setTitle("Lua Error")
                                                         .setMessage(luaException->getMessage()));
        }
    }
    
    void handleOpenFileButton()
    {
        if (luaException->hasFileSource() && !luaException->isBuiltinFile())
        {
            EditorTopWin::Ptr win;
            
            RawPtr<TopWinList> topWins = TopWinList::getInstance();
            String fileName = File(luaException->getFileName()).getAbsoluteNameWithResolvedLinks();
            for (int w = 0; w < topWins->getNumberOfTopWins(); ++w)
            {
                EditorTopWin* topWin = dynamic_cast<EditorTopWin*>(topWins->getTopWin(w));
                if (topWin != NULL && File(topWin->getFileName()).getAbsoluteNameWithResolvedLinks() == fileName) {
                    win = topWin;
                    break;
                }
            }

            if (win.isValid()) {
                win->raise();
            }
            else {
                TextData::Ptr textData = TextData::create();
                String        fileName = luaException->getFileName();
                
                textData->loadFile(fileName);

                LanguageMode::Ptr languageMode = GlobalConfig::getInstance()->getLanguageModeForFileName(fileName);
                
                if (!languageMode.isValid()) {
                    languageMode = GlobalConfig::getInstance()->getDefaultLanguageMode();
                }
                
                HilitedText::Ptr  hilitedText = HilitedText::create(textData, languageMode);
                
                win = EditorTopWin::create(hilitedText);
                win->show();
            }
            if (luaException->getFileLineNumber() >= 0) {
                win->gotoLineNumber(luaException->getFileLineNumber());
            }
            if (luaException->hasScriptBytes()) {
                win->setMessageBox(MessageBoxParameter().setTitle("Lua Error")
                                                        .setMessage(luaException->getMessage())
                                                        .setDefaultButton("O]pen script", newCallback(this, &LuaErrorHandler::handleOpenStringButton))
                                                        .setCancelButton ("C]ancel",      newCallback(this, &LuaErrorHandler::handleAbortButton)));
            }
            else {
                win->setMessageBox(MessageBoxParameter().setTitle("Lua Error")
                                                        .setMessage(luaException->getMessage()));
                EventDispatcher::getInstance()->deregisterRunningComponent(this);
            }
        }
        else {
            EventDispatcher::getInstance()->deregisterRunningComponent(this);
        }
    }
    
    void handleOpenStringButton()
    {
        if (luaException->hasScriptBytes())
        {
            TextData::Ptr textData = TextData::create();
            String        fileName = String() << System::getInstance()->getHomeDirectory() << "/Untitled.lua";
            
            ByteBuffer buffer; 
            buffer.takeOver(luaException->getPtrToScriptBytes());

            GlobalConfig::LanguageModeAndEncoding result = GlobalConfig::getInstance()
                                                           ->getLanguageModeAndEncodingForFileNameAndContent
                                                           (
                                                             fileName, 
                                                             &buffer
                                                           );
            textData->takeOverBuffer(result.encoding, &buffer);
            textData->setPseudoFileName(fileName);
    
            LanguageMode::Ptr languageMode = GlobalConfig::getInstance()->getLanguageModeForFileName(fileName);
            
            if (!languageMode.isValid()) {
                languageMode = GlobalConfig::getInstance()->getDefaultLanguageMode();
            }
            
            HilitedText::Ptr  hilitedText = HilitedText::create(textData, languageMode);
            
            EditorTopWin::Ptr win = EditorTopWin::create(hilitedText);
            win->show();

            if (luaException->getScriptLineNumber() >= 0) {
                win->gotoLineNumber(luaException->getScriptLineNumber());
            }


            win->setMessageBox(MessageBoxParameter().setTitle("Lua Error")
                                                    .setMessage(luaException->getMessage()));
        }
        EventDispatcher::getInstance()->deregisterRunningComponent(this);
    }
    
    void handleAbortButton()
    {
        EventDispatcher::getInstance()->deregisterRunningComponent(this);
    }
    
    ExceptionLuaInterface::Ptr luaException;
    Callback<const MessageBoxParameter&>::Ptr messageBoxInvoker;
};

} // namespace LucED

#endif // LUA_ERROR_HANDLER_HPP
