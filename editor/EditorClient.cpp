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
        void openFile(int numberOfWindows, const String& fileName)
        {}
    };

} // anonymous namespace


void EditorClient::startWithCommandline(Commandline::Ptr commandline)
{
    isStarted = true;
    wasCommandSet = false;
    
    if (commandline->getLength() > 0)
    {
        CommandlineInterpreter<DoNothingActor> commandInterpreter;
        commandInterpreter.doCommandline(commandline); // check and transform parameters
        
        String instanceName;
        
        if (commandInterpreter.hasInstanceName()) {
            instanceName = commandInterpreter.getInstanceName();
        } else {
            const char* ptr = ::getenv("LUCED_INSTANCE");
            if (ptr != NULL) {
                instanceName = ptr;
            }
        }

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

        }
    }
    
    if (!wasCommandSet)
    {
        EventDispatcher::getInstance()->requestProgramTermination();
        isServerStartupNeededFlag = true;
    }
}
