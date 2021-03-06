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
    LucED::WeakPtr<TopWin>  lastFocusedTopWin;

    if (topWins != NULL)
    {
        for (int w = topWins->getNumberOfTopWins() - 1; w >= 0; --w)
        {
            EditorTopWin* topWin = dynamic_cast<EditorTopWin*>(topWins->getTopWin(w));
            if (topWin != NULL) {
                lastOpenedTopWin = topWin;
                if (topWin->hasFocus()) {
                    lastFocusedTopWin = topWin;
                    break;
                }
            }
        }
    }
    LucED::WeakPtr<TopWin> referingTopWin;

    if (lastFocusedTopWin.isValid()) {
        referingTopWin = lastFocusedTopWin;
    } else {
        referingTopWin = lastOpenedTopWin;
    }

    if (errorList.isValid() && errorList->getLength() > 0)
    {
        MessageBox::Ptr messageBox;

        if (errorList->getLength() == 1)
        {
            if (!errorList->get(0).hasConfigFileName()) {
                messageBox = MessageBox::create(MessageBoxParameter()
                                                 .setTitle("Configuration Error")
                                                 .setMessage(String () << errorList->get(0).getMessage()));

                EventDispatcher::getInstance()->deregisterRunningComponent(this);
            } else {
                messageBox = MessageBox::create(MessageBoxParameter()
                                                 .setTitle("Error in config file")
                                                 .setMessage(String () << "Error within LucED config file '" << errorList->get(0).getConfigFileName() << "'")
                                                 .setDefaultButton    ("E]dit config file",    newCallback(this, &ConfigErrorHandler::handleOpenFilesButton))
                                                 .setCancelButton     ("C]ancel",              newCallback(this, &ConfigErrorHandler::handleAbortButton)));
            }
        }
        else
        {
            messageBox = MessageBox::create(MessageBoxParameter()
                                             .setTitle("Error in config files")
                                             .setMessage("There are some errors within LucED config files.")
                                             .setDefaultButton    ("E]dit config files",   newCallback(this, &ConfigErrorHandler::handleOpenFilesButton))
                                             .setCancelButton     ("C]ancel",              newCallback(this, &ConfigErrorHandler::handleAbortButton)));
        }
        messageBox->setReferingWindowForPositionHintsOnly(referingTopWin);
        messageBox->show();
    }
    else {
        EventDispatcher::getInstance()->deregisterRunningComponent(this);
    }
}

void ConfigErrorHandler::handleAbortButton()
{
    if (fileOpenerParameters.isValid() && fileOpenerParameters->getLength() > 0)
    {
        FileOpener::start(fileOpenerParameters);
    }
    EventDispatcher::getInstance()->deregisterRunningComponent(this);
}


void ConfigErrorHandler::handleOpenFilesButton()
{
    FileOpener::start(fileOpenerParameters, errorList);
    EventDispatcher::getInstance()->deregisterRunningComponent(this);
}



