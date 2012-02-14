/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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
#include "EncodingConverter.hpp"

using namespace LucED;

SaveAsPanel::SaveAsPanel(TextEditorWidget* editorWidget, 
                         Callback<const MessageBoxParameter&>::Ptr messageBoxInvoker,
                         Callback<>::Ptr                           panelCloser)
    : DialogPanel(panelCloser),
      editorWidget(editorWidget),
      messageBoxInvoker(messageBoxInvoker)
{
    saveButton = Button::create("S]ave File");
    cancelButton = Button::create("C]ancel");
   
    this->editField = SingleLineEditField::create(GlobalConfig::getInstance()->getDefaultLanguageMode());
    editField->setDesiredWidthInChars(10, 30, INT_MAX);

    LabelWidget::Ptr label0 = LabelWidget::create("File Name:");
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
    
    Callback<Button*,Button::ActivationVariant>::Ptr buttonCallback = newCallback(this, &SaveAsPanel::handleButtonPressed);
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



void SaveAsPanel::continueSave()
{
    String utf8FileName = editField->getTextData()->getAsString();
    String newFileName  = EncodingConverter::convertUtf8ToLocaleStringIgnoreErrors(utf8FileName);

    RawPtr<TextData> textData = editorWidget->getTextData();
    textData->setRealFileName(newFileName);
    requestClose();
    saveCallback->call();
}


void SaveAsPanel::handleButtonPressed(Button* button, Button::ActivationVariant variant)
{
    if (button == saveButton)
    {
        String utf8FileName = editField->getTextData()->getAsString();
        String newFileName  = EncodingConverter::convertUtf8ToLocaleStringIgnoreErrors(utf8FileName);
        
        RawPtr<TextData> textData = editorWidget->getTextData();
        
        if (textData->isFileNamePseudo() || textData->getFileName() != newFileName)
        {
            File::Info newFile = File(newFileName).getInfo();
            
            if (newFile.exists())
            {
                if (newFile.isFile() && newFile.isWritable())
                {
                    messageBoxInvoker->call(MessageBoxParameter()
                                            .setTitle("File exists")
                                            .setMessage(String() << "File '" << utf8FileName << "' exists.")
                                            .setDefaultButton("S]ave", newCallback(this, &SaveAsPanel::continueSave))
                                            .setCancelButton("Ca]ncel"));
                }
                else if (!newFile.isFile()) {
                    messageBoxInvoker->call(MessageBoxParameter()
                                            .setTitle("Not a file")
                                            .setMessage(String() << "'" << utf8FileName << "' exists and is not a file."));
                }
                else if (!newFile.isWritable()) {
                    messageBoxInvoker->call(MessageBoxParameter()
                                            .setTitle("Not writable")
                                            .setMessage(String() << "'" << utf8FileName << "' exists and is not writable."));
                }
                else {
                    ASSERT(false);
                }
            }
            else {
                continueSave();
            }
        }
        else {
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
    if (textData->isFileNamePseudo()) 
    {
        String dirName = File(textData->getFileName()).getDirName();
        newContent = String() << EncodingConverter::convertLocaleToUtf8StringIgnoreErrors(dirName)
                              << "/";
    } else {
        String absName = File(textData->getFileName()).getAbsoluteName();
        newContent     = EncodingConverter::convertLocaleToUtf8StringIgnoreErrors(absName);
    }
    editField->getTextData()->setToString(newContent);
    editField->getTextData()->clearHistory();
    editField->setCursorPosition(newContent.getLength());

    setFocus(editField);

    DialogPanel::show();
}

