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

#ifndef SAVE_AS_PANEL_HPP
#define SAVE_AS_PANEL_HPP

#include "DialogPanel.hpp"
#include "Button.hpp"
#include "TextEditorWidget.hpp"
#include "SingleLineEditField.hpp"

namespace LucED {

class SaveAsPanel : public DialogPanel
{
public:
    typedef OwningPtr<SaveAsPanel> Ptr;

    static Ptr create(GuiWidget* parent, TextEditorWidget* editorWidget) {
        return Ptr(new SaveAsPanel(parent, editorWidget));
    }
    
    virtual void treatFocusIn();
    
    virtual void show();
    
    void setSaveCallback(const Callback0& saveCallback) {
        this->saveCallback = saveCallback;
    }
    
private:
    SaveAsPanel(GuiWidget* parent, TextEditorWidget* editorWidget);

    void handleButtonPressed(Button* button);

    Button::Ptr saveButton;
    Button::Ptr cancelButton;
    SingleLineEditField::Ptr editField;
  
    WeakPtr<TextEditorWidget> editorWidget;
    Callback0 saveCallback;
};

} // namespace LucED

#endif // SAVE_AS_PANEL_HPP
