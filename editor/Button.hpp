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

#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "String.hpp"

#include "GuiWidget.hpp"
#include "OwningPtr.hpp"
#include "Callback.hpp"
#include "TimeVal.hpp"

namespace LucED {



class Button : public GuiWidget
{
public:
    typedef OwningPtr<Button> Ptr;
    
    static Ptr create(GuiWidget* parent, String buttonText) {
        return Ptr(new Button(parent, buttonText));
    }
    
    void setButtonText(String buttonText);
    
    int getStandardHeight();

    void setButtonPressedCallback(Callback<Button*>::Ptr callback) {
        pressedCallback1 = callback;
        pressedCallback0.invalidate();
    }
    
    void setButtonPressedCallback(Callback<>::Ptr callback) {
        pressedCallback0 = callback;
        pressedCallback1.invalidate();
    }
    
    void setButtonRightClickedCallback(Callback<Button*>::Ptr callback) {
        rightClickedCallback1 = callback;
        rightClickedCallback0.invalidate();
    }
    
    bool doesReactOnRightClick() const {
        return rightClickedCallback0.isValid() || rightClickedCallback1.isValid();
    }
    
    void setButtonRightClickedCallback(Callback<>::Ptr callback) {
        rightClickedCallback0 = callback;
        rightClickedCallback1.invalidate();
    }
    
    void setButtonDefaultKeyCallback(Callback<Button*>::Ptr callback) {
        buttonDefaultKeyCallback = callback;
    }
    
    virtual void treatFocusIn();
    virtual void treatFocusOut();

    virtual ProcessingResult processEvent(const XEvent *event);
    virtual ProcessingResult processKeyboardEvent(const XEvent *event);
    
    Measures getOwnDesiredMeasures();
    void setDesiredMeasures(Measures m);
    
    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position newPosition);
    virtual bool isFocusable() { return true; }
    virtual FocusType getFocusType() { return NORMAL_FOCUS; }
    
    virtual void treatLostHotKeyRegistration(const KeyMapping::Id& id);
    virtual void treatNewHotKeyRegistration(const KeyMapping::Id& id);
    virtual void treatHotKeyEvent(const KeyMapping::Id& id);
    
    void setAsDefaultButton(bool isDefault = true);
    
    
private:
    void emulateButtonPress(bool isDefaultKey, bool isRightClicked);
    void drawButton();
    bool isMouseInsideButtonArea(int mouseX, int mouseY);
    
    Button(GuiWidget* parent, String buttonText);
    Position position;
    String buttonText;
    bool isButtonPressed;
    bool isMouseButtonPressed;
    bool isMouseOverButton;
    
    Callback<Button*>::Ptr pressedCallback1;
    Callback<>::Ptr        pressedCallback0;
    
    Callback<Button*>::Ptr rightClickedCallback1;
    Callback<>::Ptr        rightClickedCallback0;
    
    
    Callback<Button*>::Ptr buttonDefaultKeyCallback;

    bool isDefaultButton;
    bool isExplicitDefaultButton;
    bool hasFocusFlag;
    TimeVal earliestButtonReleaseTime;
    bool hasHotKeyFlag;
    bool showHotKeyFlag;
    char hotKeyChar;
    int hotKeyPixX;
    int hotKeyPixW;
    bool hasForcedMeasuresFlag;
    Measures forcedMeasures;
};

} // namespace LucED

#endif // BUTTON_HPP
