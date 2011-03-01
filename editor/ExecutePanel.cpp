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

#include "ExecutePanel.hpp"
#include "GuiLayoutColumn.hpp"
#include "GuiLayoutRow.hpp"
#include "GlobalConfig.hpp"
#include "GuiLayoutSpacer.hpp"
#include "LabelWidget.hpp"
#include "EditorTopWin.hpp"

using namespace LucED;

ExecutePanel::ExecutePanel(RawPtr<TextEditorWidget> editorWidget,
                           Callback<>::Ptr          panelCloser)
    : DialogPanel(panelCloser),
      editorWidget(editorWidget)
{
    executeButton = Button::create("E]xecute");
    cancelButton = Button::create("C]ancel");
   
    this->editField = SingleLineEditField::create(GlobalConfig::getInstance()->getDefaultLanguageMode());
    editField->setDesiredWidthInChars(5, 10, INT_MAX);

    GuiLayoutColumn::Ptr  c0 = GuiLayoutColumn::create();
    GuiLayoutRow::Ptr   row0 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr   row1 = GuiLayoutRow::create();

    c0->addElement(row0);
    c0->addElement(row1);

    GuiLayoutSpacerFrame::Ptr frame0 = GuiLayoutSpacerFrame::create(c0, 0);
    setRootElement(frame0);

    LabelWidget::Ptr label0 = LabelWidget::create("Shell command:");
    label0   ->setLayoutHeight(executeButton->getStandardHeight(), VerticalAdjustment::CENTER);
    editField->setLayoutHeight(executeButton->getStandardHeight(), VerticalAdjustment::CENTER);

    row0->addElement(label0);
    row0->addElement(editField);

    row0->addElement(executeButton);
    row0->addElement(cancelButton);
    //row0->addElement(GuiLayoutSpacer::create(0, 0, 0, 0, INT_MAX, 0));
    
    selectionInputCheckBox  = CheckBox::create("S]election input");      selectionInputCheckBox->setButtonPressedCallback (newCallback(this, &ExecutePanel::handleCheckBoxPressed));
    wholeFileInputCheckBox  = CheckBox::create("Wh]ole file input");     wholeFileInputCheckBox->setButtonPressedCallback (newCallback(this, &ExecutePanel::handleCheckBoxPressed));
    replaceOutputCheckBox   = CheckBox::create("R]eplace with output");  replaceOutputCheckBox->setButtonPressedCallback  (newCallback(this, &ExecutePanel::handleCheckBoxPressed));
    newWindowOutputCheckBox = CheckBox::create("N]ew window output");    newWindowOutputCheckBox->setButtonPressedCallback(newCallback(this, &ExecutePanel::handleCheckBoxPressed));
    
    row1->addElement(selectionInputCheckBox);
    row1->addElement(wholeFileInputCheckBox);
    row1->addElement(replaceOutputCheckBox);
    row1->addElement(newWindowOutputCheckBox);
    row1->addElement(GuiLayoutSpacer::create(0, 0, 0, 0, INT_MAX, 0));
    
    Callback<Button*,Button::ActivationVariant>::Ptr buttonCallback = newCallback(this, &ExecutePanel::handleButtonPressed);
    executeButton->setButtonPressedCallback(buttonCallback);
    cancelButton->setButtonPressedCallback(buttonCallback);

    label0->show();
    editField->show();
    executeButton->show();
    cancelButton->show();  

    selectionInputCheckBox->show();  
    wholeFileInputCheckBox->show();  
    replaceOutputCheckBox->show();  
    newWindowOutputCheckBox->show();  
    
    editField->setNextFocusWidget(executeButton);
    executeButton->setNextFocusWidget(cancelButton);
    cancelButton->setNextFocusWidget(editField);
    setFocus(editField);
    executeButton->setAsDefaultButton();

    label0->setMiddleMouseButtonCallback(newCallback(editField, &SingleLineEditField::replaceTextWithPrimarySelection));
}

