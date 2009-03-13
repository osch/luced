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
#include "ActionMethodBinding.hpp"
#include "FocusableWidget.hpp"
                    
namespace LucED
{

class Button : public FocusableWidget
{
public:
    enum ActivationVariant {
        WAS_MOUSE_CLICK,
        WAS_HOT_KEY,
        WAS_DEFAULT_KEY
    };

    typedef OwningPtr<Button> Ptr;
    
    static Ptr create(GuiWidget* parent, String buttonText) {
        return Ptr(new Button(parent, buttonText));
    }
    
    void setButtonText(String buttonText);
    
    int getStandardHeight();

    void setButtonPressedCallback(Callback<Button*, ActivationVariant>::Ptr callback) {
        pressedCallback1 = callback;
        pressedCallback0.invalidate();
    }
    
    void setButtonPressedCallback(Callback<>::Ptr callback) {
        pressedCallback0 = callback;
        pressedCallback1.invalidate();
    }
    
    void setButtonRightClickedCallback(Callback<Button*, ActivationVariant>::Ptr callback) {
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
    
    
    virtual void treatFocusIn();
    virtual void treatFocusOut();

    virtual ProcessingResult processEvent(const XEvent* event);
    
    Measures getOwnDesiredMeasures();
    void setDesiredMeasures(Measures m);
    
    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position newPosition);
    
    virtual void treatLostHotKeyRegistration(const KeyMapping::Id& id);
    virtual void treatNewHotKeyRegistration(const KeyMapping::Id& id);
    virtual void treatHotKeyEvent(const KeyMapping::Id& id);
    
    void setAsDefaultButton(bool isDefault = true);
    
private:
    class Actions : public ActionMethodBinding<Actions>
    {
    public:
        typedef OwningPtr<Actions> Ptr;

        static Ptr create(RawPtr<Button> thisButton) {
            return Ptr(new Actions(thisButton));
        }
        void pressFocusedButton() {
            thisButton->emulateButtonPress(false, false);
        }
    private:
        Actions(RawPtr<Button> thisButton)
            : ActionMethodBinding<Actions>(this),
              thisButton(thisButton)
        {}
        RawPtr<Button> thisButton;
    };
    friend class ActionMethodBinding<Actions>;


    Button(GuiWidget* parent, String buttonText);

    void emulateButtonPress(bool isDefaultKey, bool isRightClicked);
    void drawButton();
    bool isMouseInsideButtonArea(int mouseX, int mouseY);
    
    Position position;
    String buttonText;
    bool isButtonPressed;
    bool isMouseButtonPressed;
    bool isMouseOverButton;
    
    Callback<Button*, ActivationVariant>::Ptr pressedCallback1;
    Callback<>::Ptr                           pressedCallback0;
    
    Callback<Button*, ActivationVariant>::Ptr rightClickedCallback1;
    Callback<>::Ptr                           rightClickedCallback0;
    
    bool isDefaultButton;
    bool isExplicitDefaultButton;
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
