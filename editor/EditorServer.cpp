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

#include <stdio.h>

#include <string>

#include "EditorServer.h"
#include "EventDispatcher.h"
#include "ClientServerUtil.h"
#include "CommandlineInterpreter.h"
#include "TextStyle.h"
#include "GlobalConfig.h"
#include "EditorTopWin.h"
#include "HeapObjectArray.h"

using namespace LucED;
using std::string;

SingletonInstance<EditorServer> EditorServer::instance;

EditorServer::~EditorServer()
{
    if (isStarted) {
        serverProperty.remove();
        isStarted = false;
    }
}

void EditorServer::startWithCommandline(HeapObjectArray<string>::Ptr commandline)
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
    processCommandline(commandline);
}


void EditorServer::processEventForServerProperty(XEvent* event)
{
    if (event->xproperty.state == PropertyDelete)
    {
        printf(" *********** Event: delete\n");
        serverProperty.setValue("running");
    }
    else if (event->xproperty.state == PropertyNewValue)
    {
        printf(" *********** Event: newValue\n");
    }
    else
    {
        printf(" *********** Event: unknown\n");
    }
}

void EditorServer::processEventForCommandProperty(XEvent* event)
{
    if (event->xproperty.state == PropertyDelete)
    {
        printf(" *********** Event: command delete\n");
    }
    else if (event->xproperty.state == PropertyNewValue)
    {
        string commandline = commandProperty.getValueAndRemove();
        printf(" *********** Event: command newValue <%s>\n", commandline.c_str());
        if (commandline.length() > 0) {
            processCommandline(ClientServerUtil::unquoteCommandline(commandline));
        }
    }
    else
    {
        printf(" *********** Event: command unknown\n");
    }
}

namespace // anonymous namespace
{
    class Actor
    {
    public:
        Actor()
            : openedWindows(HeapObjectArray<TopWin::Ptr>::create()),
              textStyles(GlobalConfig::getInstance()->getTextStyles())
        {}

        void openFile(int numberOfWindows, const string& fileName)
        {
            LanguageMode::Ptr languageMode = GlobalConfig::getInstance()->getLanguageModeForFileName(fileName);
            TextData::Ptr     textData     = TextData::create();
            HilitedText::Ptr  hilitedText  = HilitedText::create(textData, languageMode);

            textData->loadFile(fileName);

            for (int i = 0; i < numberOfWindows; ++i)
            {
                EditorTopWin::Ptr win = EditorTopWin::create(textStyles, hilitedText);
                openedWindows->append(win);
            }
        }

        void showAllOpenedWindows()
        {
            for (int i = 0; i < openedWindows->getLength(); ++i) {
                openedWindows->get(i)->show();
            }
        }

    private:
        HeapObjectArray<TopWin::Ptr>::Ptr openedWindows;
        TextStyles::Ptr                   textStyles;
    };

} // anonymous namespace

void EditorServer::processCommandline(HeapObjectArray<string>::Ptr commandline)
{
    ASSERT(isStarted);

    if (commandline->getLength() > 0) {
        CommandlineInterpreter<Actor> commandInterpreter;
        commandInterpreter.doCommandline(commandline);
        commandInterpreter.getActor().showAllOpenedWindows();
    }
}

