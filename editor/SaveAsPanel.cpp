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

#include "SaveAsPanel.hpp"
#include "GuiLayoutColumn.hpp"
#include "GuiLayoutRow.hpp"
#include "GlobalConfig.hpp"
#include "GuiLayoutSpacer.hpp"
#include "LabelWidget.hpp"

using namespace LucED;

SaveAsPanel::SaveAsPanel(GuiWidget* parent, TextEditorWidget* editorWidget, 
                                            Callback<MessageBoxParameter>::Ptr messageBoxInvoker,
                                            Callback<GuiWidget*>::Ptr          requestCloseCallback)
    : DialogPanel(parent, requestCloseCallback),
      editorWidget(editorWidget),
      messageBoxInvoker(messageBoxInvoker)
{
    saveButton = Button::create(this, "S]ave File");
    cancelButton = Button::create(this, "C]ancel");
   
    this->editField = SingleLineEditField::create(this, 
                                                  GlobalConfig::getInstance()->getDefaultLanguageMode());
    editField->setDesiredWidthInChars(10, 30, INT_MAX);

    LabelWidget::Ptr label0 = LabelWidget::create(this, "File Name:");
    GuiLayoutRow::Ptr row0 = GuiLayoutRow::create();
    GuiLayoutSpacerFrame::Ptr frame0 = GuiLayoutSpacerFrame::create(row0, 0);
    setRootElement(frame0);

    label0   ->setLayoutHeight(saveButton->getStandardHeight(), VerticalAdjustment::CENTER);
    editField->setLayoutHeight(saveButton->getStandardHeight(), VerticalAdjustment::CENTER);

    row0->addElement(label0);
    row0->addElement(editField);

    row0->addElement(saveButton);
    row0->addElement(cancelButton);
    row0->addElement(GuiLayoutSpacer::create(0, 0, 0, 0, INT_MAX, 0));
    
    Callback<Button*>::Ptr buttonCallback = newCallback(this, &SaveAsPanel::handleButtonPressed);
    saveButton->setButtonPressedCallback(buttonCallback);
    cancelButton->setButtonPressedCallback(buttonCallback);

    label0->show();
    editField->show();
    saveButton->show();
    cancelButton->show();  
    
    editField->setNextFocusWidget(saveButton);
    saveButton->setNextFocusWidget(cancelButton);
    cancelButton->setNextFocusWidget(editField);
    setFocus(editField);
    saveButton->setAsDefaultButton();

    label0->setMiddleMouseButtonCallback(newCallback(editField, &SingleLineEditField::replaceTextWithPrimarySelection));
}


void SaveAsPanel::treatFocusIn()
{
    setFocus(editField);
    DialogPanel::treatFocusIn();
}


void SaveAsPanel::continueSave()
{
    String newFileName = editField->getTextData()->getAsString();
    RawPtr<TextData> textData = editorWidget->getTextData();

    if (textData->getFileName() != newFileName)
    {
        LanguageMode::Ptr languageMode = GlobalConfig::getInstance()->getLanguageModeForFileName(newFileName);
        if (languageMode != editorWidget->getHilitedText()->getLanguageMode()) {
            editorWidget->getHilitedText()->setLanguageMode(languageMode);
        }
    }
    textData->setRealFileName(newFileName);
    requestClose();
    saveCallback->call();
}


void SaveAsPanel::handleButtonPressed(Button* button)
{
    if (button == saveButton)
    {
        String newFileName = editField->getTextData()->getAsString();
        RawPtr<TextData> textData = editorWidget->getTextData();
        
        if (   (textData->isFileNamePseudo() || textData->getFileName() != newFileName)
            && File(newFileName).exists())
        {
            messageBoxInvoker->call(MessageBoxParameter()
                                    .setTitle("File exists")
                                    .setMessage(String() << "File '" << newFileName << "' exists.")
                                    .setDefaultButton("S]ave", newCallback(this, &SaveAsPanel::continueSave))
                                    .setCancelButton("Ca]ncel"));
        }
        else
        {
            continueSave();
        }
    }
    else if (button == cancelButton) {
        requestClose();
    }
}


void SaveAsPanel::show()
{
    RawPtr<TextData> textData = editorWidget->getTextData();
    String newContent;
    if (textData->isFileNamePseudo()) {
        newContent = String() << File(textData->getFileName()).getDirName() << "/";
    } else {
        newContent = File(textData->getFileName()).getAbsoluteName();
    }
    editField->getTextData()->setToString(newContent);
    editField->getTextData()->clearHistory();
    editField->setCursorPosition(newContent.getLength());

    DialogPanel::show();
}

