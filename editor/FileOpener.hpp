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

namespace LucED
{

class FileOpener : public RunningComponent
{
public:
    typedef LucED::OwningPtr<FileOpener> OwningPtr;
    typedef LucED::WeakPtr  <FileOpener> WeakPtr;

    struct NumberAndFileName
    {
        NumberAndFileName(int numberOfWindows, String fileName)
            : numberOfWindows(numberOfWindows), fileName(fileName)
        {}
        int    numberOfWindows;
        String fileName;
    };

    static WeakPtr start(HeapObjectArray<NumberAndFileName>::Ptr numberAndFileList) {
        OwningPtr ptr(new FileOpener(numberAndFileList));
        EventDispatcher::getInstance()->registerRunningComponent(ptr);
        ptr->openFiles();
        return ptr;
    }

private:
    friend class EditorServer;

    FileOpener(HeapObjectArray<NumberAndFileName>::Ptr numberAndFileList)
        : isWaitingForMessageBox(false),
          numberAndFileList(numberAndFileList),
          numberOfRaisedWindows(0)
    {}
    void handleSkipFileButton();
    void handleCreateFileButton();
    void handleAbortButton();
    void openFiles();

    bool isWaitingForMessageBox;
    HeapObjectArray<NumberAndFileName>::Ptr numberAndFileList;
    EditorTopWin::Ptr lastTopWin;
    int numberOfRaisedWindows;
    String lastErrorMessage;
};

} // namespace LucED

#endif // FILE_OPENER_HPP
