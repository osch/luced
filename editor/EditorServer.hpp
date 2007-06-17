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

#ifndef EDITOR_SERVER_HPP
#define EDITOR_SERVER_HPP

#include "String.hpp"

#include "HeapObject.hpp"
#include "SingletonInstance.hpp"
#include "GuiRootProperty.hpp"
#include "HeapObjectArray.hpp"
#include "MessageBox.hpp"
#include "EditorTopWin.hpp"
#include "ConfigException.hpp"

namespace LucED
{

class EditorServer : public HeapObject
{
public:
    
    static EditorServer* getInstance() {
        return instance.getPtr();
    }
    
    ~EditorServer();
    
    void startWithCommandlineAndErrorList(HeapObjectArray<String>::Ptr    commandline,
                                          ConfigException::ErrorList::Ptr errorList);
    
    void startWithCommandline(HeapObjectArray<String>::Ptr commandLine)
    {
        startWithCommandlineAndErrorList(commandLine, ConfigException::ErrorList::Ptr());
    }
    
private:
    friend class SingletonInstance<EditorServer>;
    static SingletonInstance<EditorServer> instance;
  
    EditorServer()
        : isStarted(false)
    {}
    
    void processEventForServerProperty(XEvent* event);
    void processEventForCommandProperty(XEvent* event);
    void processCommandline(HeapObjectArray<String>::Ptr commandline);

    bool isStarted;
    GuiRootProperty serverProperty;
    GuiRootProperty commandProperty;
};

} // namespace LucED

#endif // EDITOR_SERVER_HPP
