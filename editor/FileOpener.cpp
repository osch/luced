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
        lastTopWin->requestCloseWindow(TopWin::CLOSED_SILENTLY);
        lastTopWin = NULL;
    }
    isWaitingForMessageBox = false;
    openFiles();
}


void FileOpener::handleCreateFileButton()
{
    isWaitingForMessageBox = false;
    if (lastTopWin.isValid()) {
        lastTopWin->closeMessageBox();
        lastTopWin = NULL;
    }
    openFiles();
}


void FileOpener::handleAbortButton()
{
    if (lastTopWin.isValid()) {
        lastTopWin->requestCloseWindow(TopWin::CLOSED_SILENTLY);
        lastTopWin = NULL;
    }
    numberAndFileList.invalidate();
    isWaitingForMessageBox = false;
    openConfigFiles();
}


void FileOpener::openFiles()
{
    if (isWaitingForMessageBox) {
        isWaitingForMessageBox = false;
        ASSERT(lastTopWin.isValid());
        if (lastTopWin.isValid()) {
            lastTopWin->requestCloseWindow(TopWin::CLOSED_SILENTLY);
            lastTopWin = NULL;
        }
    }

    ASSERT(!isWaitingForMessageBox)

    TextStyles::Ptr  textStyles = GlobalConfig::getInstance()->getTextStyles();

    while (numberAndFileList.isValid() && numberAndFileList->getLength() > 0)
    {
        int    numberOfWindows  = numberAndFileList->get(0).numberOfWindows;
        String fileName         = numberAndFileList->get(0).fileName;
        String resolvedFileName = File(fileName).getAbsoluteNameWithResolvedLinks();

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
                if (topWin != NULL && File(topWin->getFileName()).getAbsoluteNameWithResolvedLinks() == resolvedFileName) {
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
                catch (FileException& ex)
                {
                    isWaitingForMessageBox = true;
                    
                    if (ex.getErrno() == ENOENT)
                    {
                        textData->setRealFileName(fileName);
                    }
                    else {
                        textData->setPseudoFileName(fileName);
                    }
                    
                    lastTopWin = EditorTopWin::create(textStyles, hilitedText);

                    MessageBoxParameter p;
                    
                    if (numberAndFileList->getLength() > 1)
                    {
                                        p.setTitle("Error opening files")
                                         .setMessage(ex.getMessage())
                                         .setAlternativeButton("A]bort all next files", newCallback(this, &FileOpener::handleAbortButton))
                                         .setCancelButton     ("S]kip to next file",    newCallback(this, &FileOpener::handleSkipFileButton));
                        if (ex.getErrno() == ENOENT) {
                                        p.setDefaultButton    ("C]reate this file",     newCallback(this, &FileOpener::handleCreateFileButton));
                        } else {
                                        p.setDefaultButton    ("R]etry",                newCallback(this, &FileOpener::openFiles));
                        }
                    } 
                    else {
                                        p.setTitle("Error opening file")
                                         .setMessage(ex.getMessage())
                                         .setCancelButton     ("Ca]ncel",               newCallback(this, &FileOpener::handleAbortButton));
                        if (ex.getErrno() == ENOENT) {
                                        p.setDefaultButton    ("C]reate this file",     newCallback(this, &FileOpener::handleCreateFileButton));
                        } else {
                                        p.setDefaultButton    ("R]etry",                newCallback(this, &FileOpener::openFiles));
                        }
                    }
                    lastTopWin->setModalMessageBox(p);
                    lastTopWin->show();
                    return;
                }

                lastTopWin = EditorTopWin::create(textStyles, hilitedText);
                lastTopWin->show();
                lastTopWin->raise();

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
        openConfigFiles();
    }
}


void FileOpener::openConfigFiles()
{
    if (configErrorList.isValid() && configErrorList->getLength() > 0)
    {
        TextStyles::Ptr  textStyles = GlobalConfig::getInstance()->getTextStyles();
        TopWinList*      topWins    = TopWinList::getInstance();

        for (int i = 0; i < configErrorList->getLength(); ++i)
        {
            String fileName = configErrorList->get(i).getConfigFileName();

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
                 .setMessage(configErrorList->get(i).getMessage());
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
                     .setMessage(configErrorList->get(i).getMessage());
                }
                catch (FileException& ex)
                {
                    textData->setRealFileName(fileName);

                    p.setTitle("Error opening file")
                     .setMessage(ex.getMessage());
                }
                editorWin = EditorTopWin::create(textStyles, hilitedText);
                editorWin->show();
            }
            editorWin->setMessageBox(p);
        }
    }
    EventDispatcher::getInstance()->deregisterRunningComponent(this);
}
