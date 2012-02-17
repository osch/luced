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
                                          Callback<const MessageBoxParameter&>::Ptr messageBoxInvoker,
                                          Callback<>::Ptr                           catchedExceptionHandler)
    {
        Ptr ptr(new LuaErrorHandler(luaException, messageBoxInvoker, catchedExceptionHandler));
        EventDispatcher::getInstance()->registerRunningComponent(ptr);
        ptr->startMessageBox();
        return ptr;
    }

private:
    LuaErrorHandler(ExceptionLuaInterface::Ptr                luaException,
                    Callback<const MessageBoxParameter&>::Ptr messageBoxInvoker,
                    Callback<>::Ptr                           catchedExceptionHandler)
        : luaException(luaException),
          messageBoxInvoker(messageBoxInvoker),
          catchedExceptionHandler(catchedExceptionHandler)
    {}
    
    void startMessageBox()
    {
        try
        {
            LuaStackTrace::Ptr stackTrace = luaException->getLuaStackTrace();
            
            entry = stackTrace->findFirstExternalFileEntry();
            if (entry.isValid())
            {
                messageBoxInvoker->call(MessageBoxParameter().setTitle("Lua Error")
                                                             .setMessage(String() << "Lua Error in file '" << entry->getFileName() << "'")
                                                             .setDefaultButton("O]pen file", newCallback(this, &LuaErrorHandler::handleOpenFileButton))
                                                             .setCancelButton ("C]ancel",    newCallback(this, &LuaErrorHandler::handleAbortButton)));
            }
            else
            {
                entry = stackTrace->findFirstScriptBytesEntry();
                if (entry.isValid())
                {
                    messageBoxInvoker->call(MessageBoxParameter().setTitle("Lua Error")
                                                                 .setMessage(luaException->getMessage())
                                                                 .setDefaultButton("O]pen script", newCallback(this, &LuaErrorHandler::handleOpenStringButton))
                                                                 .setCancelButton ("C]ancel",      newCallback(this, &LuaErrorHandler::handleAbortButton)));
                }
                else
                {
                    entry = stackTrace->findFirstBuiltinFileEntry();
                    if (entry.isValid())
                    {
                        messageBoxInvoker->call(MessageBoxParameter().setTitle("Internal Lua Error")
                                                                     .setMessage(String() << "Lua Error in internal file '"
                                                                                          << entry->getFileName()
                                                                                          << "', line " << entry->getLineNumber()
                                                                                          << ": "
                                                                                          << luaException->getMessage()));
                    }
                    else {
                        messageBoxInvoker->call(MessageBoxParameter().setTitle("Internal Lua Error")
                                                                     .setMessage(String() << "Internal Lua Error: "
                                                                                          << luaException->getMessage()));
                    }
                    EventDispatcher::getInstance()->deregisterRunningComponent(this);
                }
            }
        }
        catch (...) {
            handleCatchedException();
        }
    }
    
    void handleOpenFileButton()
    {
        try
        {
            if (entry.isValid() && entry->hasFileName() && !entry->isBuiltinFile())
            {
                EditorTopWin::Ptr win;
                
                RawPtr<TopWinList> topWins = TopWinList::getInstance();
                String fileName = File(entry->getFileName()).getAbsoluteNameWithResolvedLinks();
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
                    String        fileName = entry->getFileName();
                    
                    textData->loadFile(fileName);
    
                    LanguageMode::Ptr languageMode = GlobalConfig::getInstance()->getLanguageModeForFileName(fileName);
                    
                    if (!languageMode.isValid()) {
                        languageMode = GlobalConfig::getInstance()->getDefaultLanguageMode();
                    }
                    
                    HilitedText::Ptr  hilitedText = HilitedText::create(textData, languageMode);
                    
                    win = EditorTopWin::create(hilitedText);
                    win->show();
                }
                if (entry->getLineNumber() >= 0) {
                    win->gotoLineNumber(entry->getLineNumber());
                }
                int i = entry->getEntryIndex();
                LuaStackTrace::Ptr stackTrace = luaException->getLuaStackTrace();
                if (i > 0 && stackTrace->getEntry(i - 1)->hasScriptBytes()) {
                    entry = stackTrace->getEntry(i - 1);
                    win->setMessageBox(MessageBoxParameter().setTitle("Lua Error")
                                                            .setMessage(luaException->getMessage())
                                                            .setDefaultButton("O]pen script", newCallback(this, &LuaErrorHandler::handleOpenStringButton))
                                                            .setCancelButton ("C]ancel",      newCallback(this, &LuaErrorHandler::handleAbortButton)));
                }
                else {
                    entry = Null;
                    win->setMessageBox(MessageBoxParameter().setTitle("Lua Error")
                                                            .setMessage(luaException->getMessage()));
                    EventDispatcher::getInstance()->deregisterRunningComponent(this);
                }
            }
            else {
                EventDispatcher::getInstance()->deregisterRunningComponent(this);
            }
        }
        catch (...) {
            handleCatchedException();
        }
    }
    
    void handleOpenStringButton()
    {
        try
        {
            if (entry.isValid() && entry->hasScriptBytes())
            {
                TextData::Ptr textData = TextData::create();
                String        fileName = String() << System::getInstance()->getHomeDirectory() << "/Untitled.lua";
                
                ByteBuffer buffer; 
                buffer.takeOver(entry->getPtrToScriptBytes());
    
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
    
                if (entry->getLineNumber() >= 0) {
                    win->gotoLineNumber(entry->getLineNumber());
                }
                entry = Null;
                win->setMessageBox(MessageBoxParameter().setTitle("Lua Error")
                                                        .setMessage(luaException->getMessage()));
            }
            EventDispatcher::getInstance()->deregisterRunningComponent(this);
        }
        catch (...) {
            handleCatchedException();
        }
    }
    
    void handleAbortButton()
    {
        EventDispatcher::getInstance()->deregisterRunningComponent(this);
    }
    
    void handleCatchedException()
    {
        EventDispatcher::getInstance()->deregisterRunningComponent(this);
        catchedExceptionHandler->call();
    }
    
    ExceptionLuaInterface::Ptr                luaException;
    Callback<const MessageBoxParameter&>::Ptr messageBoxInvoker;
    Callback<>::Ptr                           catchedExceptionHandler;
    LuaStackTrace::Entry::Ptr                 entry;
};

} // namespace LucED

#endif // LUA_ERROR_HANDLER_HPP
