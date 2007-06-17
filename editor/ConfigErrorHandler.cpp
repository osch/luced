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

#include "ConfigErrorHandler.hpp"
#include "MessageBox.hpp"
#include "GlobalConfig.hpp"
#include "FileException.hpp"
#include "EditorTopWin.hpp"
#include "TopWinList.hpp"

using namespace LucED;

void ConfigErrorHandler::startMessageBox()
{
    TopWinList*      topWins    = TopWinList::getInstance();
    LucED::WeakPtr<TopWin>  lastOpenedTopWin;

    if (topWins != NULL)
    {
        for (int w = topWins->getNumberOfTopWins() - 1; w >= 0; --w)
        {
            EditorTopWin* topWin = dynamic_cast<EditorTopWin*>(topWins->getTopWin(w));
            if (topWin != NULL) {
                lastOpenedTopWin = topWin;
                break;
            }
        }
    }

    if (errorList.isValid() && errorList->getLength() > 0)
    {
        MessageBox::Ptr messageBox;
        
        if (errorList->getLength() == 1)
        {
            messageBox = MessageBox::create(MessageBoxParameter()
                                             .setTitle("Error in config file")
                                             .setMessage(String () << "Error within config file '" << errorList->get(0).getConfigFileName() << "'")
                                             .setDefaultButton    ("E]dit config file",    Callback0(this, &ConfigErrorHandler::handleOpenFilesButton))
                                             .setCancelButton     ("C]ancel",              Callback0(this, &ConfigErrorHandler::handleAbortButton)));
        }
        else
        {
            messageBox = MessageBox::create(MessageBoxParameter()
                                             .setTitle("Error in config files")
                                             .setMessage("There are some errors within config files.")
                                             .setDefaultButton    ("E]dit config files",   Callback0(this, &ConfigErrorHandler::handleOpenFilesButton))
                                             .setCancelButton     ("C]ancel",              Callback0(this, &ConfigErrorHandler::handleAbortButton)));
        }
        messageBox->setReferingWindowForPositionHintsOnly(lastOpenedTopWin);
        messageBox->show();
    }
    else {
        EventDispatcher::getInstance()->deregisterRunningComponent(this);
    }
}

void ConfigErrorHandler::handleAbortButton()
{
    EventDispatcher::getInstance()->deregisterRunningComponent(this);
}


void ConfigErrorHandler::handleOpenFilesButton()
{
    TextStyles::Ptr  textStyles = GlobalConfig::getInstance()->getTextStyles();
    TopWinList*      topWins    = TopWinList::getInstance();

    for (int i = 0; i < errorList->getLength(); ++i)
    {
        String fileName = errorList->get(i).getConfigFileName();
        
        EditorTopWin::Ptr editorWin;        

        if (topWins != NULL)
        {
            for (int w = 0; w < topWins->getNumberOfTopWins(); ++w)
            {
                EditorTopWin* topWin = dynamic_cast<EditorTopWin*>(topWins->getTopWin(w));
                if (topWin != NULL && topWin->getFileName() == fileName) {
                    topWin->raise();
                    editorWin = topWin;
                }
            }
        }
        
        MessageBoxParameter p;

        if (editorWin.isValid())
        {
            p.setTitle("Config Error")
             .setMessage(errorList->get(i).getMessage());
        }
        else
        {
            LanguageMode::Ptr languageMode = GlobalConfig::getInstance()->getLanguageModeForFileName(fileName);
            TextData::Ptr     textData     = TextData::create();
            HilitedText::Ptr  hilitedText  = HilitedText::create(textData, languageMode);

            try
            {
                textData->loadFile(fileName);

                p.setTitle("Config Error")
                 .setMessage(errorList->get(i).getMessage());
            }
            catch (FileException& ex)
            {
                textData->setFileName(fileName);

                p.setTitle("Error opening file")
                 .setMessage(ex.getMessage());
            }
            editorWin = EditorTopWin::create(textStyles, hilitedText);
            editorWin->show();
        }
        editorWin->invokeMessageBox(p);
    }
    EventDispatcher::getInstance()->deregisterRunningComponent(this);
}

