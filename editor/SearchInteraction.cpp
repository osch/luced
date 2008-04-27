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

#include "SearchInteraction.hpp"
#include "SearchHistory.hpp"
#include "GuiRoot.hpp"

using namespace LucED;

void SearchInteraction::setTextPositionForSearchStart()
{
    int textPosition = e->getCursorTextPosition();
    
     if (e->hasPrimarySelection())
     {
         if (replaceUtil.isSearchingForward())
         {
             replaceUtil.setTextPosition(e->getBeginSelectionPos());
             if (replaceUtil.doesMatch() && replaceUtil.getMatchEndPos() == e->getEndSelectionPos()) {
                 textPosition = e->getEndSelectionPos();
             }
         }
         else
         {
             replaceUtil.setTextPosition(e->getBeginSelectionPos());
             if (replaceUtil.doesMatch() && replaceUtil.getMatchEndPos() == e->getEndSelectionPos()) {
                 textPosition = e->getBeginSelectionPos();
             }
         }
     }
     replaceUtil.setTextPosition(textPosition);
}

void SearchInteraction::internalStartFind(bool findSelection)
{
    try
    {
        if (findSelection)
        {
            cb.requestCurrentSelectionCallback->call(this, newCallback(this, &ThisClass::continueWithFindSelectionWithoutAutoContinue));
        }
        else {
            if (p.getFindString().getLength() <= 0) {
                return;
            }
    
            replaceUtil.setParameter(p);
            
            SearchHistory::getInstance()->append(p);
    
            setTextPositionForSearchStart();
            internalExecute(false, false);
        }
    }
    catch (...) 
    {
        cb.exceptionHandler->call();
    }
}

void SearchInteraction::internalReplaceAndFind(bool continueWithFind)
{
    try
    {
        if (p.getFindString().getLength() <= 0) {
            return;
        }

        replaceUtil.setParameter(p);
        
        SearchHistory::getInstance()->append(p);

        int   textPosition = e->getCursorTextPosition();
        bool  wasReplaced  = false;
        
        {
            // try to replace at current position
            
            int spos, epos;
            
            if (e->hasPrimarySelection()) {
                spos = e->getBeginSelectionPos();
                epos = e->getEndSelectionPos();
                replaceUtil.setTextPosition(spos);
            } else {
                spos = replaceUtil.getTextPosition();
                epos = replaceUtil.getTextPosition();
            }
    
            RawPtr<TextData> textData = e->getTextData();
    
            replaceUtil.setTextPosition(spos);
            
            if (replaceUtil.doesMatch() && replaceUtil.getMatchEndPos() == epos)
            {
                String substitutedString = replaceUtil.getSubstitutedString();
                
                TextData::TextMark textMark = e->createNewMarkFromCursor();
                textMark.moveToPos(spos);
                textData->insertAtMark(textMark, substitutedString);
    
                textMark.moveToPos(spos + substitutedString.getLength());
                textData->removeAtMark(textMark, replaceUtil.getMatchLength());
    
                epos += substitutedString.getLength() - replaceUtil.getMatchLength();
                
                if (p.hasSearchForwardFlag()) {    
                    e->moveCursorToTextPosition(epos);
                } else {
                    e->moveCursorToTextPosition(spos);
                }
                e->assureCursorVisible();
                wasReplaced = true;
            }
        }
        
        if (continueWithFind)
        {
            int textPosition = e->getCursorTextPosition();
            
            if (!wasReplaced)
            {
                if (p.hasSearchForwardFlag()) {
                    if (e->hasPrimarySelection()) {
                        replaceUtil.setTextPosition(e->getBeginSelectionPos());
                        if (replaceUtil.doesMatch() && replaceUtil.getMatchEndPos() == e->getEndSelectionPos()) {
                            textPosition = e->getEndSelectionPos();
                        }
                    }
                } else {
                    if (e->hasPrimarySelection()) {
                        replaceUtil.setTextPosition(e->getBeginSelectionPos());
                        if (replaceUtil.doesMatch() && replaceUtil.getMatchEndPos() == e->getEndSelectionPos()) {
                            textPosition = e->getBeginSelectionPos();
                        }
                    }
                }
            }
            replaceUtil.setTextPosition(textPosition);
            
            internalExecute(false, false);
        }
    }
    catch (...) 
    {
        cb.exceptionHandler->call();
    }
}

