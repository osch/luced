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


#ifndef FILE_OPENER_HPP
#define FILE_OPENER_HPP

#include "HeapObject.hpp"
#include "HeapObjectArray.hpp"
#include "RunningComponent.hpp"
#include "EventDispatcher.hpp"
#include "OwningPtr.hpp"
#include "EditorTopWin.hpp"
#include "ConfigException.hpp"

namespace LucED
{

class FileOpener : public RunningComponent
{
public:
    typedef LucED::OwningPtr<FileOpener> OwningPtr;
    typedef LucED::WeakPtr  <FileOpener> WeakPtr;

    struct FileParameter
    {
        FileParameter(int numberOfWindows, const String& fileName, const String& encoding = "")
            : numberOfWindows(numberOfWindows), fileName(fileName), encoding(encoding)
        {}
        int    numberOfWindows;
        String fileName;
        String encoding;
    };

    typedef HeapObjectArray<FileParameter> ParameterList;

    static WeakPtr start(ParameterList::Ptr              fileParameterList,
                         ConfigException::ErrorList::Ptr errorList = ConfigException::ErrorList::Ptr())
    {
        OwningPtr ptr(new FileOpener(fileParameterList, errorList));
        EventDispatcher::getInstance()->registerRunningComponent(ptr);
        ptr->openFiles();
        return ptr;
    }

    static WeakPtr start(String fileName)
    {
        ParameterList::Ptr pars = ParameterList::create();
        pars->append(FileParameter(1, fileName));
        return start(pars);
    }

private:
    friend class EditorServer;

    FileOpener(ParameterList::Ptr              fileParameterList,
               ConfigException::ErrorList::Ptr errorList)

        : isWaitingForMessageBox(false),
          fileParameterList(fileParameterList),
          configErrorList(errorList),
          numberOfRaisedWindows(0)
    {}
    void handleSkipFileButton();
    void handleCreateFileButton();
    void handleAbortButton();
    void openFiles();
    void openConfigFiles();

    bool isWaitingForMessageBox;
    
    ParameterList::Ptr fileParameterList;
    ConfigException::ErrorList::Ptr configErrorList;
    
    EditorTopWin::Ptr lastTopWin;
    int numberOfRaisedWindows;
};

} // namespace LucED

#endif // FILE_OPENER_HPP
