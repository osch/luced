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

#ifndef EDITORCLIENT_H
#define EDITORCLIENT_H

#include "String.h"

#include "HeapObject.h"
#include "SingletonInstance.h"
#include "GuiRootProperty.h"
#include "HeapObjectArray.h"

namespace LucED
{



class EditorClient : public HeapObject
{
public:
    static EditorClient* getInstance() {
        return instance.getPtr();
    }
    
    ~EditorClient();

    void startWithCommandline(HeapObjectArray<String>::Ptr commandline);
    
private:
    friend class SingletonInstance<EditorClient>;
    static SingletonInstance<EditorClient> instance;

    EditorClient();

    void processEventForCommandProperty(XEvent* event);
    
    bool isStarted;
    bool wasCommandSet;
    GuiRootProperty serverProperty;
    GuiRootProperty commandProperty;
};

} // namespace LucED

#endif // EDITORCLIENT_H
