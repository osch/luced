/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#include "EditorClient.h"
#include "GuiRootProperty.h"
#include "ClientServerUtil.h"
#include "CommandlineInterpreter.h"
#include "EventDispatcher.h"

using namespace LucED;

SingletonInstance<EditorClient> EditorClient::instance;

EditorClient::EditorClient()
    : isStarted(false),
      wasCommandSet(false)
{
    serverProperty = GuiRootProperty(ClientServerUtil::getDefaultServerRunningProperty());
    commandProperty = GuiRootProperty(ClientServerUtil::getDefaultServerCommandProperty());

    EventDispatcher::getInstance()
                     ->registerEventReceiverForRootProperty(commandProperty, 
                                                            Callback1<XEvent*>(this, &EditorClient::processEventForCommandProperty));
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
    }
    else if (event->xproperty.state == PropertyNewValue)
    {
    }
}


namespace // anonymous namespace
{
    class DoNothingActor
    {
    public:
        void openFile(int numberOfWindows, const string& fileName)
        {}
    };

} // anonymous namespace


void EditorClient::startWithCommandline(HeapObjectArray<string>::Ptr commandline)
{
    isStarted = true;
    wasCommandSet = false;
    
    if (commandline->getLength() > 0)
    {
        CommandlineInterpreter<DoNothingActor> commandInterpreter;
        commandInterpreter.doCommandline(commandline); // check and transform parameters

        if (serverProperty.exists())
        {
            commandProperty.setValue(ClientServerUtil::quoteCommandline(commandline));
            wasCommandSet = true;
        }
    }
    
    if (!wasCommandSet)
    {
        EventDispatcher::getInstance()->requestProgramTermination();
    }
}


