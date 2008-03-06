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

#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "String.hpp"

#include "GuiWidget.hpp"
#include "OwningPtr.hpp"
#include "Callback.hpp"
#include "TimeVal.hpp"

namespace LucED {



class CheckBox : public GuiWidget
{
public:
    typedef OwningPtr<CheckBox> Ptr;
    
    static Ptr create(GuiWidget* parent, String buttonText) {
        return Ptr(new CheckBox(parent, buttonText));
    }
    

    void setButtonPressedCallback(Callback<CheckBox*>::Ptr callback) {
        pressedCallback = callback;
    }
    
    virtual void treatFocusIn();
    virtual void treatFocusOut();

    virtual ProcessingResult processEvent(const XEvent *event);
    virtual ProcessingResult processKeyboardEvent(const XEvent *event);
    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position newPosition);
    virtual bool isFocusable() { return true; }
    virtual FocusType getFocusType() { return NORMAL_FOCUS; }
    
    virtual void treatLostHotKeyRegistration(const KeyMapping::Id& id);
    virtual void treatNewHotKeyRegistration(const KeyMapping::Id& id);
    virtual void treatHotKeyEvent(const KeyMapping::Id& id);
    
    
    void setChecked(bool checked);
    
    bool isChecked() const;
    
private:
    void draw();
    bool isMouseInsideButtonArea(int mouseX, int mouseY);
    
    CheckBox(GuiWidget* parent, String buttonText);
    Position position;
    String buttonText;
    bool isBoxChecked;
    bool isMouseButtonPressed;
    bool isMouseOverButton;
    Callback<CheckBox*>::Ptr pressedCallback;
    bool hasFocus;
    bool hasHotKey;
    bool showHotKey;
    char hotKeyChar;
    int hotKeyPixX;
    int hotKeyPixW;
};

} // namespace LucED

#endif // CHECKBOX_H
