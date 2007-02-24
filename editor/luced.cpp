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
        
        GlobalConfig::getInstance()->readConfig("./config");

        TextStyles::Ptr  textStyles     = GlobalConfig::getInstance()->getTextStyles();

        for (int argIndex = 1; argIndex < argc; ++argIndex)
        {
            const char* fileName;
            int         numberOfWindowsForThisFile = 1;
            
            while (argIndex < argc && argv[argIndex][0] == '-' && argv[argIndex][1] != '-')
            {
                // Parameter is a command option
            
                if (strcmp(argv[argIndex], "-n") == 0)
                {
                    argIndex += 1;

                    if (argIndex >= argc) {
                        fprintf(stderr, "Command option -n needs additional argument\n");
                        return 8;
                    }
                    
                    numberOfWindowsForThisFile = atoi(argv[argIndex]);
                    
                    if (numberOfWindowsForThisFile < 1) {
                        fprintf(stderr, "Command option -n needs additional argument number >= 1\n");
                        return 8;
                    }
                    
                    argIndex += 1;
                }
                else
                {
                    fprintf(stderr, "Unknown command option %s\n", argv[argIndex]);
                    return 8;
                }
            }
            if (argIndex < argc)
            {
                // Parameter is a filename

                if (argv[argIndex][0] == '-') {
                    fileName = argv[argIndex] + 1;  // "--fname" means filename == "-fname"
                } else {
                    fileName = argv[argIndex];
                }
                
                LanguageMode::Ptr languageMode = GlobalConfig::getInstance()->getLanguageModeForFileName(fileName);
                TextData::Ptr     textData     = TextData::create();
                HilitedText::Ptr  hilitedText  = HilitedText::create(textData, languageMode);

                textData->loadFile(fileName);

                for (int i = 0; i < numberOfWindowsForThisFile; ++i)
                {
                    EditorTopWin::Ptr win = EditorTopWin::create(textStyles, hilitedText);
                }
            }
            else
            {
                fprintf(stderr, "Command needs filename parameter\n");
                return 8;
            }
        }
        
        TopWinList* topWins = TopWinList::getInstance();

        for (int i = 0; i < topWins->getNumberOfTopWins(); ++i) {
            topWins->getTopWin(i)->show();
        }
        
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
#ifdef DEBUG
    HeapObjectChecker::assertAllCleared();
#endif    
}
