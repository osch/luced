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

#ifndef COMMANDLINEINTERPRETER_H
#define COMMANDLINEINTERPRETER_H

#include "String.hpp"
#include "File.hpp"
#include "HeapObjectArray.hpp"
#include "CommandlineException.hpp"
#include "FileOpener.hpp"

namespace LucED
{

class CommandlineInterpreter
{
public:

    typedef HeapObjectArray<String> Commandline;

    CommandlineInterpreter()
        : hasInstanceNameFlag(false),
          hasCloneDefaultConfigFlag(false),
          noServerFlag(false),
          fileParameterList(HeapObjectArray<FileOpener::FileParameter>::create())
    {}

    void doCommandline(Commandline::Ptr commandline)
    {
        const int argc = commandline->getLength();
        
        for (int i = 0; i < argc; ++i)
        {
            String  fileName;
            int     numberOfWindowsForThisFile = -1;
            String  encodingForThisFile;
            
            while (i < argc && commandline->get(i).startsWith("-"))
            {
                // Parameter is a command option
            
                if (commandline->get(i) == "-i" || commandline->get(i) == "--instance")
                {
                    i += 1;
                    if (i >= argc) {
                        throw CommandlineException("Command option -i needs additional argument 'instance name'.");
                    }
                    instanceName = commandline->get(i);

                    hasInstanceNameFlag = true;
                }
                else if (commandline->get(i) == "-w")
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
                else if (commandline->get(i) == "-e" || commandline->get(i) == "--encoding")
                {
                    i += 1;

                    if (i >= argc) {
                        throw CommandlineException("Command option -e needs additional argument.");
                    }
                    
                    encodingForThisFile = commandline->get(i);
                }
                else if (commandline->get(i) == "-f" || commandline->get(i) == "--file")
                {
                    i += 1;

                    if (i >= argc) {
                        throw CommandlineException("Command option -f needs additional argument.");
                    }

                    break; // next argument is filename
                }
                else if (commandline->get(i) == "-cdc" || commandline->get(i) == "--clone-default-config")
                {
                    hasCloneDefaultConfigFlag = true;
                }
                else if (commandline->get(i) == "-ns" || commandline->get(i) == "--no-server")
                {
                    noServerFlag = true;
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
                
                fileName = commandline->get(i);

                fileName = File(fileName).getAbsoluteName();
                commandline->set(i, fileName); // replace with absolute filename in command array
                fileParameterList->append(FileOpener::FileParameter(numberOfWindowsForThisFile, fileName, encodingForThisFile));
                numberOfWindowsForThisFile = -1;
            }
            else if (numberOfWindowsForThisFile > 0)
            {
                throw CommandlineException("Command needs filename parameter.");
            }
        }
    }
    
    String getInstanceName() const {
        return instanceName;
    }
    bool hasInstanceName() const {
        return hasInstanceNameFlag;
    }
    bool hasCloneDefaultConfig() const {
        return hasCloneDefaultConfigFlag;
    }
    
    bool hasNoServerFlag() const {
        return noServerFlag;
    }

    HeapObjectArray<FileOpener::FileParameter>::Ptr getFileParameterList() {
        return fileParameterList;
    }
private:
    String instanceName;
    bool hasInstanceNameFlag;
    bool hasCloneDefaultConfigFlag;
    bool noServerFlag;
    HeapObjectArray<FileOpener::FileParameter>::Ptr fileParameterList;
};

} // namespace LucED

#endif // COMMANDLINEINTERPRETER_H
