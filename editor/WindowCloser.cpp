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

#include "WindowCloser.hpp"
#include "TopWinList.hpp"

using namespace LucED;


WindowCloser::WeakPtr WindowCloser::instance;


void WindowCloser::handleSaveFileButton()
{
    if (referingTopWin.isValid())
    {
        referingTopWin->saveAndClose();
        if (referingTopWin->isClosing()) {
            closeWindows();
        } else {
            EventDispatcher::getInstance()->deregisterRunningComponent(this);
        }
    } else {
        closeWindows();
    }
}


void WindowCloser::handleDiscardButton()
{
    if (referingTopWin.isValid())
    {
        referingTopWin->requestCloseWindowAndDiscardChanges();
    }
    closeWindows();
}


void WindowCloser::handleCancelButton()
{
    EventDispatcher::getInstance()->deregisterRunningComponent(this);
}


void WindowCloser::closeWindows()
{
    TopWinList* topWins = TopWinList::getInstance();

    while (0 < topWins->getNumberOfTopWins())
    {
        TopWin* topWin = topWins->getTopWin(topWins->getNumberOfTopWins() - 1);
        
        EditorTopWin* editorTopWin = dynamic_cast<EditorTopWin*>(topWin);

        if (editorTopWin != NULL)
        {
            if (editorTopWin->hasUnsavedData())
            {
                editorTopWin->raise();
                editorTopWin->setMessageBox(MessageBoxParameter()
                                            .setTitle("Unsaved Data")
                                            .setMessage(String() << "File '" << editorTopWin->getFileName() << "' has unsaved data.")
                                            .setDefaultButton    ("S]ave",     newCallback(this, &WindowCloser::handleSaveFileButton))
                                            .setAlternativeButton("D]iscard",  newCallback(this, &WindowCloser::handleDiscardButton))
                                            .setCancelButton     ("C]ancel",   newCallback(this, &WindowCloser::handleCancelButton)));
                this->referingTopWin = editorTopWin;
                break;
            }
            else
            {
                editorTopWin->requestCloseWindow();
            }
        }
        else
        {
            topWin->requestCloseWindow();
        }
    }
    
    if (topWins->getNumberOfTopWins() == 0)
    {
        EventDispatcher::getInstance()->deregisterRunningComponent(this);
    }    
}

