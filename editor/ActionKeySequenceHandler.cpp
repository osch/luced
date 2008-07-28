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

#include "ActionKeySequenceHandler.hpp"

using namespace LucED;


bool ActionKeySequenceHandler::handleKeyPress(const KeyPressEvent& keyPressEvent, 
                                              RawPtr<GuiWidget>    focusedWidget)
{
    hasJustQuitSequenceFlag    = false;
    hasJustEnteredSequenceFlag = false;
    
    bool keyProcessed = false;

    if (   !isWithinSequence() && focusedWidget.isValid()
        && focusedWidget->handleHighPriorityKeyPress(keyPressEvent))
    {
        return true;
    }

    ActionKeyConfig::Node::Ptr node;
    
    node = currentActionKeyConfig->find((isWithinSequence() ? sequenceKeyModifier
                                                            : keyPressEvent.getKeyModifier()), 
                                        keyPressEvent.getKeyId());

    if (node.isValid())
    {
        if (node->hasActionIds())
        {
            ActionKeyConfig::ActionIds::Ptr actionIds = node->getActionIds();

            for (int i = 0, n = actionIds->getLength(); i < n; ++i)
            {
                if (   (focusedWidget.isValid() && focusedWidget->invokeActionMethod(actionIds->get(i)))
                    || (                              thisWidget->invokeActionMethod(actionIds->get(i))))
                {
                    reset();
                    keyProcessed = true;
                    break;
                }
            }
        }
        if (!keyProcessed && node->hasNext())
        {
            if (!isWithinSequence()) {
                sequenceKeyModifier = keyPressEvent.getKeyModifier();
                combinationKeys = String() << (  sequenceKeyModifier.toString().getLength() > 0
                                               ? (sequenceKeyModifier.toString() << "+")
                                               : "")
                                           << keyPressEvent.getKeyId().toString().toUpper();
            } else {
                combinationKeys << "," << keyPressEvent.getKeyId().toString().toUpper();
            }
            currentActionKeyConfig = node->getNext();
            hasJustEnteredSequenceFlag = true;
            keyProcessed = true;
        }
    }
    if (   !keyProcessed
        && !isWithinSequence() && focusedWidget.isValid()
        && focusedWidget->handleLowPriorityKeyPress(keyPressEvent))
    {
        keyProcessed = true;
    }
    return  keyProcessed;   
}

