/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch@luced.de
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

#include "BackliteBuffer.h"


using namespace LucED;

BackliteBuffer::BackliteBuffer(TextData::Ptr textData)
      : textData(textData),
        startPos(0),
        hasSelection(false)
{
}

byte BackliteBuffer::getNonBufferedBackground(long textPos)
{
    return 0;
}

void BackliteBuffer::registerUpdateListener(const Callback1<HilitingBuffer::UpdateInfo>& updateCallback)
{
    updateListeners.registerCallback(updateCallback);
}

void BackliteBuffer::activateSelection(long textPos)
{
    if (hasSelection) {
        deactivateSelection();
    }
    hasSelection = true;
    isSelectionAnchorAtBegin = true;
    beginSelection = textData->createNewMark();
    beginSelection.moveToPos(textPos);
    endSelection   = textData->createNewMark(beginSelection);
}


void BackliteBuffer::deactivateSelection()
{
    if (hasSelection) {
        hasSelection = false;
        isSelectionAnchorAtBegin = true;
        long oldBeginPos = beginSelection.getPos();
        long oldEndPos   = endSelection.getPos();
        beginSelection = TextData::TextMark();
        endSelection   = TextData::TextMark();
        updateListeners.invokeAllCallbacks(HilitingBuffer::UpdateInfo(oldBeginPos, oldEndPos));
    }
}


void BackliteBuffer::extendSelectionTo(long textPos)
{
    if (isSelectionAnchorAtBegin)
    {
        if (textPos < beginSelection.getPos())
        {
            isSelectionAnchorAtBegin = false;
            long oldEnd = endSelection.getPos();
            textData->moveMarkToPosOfMark(endSelection, beginSelection);
            textData->moveMarkToPos(beginSelection, textPos);
            
            updateListeners.invokeAllCallbacks(HilitingBuffer::UpdateInfo(beginSelection.getPos(), oldEnd));
        }
        else
        {
            long oldEnd = endSelection.getPos();
            textData->moveMarkToPos(endSelection, textPos);
            
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

            updateListeners.invokeAllCallbacks(HilitingBuffer::UpdateInfo(oldBegin, endSelection.getPos()));
        }
        else
        {
            long oldBegin = beginSelection.getPos();
            textData->moveMarkToPos(beginSelection, textPos);

            if (textPos > oldBegin) {
                updateListeners.invokeAllCallbacks(HilitingBuffer::UpdateInfo(oldBegin, textPos));
            } else {
                updateListeners.invokeAllCallbacks(HilitingBuffer::UpdateInfo(textPos, oldBegin));
            }
        }
    }
}
