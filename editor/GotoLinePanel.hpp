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

#ifndef GOTO_LINE_PANEL_HPP
#define GOTO_LINE_PANEL_HPP

#include "DialogPanel.hpp"
#include "Button.hpp"
#include "CheckBox.hpp"
#include "TextEditorWidget.hpp"
#include "SingleLineEditField.hpp"

namespace LucED
{

class GotoLinePanel : public DialogPanel
{
public:
    typedef OwningPtr<GotoLinePanel> Ptr;

    static Ptr create(TextEditorWidget* editorWidget,
                      Callback<>::Ptr   panelCloser)
    {
        return Ptr(new GotoLinePanel(editorWidget, panelCloser));
    }

    virtual void show();
    
private:
    GotoLinePanel(TextEditorWidget* editorWidget, 
                  Callback<>::Ptr   panelCloser);

    void handleButtonPressed(Button* button, Button::ActivationVariant variant);

    void filterInsert(const byte** buffer, long* length);

    Button::Ptr gotoButton;
    Button::Ptr cancelButton;
    SingleLineEditField::Ptr editField;
  
    WeakPtr<TextEditorWidget> editorWidget;
    ByteArray filterBuffer;
};

} // namespace LucED

#endif // GOTO_LINE_PANEL_HPP
