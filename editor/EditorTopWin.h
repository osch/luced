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
#include "MultiLineEditorWidget.h"
#include "TextData.h"
#include "TextStyle.h"
#include "StatusLine.h"
#include "GuiLayoutColumn.h"
#include "OwningPtr.h"
#include "TopWinList.h"
#include "KeyMapping.h"
#include "GotoLinePanel.h"
#include "FindPanel.h"
#include "MessageBox.h"

namespace LucED {


class EditorTopWin : public TopWin
{
public:
    typedef WeakPtr<EditorTopWin> Ptr;
    
    static EditorTopWin::Ptr create(
            TextStyles::Ptr textStyles, HilitedText::Ptr hilitedText)
    {
        return transferOwnershipTo(
                new EditorTopWin(textStyles, hilitedText),
                TopWinList::getInstance());
    }
    
    virtual ProcessingResult processEvent(const XEvent *event);
    virtual ProcessingResult processKeyboardEvent(const XEvent *event);

    virtual void treatNewWindowPosition(Position newPosition);
    virtual void treatFocusIn();
    virtual void treatFocusOut();
    virtual void show();
    
    virtual void requestCloseFor(GuiWidget* w);
    virtual void requestCloseChildWindow(TopWin *topWin);
    void invokeGotoLinePanel();
    void invokeFindPanel();

private:
    EditorTopWin(TextStyles::Ptr textStyles, HilitedText::Ptr hilitedText);
    void handleEscapeKey();
    void invokePanel(DialogPanel* panel);
    void invokeMessageBox(MessageBoxParameter p);
    
    MultiLineEditorWidget::Ptr textEditor;
    ScrollBar::Ptr scrollBarH;
    ScrollBar::Ptr scrollBarV;    
    StatusLine::Ptr statusLine;
    GuiLayoutColumn::Ptr rootElement;
    KeyMapping keyMapping;
    GotoLinePanel::Ptr gotoLinePanel;
    bool wasNeverShown;

    FindPanel::Ptr findPanel;
    int upperPanelIndex;
    int lowerPanelIndex;
    
    WeakPtr<DialogPanel> invokedPanel;
    MessageBox::Ptr messageBox;
};

} // namespace LucED

#endif // EDITORTOPWIN_H
