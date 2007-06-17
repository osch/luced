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

#include <errno.h>
#include <stdio.h>

#include "FileOpener.hpp"
#include "GlobalConfig.hpp"
#include "FileException.hpp"

using namespace LucED;


void FileOpener::handleSkipFileButton()
{
    if (numberAndFileList.isValid() && numberAndFileList->getLength() > 0) {
        numberAndFileList->remove(0);
    }
    if (lastTopWin.isValid()) {
        lastTopWin->requestCloseWindow();
        lastTopWin = NULL;
    }
    isWaitingForMessageBox = false;
    openFiles();
}


void FileOpener::handleCreateFileButton()
{
    isWaitingForMessageBox = false;
    if (lastTopWin.isValid()) {
        lastTopWin->closeModalMessageBox();
        lastTopWin = NULL;
    }
    openFiles();
}


void FileOpener::handleAbortButton()
{
    if (lastTopWin.isValid()) {
        lastTopWin->requestCloseWindow();
        lastTopWin = NULL;
    }
    numberAndFileList.invalidate();
    isWaitingForMessageBox = false;
    EventDispatcher::getInstance()->deregisterRunningComponent(this);
}


void FileOpener::openFiles()
{
    ASSERT(!isWaitingForMessageBox)

    TextStyles::Ptr  textStyles = GlobalConfig::getInstance()->getTextStyles();

    while (numberAndFileList.isValid() && numberAndFileList->getLength() > 0)
    {
        int    numberOfWindows = numberAndFileList->get(0).numberOfWindows;
        String fileName        = numberAndFileList->get(0).fileName;

        if (numberOfWindows <= 0)
        {
            numberOfWindows = 1;
        }

        TopWinList* topWins = TopWinList::getInstance();

        if (lastTopWin.isInvalid())
        {

            numberOfRaisedWindows = 0;

            for (int w = 0; w < topWins->getNumberOfTopWins() && numberOfRaisedWindows < numberOfWindows; ++w)
            {
                EditorTopWin* topWin = dynamic_cast<EditorTopWin*>(topWins->getTopWin(w));
                if (topWin != NULL && topWin->getFileName() == fileName) {
                    topWin->raise();
                    numberOfRaisedWindows += 1;
                    lastTopWin = topWin;
                }
            }

            if (lastTopWin.isInvalid() && numberOfRaisedWindows < numberOfWindows)
            {
                LanguageMode::Ptr languageMode = GlobalConfig::getInstance()->getLanguageModeForFileName(fileName);
                TextData::Ptr     textData     = TextData::create();
                HilitedText::Ptr  hilitedText  = HilitedText::create(textData, languageMode);

                try
                {
                    textData->loadFile(fileName);
                }
                catch (FileException& ex) {
                    if (ex.getErrno() == ENOENT)
                    {
                        isWaitingForMessageBox = true;
                        lastErrorMessage = ex.getMessage();
                        textData->setFileName(fileName);
                        lastTopWin = EditorTopWin::create(textStyles, hilitedText);

                        MessageBoxParameter p;

                        if (numberAndFileList->getLength() > 1) {
                                            p.setTitle("Error opening files")
                                             .setMessage(ex.getMessage())
                                             .setDefaultButton    ("C]reate this file",     Callback0(this, &FileOpener::handleCreateFileButton))
                                             .setAlternativeButton("A]bort all next files", Callback0(this, &FileOpener::handleAbortButton))
                                             .setCancelButton     ("S]kip to next file",    Callback0(this, &FileOpener::handleSkipFileButton));

                        } else {
                                            p.setTitle("Error opening file")
                                             .setMessage(ex.getMessage())
                                             .setDefaultButton    ("C]reate this file",     Callback0(this, &FileOpener::handleCreateFileButton))
                                             .setCancelButton     ("Ca]ncel", Callback0(this, &FileOpener::handleAbortButton));
                        }
                        lastTopWin->setModalMessageBox(p);
                        lastTopWin->show();
                        return;
                    } else {
                        throw;
                    }
                }

                lastTopWin = EditorTopWin::create(textStyles, hilitedText);
                lastTopWin->show();

                numberOfRaisedWindows += 1;
            }
        }

        for (int i = numberOfRaisedWindows; i < numberOfWindows; ++i)
        {
            EditorTopWin::Ptr win = EditorTopWin::create(lastTopWin->getTextStyles(),
                                                         lastTopWin->getHilitedText());
            win->show();
        }
        numberAndFileList->remove(0);
        lastTopWin = NULL;
    }
    if ((numberAndFileList.isInvalid() || numberAndFileList->getLength() == 0) && !isWaitingForMessageBox) {
        numberAndFileList.invalidate();
        EventDispatcher::getInstance()->deregisterRunningComponent(this);
    }
}

