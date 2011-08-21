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
#include "WindowCloser.hpp"

using namespace LucED;


SingletonInstance<EditorServer> EditorServer::instance;

EditorServer::EditorServer()
    : isStarted(false)
{
    GuiRoot                ::getInstance();  // assure that GuiRoot         instance lives longer than EditorServer
    EventDispatcher        ::getInstance();  // assure that EventDispatcher instance lives longer than EditorServer
}

EditorServer::~EditorServer()
{
    if (isStarted) {
        serverProperty.remove();
        isStarted = false;
    }
}

void EditorServer::startWithCommandlineAndErrorList(Commandline::Ptr                commandline,
                                                    ConfigException::ErrorList::Ptr errorList)
{
    try
    {
        GlobalConfig::getInstance()->readConfig();
    }
    catch (ConfigException& ex) {
        ConfigException::ErrorList::Ptr newErrors = ex.getErrorList();
        if (errorList.isValid()) {
            errorList->appendAll(newErrors);
        } else {
            errorList = newErrors;
        }
    }
    isStarted = true;
    processCommandline(commandline, true, errorList);
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
            processCommandline(Commandline::createFromQuotedString(commandline), false);
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
            : fileParameterList(HeapObjectArray<FileOpener::FileParameter>::create())
        {}

        void openFile(int numberOfWindows, const String& fileName, const String& encoding)
        {
            fileParameterList->append(FileOpener::FileParameter(numberOfWindows, fileName, encoding));
        }
        

        HeapObjectArray<FileOpener::FileParameter>::Ptr getFileParameterList() {
            return fileParameterList;
        }
        
    private:
        HeapObjectArray<FileOpener::FileParameter>::Ptr fileParameterList;
    };

} // anonymous namespace



void EditorServer::processCommandline(Commandline::Ptr commandline,
                                      bool isStarting,
                                      ConfigException::ErrorList::Ptr errorList)
{
    ASSERT(isStarted);

    CommandlineInterpreter commandInterpreter;
    commandInterpreter.doCommandline(commandline);

    if (isStarting)
    {
        String instanceName;
        if (commandInterpreter.hasInstanceName()) {
            instanceName = commandInterpreter.getInstanceName();
        } else {
            const char* fromEnv = ::getenv("LUCED_INSTANCE");
            if (fromEnv != NULL) {
                instanceName = fromEnv;
            }
        }
        if (!GuiRoot::getInstance()->hasInstanceName()) {
            GuiRoot::getInstance()->setInstanceName(instanceName);
        }

        
        serverProperty = ClientServerUtil::getServerRunningProperty(instanceName);
        serverProperty.setValue("running");
        EventDispatcher::getInstance()
                         ->registerEventReceiverForRootProperty(serverProperty, 
                                                                newCallback(this, &EditorServer::processEventForServerProperty));
    
        commandProperty = GuiRootProperty(ClientServerUtil::getServerCommandProperty(instanceName));
        commandProperty.remove();
        EventDispatcher::getInstance()
                         ->registerEventReceiverForRootProperty(commandProperty, 
                                                                newCallback(this, &EditorServer::processEventForCommandProperty));
    }

    if (commandline->getLength() > 0)
    {
        if (errorList.isValid() && errorList->getLength() > 0)
        {
            ConfigErrorHandler::start(errorList, commandInterpreter.getFileParameterList());
        }
        else
        {
            FileOpener::start(commandInterpreter.getFileParameterList());
        }
        if (commandInterpreter.hasQuitServer()) {
            WindowCloser::start();
        }
    }
    else
    {
        EventDispatcher::getInstance()->requestProgramTermination();
    }
}

