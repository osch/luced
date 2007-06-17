/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2007 Oliver Schmidt, oliver at luced dot de
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


#include "String.hpp"

#include "EditorServer.hpp"
#include "EventDispatcher.hpp"
#include "ClientServerUtil.hpp"
#include "CommandlineInterpreter.hpp"
#include "TextStyle.hpp"
#include "GlobalConfig.hpp"
#include "EditorTopWin.hpp"
#include "HeapObjectArray.hpp"
#include "TopWinList.hpp"
#include "FileException.hpp"
#include "FileOpener.hpp"
#include "ConfigErrorHandler.hpp"

using namespace LucED;


SingletonInstance<EditorServer> EditorServer::instance;

EditorServer::~EditorServer()
{
    if (isStarted) {
        serverProperty.remove();
        isStarted = false;
    }
}

void EditorServer::startWithCommandlineAndErrorList(HeapObjectArray<String>::Ptr    commandline,
                                                    ConfigException::ErrorList::Ptr errorList)
{
    serverProperty = GuiRootProperty(ClientServerUtil::getDefaultServerRunningProperty());
    serverProperty.setValue("running");
    EventDispatcher::getInstance()
                     ->registerEventReceiverForRootProperty(serverProperty, 
                                                            Callback1<XEvent*>(this, &EditorServer::processEventForServerProperty));

    commandProperty = GuiRootProperty(ClientServerUtil::getDefaultServerCommandProperty());
    commandProperty.remove();
    EventDispatcher::getInstance()
                     ->registerEventReceiverForRootProperty(commandProperty, 
                                                            Callback1<XEvent*>(this, &EditorServer::processEventForCommandProperty));
    isStarted = true;
    processCommandline(commandline, errorList);
}


void EditorServer::processEventForServerProperty(XEvent* event)
{
    if (event->xproperty.state == PropertyDelete)
    {
//        printf(" *********** Event: delete\n");
        serverProperty.setValue("running");
    }
    else if (event->xproperty.state == PropertyNewValue)
    {
//        printf(" *********** Event: newValue\n");
    }
    else
    {
//        printf(" *********** Event: unknown\n");
    }
}

void EditorServer::processEventForCommandProperty(XEvent* event)
{
    if (event->xproperty.state == PropertyDelete)
    {
//        printf(" *********** Event: command delete\n");
    }
    else if (event->xproperty.state == PropertyNewValue)
    {
        String commandline = commandProperty.getValueAndRemove();
//        printf(" *********** Event: command newValue <%s>\n", commandline.toCString());
        if (commandline.getLength() > 0) {
            processCommandline(ClientServerUtil::unquoteCommandline(commandline));
        }
    }
    else
    {
//        printf(" *********** Event: command unknown\n");
    }
}

namespace // anonymous namespace
{


    class Actor
    {
    public:
        Actor()
            : numberAndFileList(HeapObjectArray<FileOpener::NumberAndFileName>::create())
        {}

        void openFile(int numberOfWindows, const String& fileName)
        {
            numberAndFileList->append(FileOpener::NumberAndFileName(numberOfWindows, fileName));
        }
        

        HeapObjectArray<FileOpener::NumberAndFileName>::Ptr getNumberAndFileList() {
            return numberAndFileList;
        }
        
    private:
        HeapObjectArray<FileOpener::NumberAndFileName>::Ptr numberAndFileList;
        
        HeapObjectArray<TopWin::Ptr>::Ptr openedWindows;
    };

} // anonymous namespace



void EditorServer::processCommandline(HeapObjectArray<String>::Ptr commandline,
                                      ConfigException::ErrorList::Ptr errorList)
{
    ASSERT(isStarted);

    if (commandline->getLength() > 0)
    {
        CommandlineInterpreter<Actor> commandInterpreter;
        commandInterpreter.doCommandline(commandline);

        if (errorList.isValid() && errorList->getLength() > 0)
        {
            ConfigErrorHandler::start(errorList, commandInterpreter.getActor().getNumberAndFileList());
        }
        else
        {
            FileOpener::start(commandInterpreter.getActor().getNumberAndFileList());
        }
    }
}

