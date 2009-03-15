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

#ifndef ACTION_KEY_SEQUENCE_HANDLER_HPP
#define ACTION_KEY_SEQUENCE_HANDLER_HPP

#include "RawPtr.hpp"
#include "GuiWidget.hpp"
#include "ActionKeyConfig.hpp"
#include "String.hpp"
#include "KeyModifier.hpp"
#include "KeyId.hpp"
#include "KeyPressEvent.hpp"
#include "GlobalConfig.hpp"
#include "ActionMethodContainer.hpp"
#include "KeyActionHandler.hpp"

namespace LucED
{

class ActionKeySequenceHandler
{
public:
    ActionKeySequenceHandler(WeakPtr<ActionMethodContainer> theseActions)
        : theseActions(theseActions),
          hasJustQuitSequenceFlag(false),
          hasJustEnteredSequenceFlag(false),

          rootActionKeyConfig(GlobalConfig::getInstance()->getActionKeyConfig()),
          currentActionKeyConfig(rootActionKeyConfig)
    {}
    
    
    void reset() {
        hasJustQuitSequenceFlag    = (currentActionKeyConfig != rootActionKeyConfig);
        hasJustEnteredSequenceFlag = false;

        rootActionKeyConfig        = GlobalConfig::getInstance()->getActionKeyConfig();
        currentActionKeyConfig     = rootActionKeyConfig;
    }
    
    bool isWithinSequence() const {
        return currentActionKeyConfig != rootActionKeyConfig;
    }
    
    bool hasJustQuitSequence() const {
        return hasJustQuitSequenceFlag;
    }
    
    bool hasJustEnteredSequence() const {
        return hasJustEnteredSequenceFlag;
    }
    
    String getKeySequenceAsString() const {
        ASSERT(isWithinSequence());
        return combinationKeys;
    }
    
    bool handleKeyPress(const KeyPressEvent&    keyPressEvent, 
                        RawPtr<KeyActionHandler> focusedKeyActionHandler = Null);
    
private:
    WeakPtr<ActionMethodContainer> theseActions;
    ActionKeyConfig::Ptr           rootActionKeyConfig;
    ActionKeyConfig::Ptr           currentActionKeyConfig;

    KeyModifier             sequenceKeyModifier;
    String                  combinationKeys;
    bool                    hasJustQuitSequenceFlag;
    bool                    hasJustEnteredSequenceFlag;
};

} // LucED

#endif // ACTION_KEY_SEQUENCE_HANDLER_HPP
