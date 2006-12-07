/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch at luced dot de
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

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include "TopWin.h"
#include "EventDispatcher.h"
#include "Button.h"
#include "GuiLayoutColumn.h"
#include "DialogWin.h"
#include "Slot.h"
#include "PanelDialogWin.h"

namespace LucED {

class MessageBoxParameter
{
public:
    MessageBoxParameter& setTitle(std::string title) {
        this->title = title;
        return *this;
    }
    MessageBoxParameter& setMessage(std::string message) {
        this->message = message;
        return *this;
    }
    MessageBoxParameter& setDefaultButton(std::string buttonLabel, const Callback0& callback) {
        defaultButtonLabel = buttonLabel;
        defaultButtonCallback = callback;
        return *this;
    }
    MessageBoxParameter& setCancelButton(std::string buttonLabel) {
        cancelButtonLabel = buttonLabel;
        return *this;
    }
    
private:
    friend class MessageBox;
    std::string title;
    std::string message;
    std::string defaultButtonLabel;
    Callback0   defaultButtonCallback;
    std::string cancelButtonLabel;
};

class MessageBox : public PanelDialogWin
{
public:
    typedef WeakPtr<MessageBox> Ptr;
    
    static MessageBox::Ptr create(TopWin* referingWindow, 
                                  MessageBoxParameter p) {
        return transferOwnershipTo(
                new MessageBox(referingWindow, p),
                referingWindow);
    }
    
private:
    MessageBox(TopWin* referingWindow, MessageBoxParameter p);
    
    void handleButtonPressed(Button* button);

    Button::Ptr button1;
    Button::Ptr button2;
    Callback0 defaultButtonCallback;
};

} // namespace LucED

#endif // MESSAGEBOX_H
