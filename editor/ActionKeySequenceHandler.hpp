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

namespace LucED
{

class ActionKeySequenceHandler
{
public:
    ActionKeySequenceHandler(RawPtr<GuiWidget> thisWidget)
        : thisWidget(thisWidget)
    {}
    
    void setActionKeyConfig(ActionKeyConfig::Ptr actionKeyConfig) {
        rootActionKeyConfig    = actionKeyConfig;
        currentActionKeyConfig = actionKeyConfig;
    }
    
    void reset() {
        currentActionKeyConfig = rootActionKeyConfig;
    }
    
    bool isWithinSequence() const {
        return currentActionKeyConfig != rootActionKeyConfig;
    }
    
    String getKeySequenceAsString() const {
        ASSERT(isWithinSequence());
        return combinationKeys;
    }
    

    bool handleKeyPress(const KeyPressEvent& keyPressEvent, 
                        RawPtr<GuiWidget>    focusedWidget = RawPtr<GuiWidget>());
    
private:
    RawPtr<GuiWidget>       thisWidget;
    ActionKeyConfig::Ptr    rootActionKeyConfig;
    ActionKeyConfig::Ptr    currentActionKeyConfig;

    KeyModifier             sequenceKeyModifier;
    String                  combinationKeys;
};

} // LucED

#endif // ACTION_KEY_SEQUENCE_HANDLER_HPP
