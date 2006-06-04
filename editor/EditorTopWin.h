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

#ifndef EDITORTOPWIN_H
#define EDITORTOPWIN_H

#include "TopWin.h"
#include "EventDispatcher.h"
#include "ScrollBar.h"
#include "TextEditorWidget.h"
#include "TextData.h"
#include "TextStyle.h"
#include "HilitingBuffer.h"
#include "StatusLine.h"
#include "GuiLayoutColumn.h"
#include "OwningPtr.h"
#include "TopWinList.h"

namespace LucED {

class EditorTopWin : public TopWin
{
public:
    typedef WeakPtr<EditorTopWin> Ptr;
    
    static EditorTopWin::Ptr create(
            TextData::Ptr textData, TextStyles::Ptr textStyles, HilitingBuffer::Ptr hilitingBuffer,
            int x, int y, unsigned int width, unsigned int height)
    {
        return transferOwnershipToTopWinList(
                new EditorTopWin(textData, textStyles, hilitingBuffer, x, y, width, height));
    }
    
    virtual void requestCloseWindow() {
        getEventDispatcher()->requestProgramTermination();
    }

    virtual bool processEvent(const XEvent *event);
    virtual bool processKeyboardEvent(const XEvent *event);

    virtual void treatNewWindowPosition(Position newPosition);
    virtual void treatFocusIn();
    virtual void treatFocusOut();
    
private:
    EditorTopWin(
            TextData::Ptr textData, TextStyles::Ptr textStyles, HilitingBuffer::Ptr hilitingBuffer,
            int x, int y, unsigned int width, unsigned int height);

    TextEditorWidget::Ptr textEditor;
    ScrollBar::Ptr scrollBarH;
    ScrollBar::Ptr scrollBarV;    
    StatusLine::Ptr statusLine;
    GuiLayoutColumn::Ptr layout;
};

} // namespace LucED

#endif // EDITORTOPWIN_H
