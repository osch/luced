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

#ifndef COMMANDLINEINTERPRETER_H
#define COMMANDLINEINTERPRETER_H

#include "String.h"
#include "File.h"
#include "HeapObjectArray.h"
#include "CommandlineException.h"

namespace LucED
{



template<class Actor> class CommandlineInterpreter
{
public:

    typedef HeapObjectArray<String> Commandline;

    void doCommandline(Commandline::Ptr commandline)
    {
        const int argc = commandline->getLength();
        
        for (int i = 0; i < argc; ++i)
        {
            String  fileName;
            int     numberOfWindowsForThisFile = 1;
            
            while (i < argc && commandline->get(i)[0] == '-' && commandline->get(i)[1] != '-')
            {
                // Parameter is a command option
            
                if (commandline->get(i) == "-w")
                {
                    i += 1;

                    if (i >= argc) {
                        throw CommandlineException("Command option -w needs additional argument.");
                    }
                    
                    numberOfWindowsForThisFile = commandline->get(i).toInt();
                    
                    if (numberOfWindowsForThisFile < 1) {
                        throw CommandlineException("Command option -w needs additional argument number >= 1.");
                    }
                }
                else
                {
                    throw CommandlineException(String() << "Unknown command option '" << commandline->get(i) << "'.");
                }
                i += 1;
            }
            
            if (i < argc)
            {
                // Parameter is a filename

                if (commandline->get(i)[0] == '-') {
                    fileName = commandline->get(i).getTail(1);  // "--fname" means filename == "-fname"
                } else {
                    fileName = commandline->get(i);
                }
                fileName = File(fileName).getAbsoluteFileName();
                commandline->set(i, fileName); // replace with absolute filename in command array
                actor.openFile(numberOfWindowsForThisFile, fileName);
                
            }
            else
            {
                throw CommandlineException("Command needs filename parameter.");
            }
        }
    }
    
    Actor& getActor() {
        return actor;
    }

private:
    Actor actor;
};

} // namespace LucED

#endif // COMMANDLINEINTERPRETER_H