void SearchInteraction::continueWithFindSelectionWithoutAutoContinue(String currentSelection)
{
    continueWithFindSelection(currentSelection, false);
}

void SearchInteraction::continueWithFindSelectionAndAutoContinue(String currentSelection)
{
    continueWithFindSelection(currentSelection, true);
}

void SearchInteraction::continueWithFindSelection(String currentSelection, bool autoContinue)
{
    try
    {
        if (currentSelection.getLength() > 0)
        {
            if (!e->areCursorChangesDisabled())
            {
                bool selectSearchRegexFlag = false;
                if (currentSelection.contains('\n')) {
                    selectSearchRegexFlag = true;
                    currentSelection = ReplaceUtil::quoteRegexCharacters(currentSelection);
                }
                p.setRegexFlag(selectSearchRegexFlag);
                p.setFindString(currentSelection);
                
                SearchHistory::getInstance()->append(p);
        
                replaceUtil.setParameter(p);

                setTextPositionForSearchStart();
                internalExecute(false, autoContinue);
            }
            e->assureCursorVisible();
        }
    }
    catch (...) 
    {
        cb.exceptionHandler->call();
    }
}


void SearchInteraction::handleContinueAtBeginButton()
{
    ASSERT(replaceUtil.getSearchForwardFlag() == true);

    replaceUtil.setTextPosition(0);
    internalExecute(true, false);
}

void SearchInteraction::handleContinueAtEndButton()
{
    ASSERT(replaceUtil.getSearchForwardFlag() == false);

    replaceUtil.setTextPosition(e->getTextData()->getLength());
    internalExecute(true, false);
}

void SearchInteraction::findAgainForwardAndAutoContinue()
{
    cb.requestCloseOfInvokingPanel->call(this);

    replaceUtil.setSearchForwardFlag(true);

    setTextPositionForSearchStart();
    internalExecute(false, true);
}

void SearchInteraction::findAgainBackwardAndAutoContinue()
{
    cb.requestCloseOfInvokingPanel->call(this);

    replaceUtil.setSearchForwardFlag(false);

    setTextPositionForSearchStart();
    internalExecute(false, true);
}

void SearchInteraction::findAgainForward()
{
    cb.requestCloseOfInvokingPanel->call(this);

    replaceUtil.setSearchForwardFlag(true);

    setTextPositionForSearchStart();
    internalExecute(false, false);
}

void SearchInteraction::findAgainBackward()
{
    cb.requestCloseOfInvokingPanel->call(this);
    
    replaceUtil.setSearchForwardFlag(false);

    setTextPositionForSearchStart();
    internalExecute(false, false);
}


void SearchInteraction::findSelectionForward()
{
    cb.requestCloseOfInvokingPanel->call(this);

    p.setSearchForwardFlag(true);

    cb.requestCurrentSelectionCallback->call(this, newCallback(this, &ThisClass::continueWithFindSelectionWithoutAutoContinue));
}


void SearchInteraction::findSelectionForwardAndAutoContinue()
{
    cb.requestCloseOfInvokingPanel->call(this);

    p.setSearchForwardFlag(true);

    cb.requestCurrentSelectionCallback->call(this, newCallback(this, &ThisClass::continueWithFindSelectionAndAutoContinue));
}


void SearchInteraction::findSelectionBackward()
{
    cb.requestCloseOfInvokingPanel->call(this);

    p.setSearchForwardFlag(false);

    cb.requestCurrentSelectionCallback->call(this, newCallback(this, &ThisClass::continueWithFindSelectionWithoutAutoContinue));
}


void SearchInteraction::findSelectionBackwardAndAutoContinue()
{
    cb.requestCloseOfInvokingPanel->call(this);

    p.setSearchForwardFlag(false);

    cb.requestCurrentSelectionCallback->call(this, newCallback(this, &ThisClass::continueWithFindSelectionAndAutoContinue));
}


