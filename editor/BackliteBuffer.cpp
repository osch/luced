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

#include "BackliteBuffer.hpp"


using namespace LucED;

BackliteBuffer::BackliteBuffer(TextData::Ptr textData)
      : textData(textData),
        startPos(0),
        hasSelection(false),
        isSecondarySelection(false)
{
}

byte BackliteBuffer::getNonBufferedBackground(long textPos)
{
    return 0;
}

void BackliteBuffer::registerUpdateListener(Callback<HilitingBuffer::UpdateInfo>::Ptr updateCallback)
{
    updateListeners.registerCallback(updateCallback);
}

void BackliteBuffer::registerListenerForNextChange(Callback<>::Ptr callback)
{
    nextChangeListeners.registerCallback(callback);
}

void BackliteBuffer::activateSelection(long textPos)
{
    isSecondarySelection = false;
    if (hasSelection) {
        deactivateSelection();
    }
    hasSelection = true;
    isSelectionAnchorAtBegin = true;
    beginSelection = textData->createNewMark();
    beginSelection.moveToPos(textPos);
    endSelection   = textData->createNewMark(beginSelection);
}

void BackliteBuffer::makeSelectionToSecondarySelection()
{
    if (hasSelection) {
        if (!isSecondarySelection) {
            isSecondarySelection = true;
            textData->flushPendingUpdates();
            updateListeners.invokeAllCallbacks(HilitingBuffer::UpdateInfo(beginSelection.getPos(), endSelection.getPos()));
        }
    }
}

void BackliteBuffer::makeSecondarySelectionToPrimarySelection()
{
    if (hasSelection && isSecondarySelection) {
        isSecondarySelection = false;
        textData->flushPendingUpdates();
        updateListeners.invokeAllCallbacks(HilitingBuffer::UpdateInfo(beginSelection.getPos(), endSelection.getPos()));
    }
}

void BackliteBuffer::deactivateSelection()
{
    nextChangeListeners.invokeAllCallbacks();
    nextChangeListeners.clear();

    if (hasSelection) {
        hasSelection = false;
        isSecondarySelection = false;
        isSelectionAnchorAtBegin = true;
        long oldBeginPos = beginSelection.getPos();
        long oldEndPos   = endSelection.getPos();
        beginSelection = TextData::TextMark();
        endSelection   = TextData::TextMark();
        textData->flushPendingUpdates();
        updateListeners.invokeAllCallbacks(HilitingBuffer::UpdateInfo(oldBeginPos, oldEndPos));
    }
}


void BackliteBuffer::extendSelectionTo(long textPos)
{
    ASSERT(hasSelection);

    nextChangeListeners.invokeAllCallbacks();
    nextChangeListeners.clear();

    if (isSelectionAnchorAtBegin)
    {
        if (textPos < beginSelection.getPos())
        {
            isSelectionAnchorAtBegin = false;
            long oldEnd = endSelection.getPos();
            textData->moveMarkToPosOfMark(endSelection, beginSelection);
            textData->moveMarkToPos(beginSelection, textPos);
            
            textData->flushPendingUpdates();
            updateListeners.invokeAllCallbacks(HilitingBuffer::UpdateInfo(beginSelection.getPos(), oldEnd));
        }
        else
        {
            long oldEnd = endSelection.getPos();
            textData->moveMarkToPos(endSelection, textPos);
            
            textData->flushPendingUpdates();
            if (textPos > oldEnd) {
                updateListeners.invokeAllCallbacks(HilitingBuffer::UpdateInfo(oldEnd, textPos));
            } else {
                updateListeners.invokeAllCallbacks(HilitingBuffer::UpdateInfo(textPos, oldEnd));
            }
        }
    }
    else
    {
        if (textPos > endSelection.getPos())
        {
            isSelectionAnchorAtBegin = true;
            long oldBegin = beginSelection.getPos();
            textData->moveMarkToPosOfMark(beginSelection, endSelection);
            textData->moveMarkToPos(endSelection, textPos);

            textData->flushPendingUpdates();
            updateListeners.invokeAllCallbacks(HilitingBuffer::UpdateInfo(oldBegin, endSelection.getPos()));
        }
        else
        {
            long oldBegin = beginSelection.getPos();
            textData->moveMarkToPos(beginSelection, textPos);

            textData->flushPendingUpdates();
            if (textPos > oldBegin) {
                updateListeners.invokeAllCallbacks(HilitingBuffer::UpdateInfo(oldBegin, textPos));
            } else {
                updateListeners.invokeAllCallbacks(HilitingBuffer::UpdateInfo(textPos, oldBegin));
            }
        }
    }
}
