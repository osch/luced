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

#include "EditorClient.hpp"
#include "GuiRootProperty.hpp"
#include "ClientServerUtil.hpp"
#include "CommandlineInterpreter.hpp"
#include "EventDispatcher.hpp"
#include "DefaultConfig.hpp"
#include "ProgramName.hpp"
#include "FileOpener.hpp"
#include "GlobalConfig.hpp"

using namespace LucED;

SingletonInstance<EditorClient> EditorClient::instance;

EditorClient::EditorClient()
    : isStarted(false),
      wasCommandSet(false),
      isServerStartupNeededFlag(true)
{
    GuiRoot::getInstance(); // assure that GuiRoot instance lives longer than EditorClient
}

EditorClient::~EditorClient()
{
    if (isStarted)
    {
        if (wasCommandSet) {
            commandProperty.remove();
            wasCommandSet = false;
        }
        isStarted = false;
    }
}



void EditorClient::processEventForCommandProperty(XEvent* event)
{
    if (event->xproperty.state == PropertyDelete)
    {
        EventDispatcher::getInstance()->requestProgramTermination();
        isServerStartupNeededFlag = false;
    }
    else if (event->xproperty.state == PropertyNewValue)
    {
    }
}

void EditorClient::waitingForServerFailed()
{
        EventDispatcher::getInstance()->requestProgramTermination();
        isServerStartupNeededFlag = true;
}


namespace // anonymous namespace
{
    class DoNothingActor
    {
    public:
        void openFile(int numberOfWindows, const String& fileName, const String& encoding)
        {}
    };

} // anonymous namespace


void EditorClient::startWithCommandline(Commandline::Ptr commandline)
{
    isStarted = true;
    wasCommandSet = false;
    bool wasFileOpenerStarted = false;
    
    if (commandline->getLength() > 0)
    {
        CommandlineInterpreter commandInterpreter;
        commandInterpreter.doCommandline(commandline); // check and transform parameters
        
        if (commandInterpreter.hasCloneDefaultConfig())
        {
            File dirName = DefaultConfig::writeCopyOfDefaultConfigFiles();
            printf("[%s]: Cloned default config package to %s\n",
                   ProgramName::get().c_str(),
                   dirName.getAbsoluteName().toCString());
        }

        String instanceName;

        if (commandInterpreter.hasInstanceName()) {
            instanceName = commandInterpreter.getInstanceName();
        } else {
            const char* fromEnv = ::getenv("LUCED_INSTANCE");
            if (fromEnv != NULL) {
                instanceName = fromEnv;
            }
        }

        if (commandInterpreter.hasNoServerFlag()) 
        {
            GuiRoot::getInstance()->setInstanceName(instanceName);
            GlobalConfig::getInstance()->readConfig();
            FileOpener::start(commandInterpreter.getFileParameterList());
            isServerStartupNeededFlag = false;
            wasFileOpenerStarted = true;
        }
        else
        {
            serverProperty  = ClientServerUtil::getServerRunningProperty(instanceName);
            commandProperty = ClientServerUtil::getServerCommandProperty(instanceName);
    
            if (serverProperty.exists())
            {
                commandProperty.setValue(commandline->toQuotedString());
                wasCommandSet = true;
    
                EventDispatcher::getInstance()
                         ->registerEventReceiverForRootProperty(commandProperty, 
                                                                newCallback(this, &EditorClient::processEventForCommandProperty));
    
                EventDispatcher::getInstance()->registerTimerCallback(
                        Seconds(3), MicroSeconds(0),
                        newCallback(this, &EditorClient::waitingForServerFailed));
    
                isServerStartupNeededFlag = false;
            }
            else
            {
                isServerStartupNeededFlag = true;
            }
        }
    }
    else {
        isServerStartupNeededFlag = false;
    }
    
    if (!wasCommandSet && !wasFileOpenerStarted)
    {
        EventDispatcher::getInstance()->requestProgramTermination();
    }
}