void ExecutePanel::handleCheckBoxPressed(CheckBox* checkBox)
{
    if (checkBox == selectionInputCheckBox && checkBox->isChecked()) {
        wholeFileInputCheckBox->setChecked(false);
    }
    else if (checkBox == wholeFileInputCheckBox && checkBox->isChecked()) {
        selectionInputCheckBox->setChecked(false);
    }
    else if (checkBox == replaceOutputCheckBox && checkBox->isChecked()) {
        newWindowOutputCheckBox->setChecked(false);
    }
    else if (checkBox == newWindowOutputCheckBox && checkBox->isChecked()) {
        replaceOutputCheckBox->setChecked(false);
    }
}



void ExecutePanel::handleButtonPressed(Button* button, Button::ActivationVariant variant)
{
    if (button == executeButton)
    {
        if (editorWidget.isValid() && !editorWidget->areCursorChangesDisabled())
        {
            int textLength = editField->getTextData()->getLength();
            if (textLength > 0)
            {
                String input;
                
                if (selectionInputCheckBox->isChecked() && (editorWidget->hasPrimarySelection() || editorWidget->hasPseudoSelection()))
                {
                    input = editorWidget->getTextData()->getSubstring(Pos(editorWidget->getBeginSelectionPos()),
                                                                      Pos(editorWidget->getEndSelectionPos()));
                } else if (wholeFileInputCheckBox->isChecked()) {
                    input = editorWidget->getTextData()->getAsString();
                }
                Commandline::Ptr cmd = Commandline::create();
                cmd->append("/bin/sh");
                cmd->append("-c");
                cmd->append(editField->getTextData()->getAsString());
                
                ProgramExecutor::start(cmd,
                                       input,
                                       Null,
                                       newCallback(this, &ExecutePanel::handleExecutionResult));
            }
        }
        requestClose();
        editField->getTextData()->clear();
    }
    else if (button == cancelButton) {
        requestClose();
        editField->getTextData()->clear();
    }
}


void ExecutePanel::show()
{
    setFocus(editField);

    selectionInputCheckBox ->setChecked(false);
    wholeFileInputCheckBox ->setChecked(false);
    replaceOutputCheckBox  ->setChecked(false);
    newWindowOutputCheckBox->setChecked(false);

    if (editorWidget->hasPrimarySelection() || editorWidget->hasPseudoSelection()) {
        selectionInputCheckBox->setChecked(true);
        replaceOutputCheckBox->setChecked(true);
    } else {
        wholeFileInputCheckBox->setChecked(true);
        replaceOutputCheckBox->setChecked(true);
    }
    DialogPanel::show();
}


void ExecutePanel::handleExecutionResult(ProgramExecutor::Result result)
{
    long spos;
    long epos;
    
    TextData::Ptr textData = editorWidget->getTextData();
    
    if (   selectionInputCheckBox->isChecked()
        && replaceOutputCheckBox->isChecked() 
        && (editorWidget->hasPrimarySelection() || editorWidget->hasPseudoSelection()))
    {
        spos = editorWidget->getBeginSelectionPos();
        epos = editorWidget->getEndSelectionPos();
    }
    else if (   wholeFileInputCheckBox->isChecked()
             && replaceOutputCheckBox->isChecked())
    {
        spos = 0;
        epos = textData->getLength();
    } else {
        spos = editorWidget->getCursorTextPosition();
        epos = spos;
    }
    if (newWindowOutputCheckBox->isChecked())
    {
        LanguageMode::Ptr languageMode = GlobalConfig::getInstance()->getDefaultLanguageMode();
     
        String untitledFileName = File(textData->getFileName()).getDirName() << "/Untitled";
        TextData::Ptr     newTextData     = TextData::create();
                          newTextData->setPseudoFileName(untitledFileName);
                          newTextData->setToData(result.outputBuffer, result.outputLength);
     
        HilitedText::Ptr  hilitedText  = HilitedText::create(newTextData, languageMode);
     
        EditorTopWin::Ptr win          = EditorTopWin::create(hilitedText);
                          win->show();
    }
    else
    {
        TextData::TextMark m = editorWidget->createNewMarkFromCursor();
        
        m.moveToPos(spos);
        textData->insertAtMark(m, (const byte*)result.outputBuffer, result.outputLength);
        
        if (replaceOutputCheckBox->isChecked()) {
            m.moveToPos(spos + result.outputLength);
            textData->removeAtMark(m, epos - spos);
        }
        editorWidget->moveCursorToTextPositionAndAdjustVisibility(spos + result.outputLength);
    }
}

