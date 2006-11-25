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

#ifndef FINDPANEL_H
#define FINDPANEL_H

#include "DialogPanel.h"
#include "Button.h"
#include "CheckBox.h"
#include "TextEditorWidget.h"
#include "SingleLineEditField.h"

namespace LucED {

class FindPanel : public DialogPanel
{
public:
    typedef OwningPtr<FindPanel> Ptr;

    static Ptr create(GuiWidget* parent, TextEditorWidget* editorWidget) {
        return Ptr(new FindPanel(parent, editorWidget));
    }
    
    virtual void treatFocusIn();
    
private:
    FindPanel(GuiWidget* parent, TextEditorWidget* editorWidget);

    void handleButtonPressed(Button* button);


    WeakPtr<TextEditorWidget> editorWidget;

    SingleLineEditField::Ptr editField;
    Button::Ptr findNextButton;
    Button::Ptr findPrevButton;
    Button::Ptr cancelButton;
    Button::Ptr goBackButton;
    CheckBox::Ptr ignoreCaseCheckBox;
    CheckBox::Ptr wholeWordCheckBox;
    CheckBox::Ptr regularExprCheckBox;
};

} // namespace LucED

#endif // FINDPANEL_H
