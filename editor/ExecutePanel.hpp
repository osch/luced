/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2011 Oliver Schmidt, oliver at luced dot de
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

#ifndef EXECUTE_PANEL_HPP
#define EXECUTE_PANEL_HPP

#include "RawPtr.hpp"
#include "DialogPanel.hpp"
#include "Button.hpp"
#include "CheckBox.hpp"
#include "TextEditorWidget.hpp"
#include "SingleLineEditField.hpp"
#include "ProgramExecutor.hpp"
#include "CheckBox.hpp"

namespace LucED
{

class ExecutePanel : public DialogPanel
{
public:
    typedef OwningPtr<ExecutePanel> Ptr;

    static Ptr create(RawPtr<TextEditorWidget> editorWidget,
                      Callback<>::Ptr          panelCloser)
    {
        return Ptr(new ExecutePanel(editorWidget, panelCloser));
    }

    virtual void show();
    
private:
    ExecutePanel(RawPtr<TextEditorWidget> editorWidget, 
                 Callback<>::Ptr          panelCloser);

    void handleButtonPressed(Button* button, Button::ActivationVariant variant);
    void handleCheckBoxPressed(CheckBox* checkBox);
    
    void handleExecutionResult(ProgramExecutor::Result result);

    Button::Ptr executeButton;
    Button::Ptr cancelButton;
    
    CheckBox::Ptr selectionInputCheckBox;
    CheckBox::Ptr wholeFileInputCheckBox;
    CheckBox::Ptr replaceOutputCheckBox;
    CheckBox::Ptr newWindowOutputCheckBox;
    
    SingleLineEditField::Ptr editField;
    
    WeakPtr<TextEditorWidget> editorWidget;
};

} // namespace LucED

#endif // EXECUTE_PANEL_HPP
