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

#ifndef BUTTON_H
#define BUTTON_H

#include <string>

#include "GuiWidget.h"
#include "OwningPtr.h"
#include "Callback.h"

namespace LucED {

using std::string;

class Button : public GuiWidget
{
public:
    typedef OwningPtr<Button> Ptr;
    
    static Ptr create(GuiWidget* parent, string buttonText) {
        return Ptr(new Button(parent, buttonText));
    }

    void setButtonPressedCallback(const Callback1<Button*>& callback) {
        pressedCallback = callback;
    }
    void setAsDefault(bool isDefault = true) {
        isDefaultButton = isDefault;
    }

    virtual bool processEvent(const XEvent *event);
    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position newPosition);
    
private:
    void drawButton();
    bool isMouseInsideButtonArea(int mouseX, int mouseY);
    
    Button(GuiWidget* parent, string buttonText);
    Position position;
    string buttonText;
    bool isButtonPressed;
    bool isMouseButtonPressed;
    bool isMouseOverButton;
    Callback1<Button*> pressedCallback;
    bool isDefaultButton;
};

} // namespace LucED

#endif // BUTTON_H