void SearchInteraction::internalExecute(bool isWrapping, bool autoContinue)
{
    try
    {
        GuiRoot::getInstance()->flushDisplay();
    
        replaceUtil.setAllowMatchAtStartOfSearchFlag(isWrapping);
        replaceUtil.findNext();
    
        if (replaceUtil.wasFound())
        {
            TextData::TextMark m = e->createNewMarkFromCursor();
            if (replaceUtil.isSearchingForward()) {
                m.moveToPos(replaceUtil.getMatchBeginPos());
            } else {
                m.moveToPos(replaceUtil.getMatchEndPos());
            }
            e->moveCursorToTextMarkAndAdjustVisibility(m);
            if (replaceUtil.isSearchingForward()) {
                m.moveToPos(replaceUtil.getMatchEndPos());
            } else {
                m.moveToPos(replaceUtil.getMatchBeginPos());
            }
            e->moveCursorToTextMarkAndAdjustVisibility(m);
            e->rememberCursorPixX();
            if (replaceUtil.getMatchBeginPos() < replaceUtil.getMatchEndPos())
            {
                e->setPrimarySelection(replaceUtil.getMatchBeginPos(),
                                       replaceUtil.getMatchEndPos());
            } else {
                e->releaseSelection();
            }
        }
        else if (isWrapping)
        {
                cb.messageBoxInvoker->call(MessageBoxParameter()
                                          .setTitle("Not found")
                                          .setMessage("String was not found")
                                          .setMessageBoxQueue(cb.messageBoxQueue));
        }
        else if (autoContinue)
        {
            if (replaceUtil.isSearchingForward()) {
                handleContinueAtBeginButton();
            } else {
                handleContinueAtEndButton();
            }
        }
        else
        {
            if (replaceUtil.isSearchingForward()) {
                cb.messageBoxInvoker->call(MessageBoxParameter()
                                          .setTitle("Not found")
                                          .setMessage("Continue search from beginning of file?")
                                          .setDefaultButton("C]ontinue", newCallback(this, &ThisClass::handleContinueAtBeginButton))
                                          .setCancelButton("Ca]ncel")
                                          .addKeyMapping(KeyModifier("Ctrl"),       KeyId("g"), newCallback(this, &ThisClass::findAgainForwardAndAutoContinue))
                                          .addKeyMapping(KeyModifier("Ctrl+Shift"), KeyId("g"), newCallback(this, &ThisClass::findAgainBackward))
                                          .addKeyMapping(KeyModifier("Ctrl"),       KeyId("h"), newCallback(this, &ThisClass::findSelectionForwardAndAutoContinue))
                                          .addKeyMapping(KeyModifier("Ctrl+Shift"), KeyId("h"), newCallback(this, &ThisClass::findSelectionBackward))
                                          .setMessageBoxQueue(cb.messageBoxQueue));
            } else {
                cb.messageBoxInvoker->call(MessageBoxParameter()
                                          .setTitle("Not found")
                                          .setMessage("Continue search from end of file?")
                                          .setDefaultButton("C]ontinue", newCallback(this, &ThisClass::handleContinueAtEndButton))
                                          .setCancelButton("Ca]ncel")
                                          .addKeyMapping(KeyModifier("Ctrl"),       KeyId("g"), newCallback(this, &ThisClass::findAgainForward))
                                          .addKeyMapping(KeyModifier("Ctrl+Shift"), KeyId("g"), newCallback(this, &ThisClass::findAgainBackwardAndAutoContinue))
                                          .addKeyMapping(KeyModifier("Ctrl"),       KeyId("h"), newCallback(this, &ThisClass::findSelectionForward))
                                          .addKeyMapping(KeyModifier("Ctrl+Shift"), KeyId("h"), newCallback(this, &ThisClass::findSelectionBackwardAndAutoContinue))
                                          .setMessageBoxQueue(cb.messageBoxQueue));
            }
        }
    }
    catch (...) 
    {
        cb.exceptionHandler->call();
    }
}

