/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch@luced.de
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

#include "EditorTopWin.h"
#include "EventDispatcher.h"
#include "TextStyle.h"
#include "SyntaxPatterns.h"
#include "LuaException.h"
#include "GlobalConfig.h"
#include "ConfigException.h"
#include "SingletonKeeper.h"

using namespace LucED;

int main(int argc, char **argv)
{
    try
    {
        SingletonKeeper::Ptr singletonKeeper = SingletonKeeper::create();
        
        string fileName;
        if (argc >= 2) {
            fileName = argv[1];
        }

        GlobalConfig::getInstance()->readConfig("./config");

        TextStyles::Ptr     textStyles     = GlobalConfig::getInstance()->getTextStyles();
        LanguageMode::Ptr   languageMode   = GlobalConfig::getInstance()->getLanguageModeForFileName(fileName);
        
        TextData::Ptr textData = TextData::create();
        Hiliting::Ptr hiliting = Hiliting::create(textData, languageMode);
        HilitingBuffer::Ptr hilitingBuffer1 = HilitingBuffer::create(hiliting);
        HilitingBuffer::Ptr hilitingBuffer2 = HilitingBuffer::create(hiliting);

        EditorTopWin::Ptr win = EditorTopWin::create(textData, textStyles, hilitingBuffer1);
        win->setTitle("Title - Test");
        win->show();

        if (argc >= 2) {
            textData->loadFile(argv[1]);  // TODO: this is all experimental!
        }

        EditorTopWin::Ptr win2 = EditorTopWin::create(textData, textStyles, hilitingBuffer2);
        win2->setTitle("Title - Test2");
        win2->show();
        
        EventDispatcher::getInstance()->doEventLoop();
    }
    catch (LuaException& ex)
    {
        fprintf(stderr, "LuaException: %s\n", ex.getMessage().c_str());
    }
    catch (ConfigException& ex)
    {
        fprintf(stderr, "ConfigException: %s\n", ex.getMessage().c_str());
    }
}
