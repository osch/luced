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

#ifndef EDITOR_SERVER_HPP
#define EDITOR_SERVER_HPP

#include "String.hpp"

#include "HeapObject.hpp"
#include "SingletonInstance.hpp"
#include "GuiRootProperty.hpp"
#include "HeapObjectArray.hpp"
#include "ConfigException.hpp"
#include "WeakPtr.hpp"
#include "RawPtr.hpp"
#include "Commandline.hpp"

namespace LucED
{

class EditorServer : public HeapObject
{
public:

    typedef WeakPtr<EditorServer> Ptr;
    
    static EditorServer* getInstance() {
        return instance.getPtr();
    }
    
    ~EditorServer();
    
    void startWithCommandlineAndErrorList(Commandline::Ptr                commandline,
                                          ConfigException::ErrorList::Ptr errorList);
    
    void startWithCommandline(Commandline::Ptr commandLine)
    {
        startWithCommandlineAndErrorList(commandLine, ConfigException::ErrorList::Ptr());
    }
    
    String getInstanceName() const {
        return instanceName;
    }
    
private:
    friend class SingletonInstance<EditorServer>;
    static SingletonInstance<EditorServer> instance;
  
    EditorServer();

    void processEventForServerProperty(XEvent* event);
    void processEventForCommandProperty(XEvent* event);
    void processCommandline(Commandline::Ptr  commandline, 
                            bool              isStarting,
                            ConfigException::ErrorList::Ptr errorList    = ConfigException::ErrorList::Ptr());

    bool isStarted;
    GuiRootProperty serverProperty;
    GuiRootProperty commandProperty;
    String instanceName;
};

} // namespace LucED

#endif // EDITOR_SERVER_HPP
