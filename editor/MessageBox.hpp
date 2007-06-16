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

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include "TopWin.hpp"
#include "EventDispatcher.hpp"
#include "Button.hpp"
#include "GuiLayoutColumn.hpp"
#include "DialogWin.hpp"
#include "PanelDialogWin.hpp"
#include "TopWinList.hpp"

namespace LucED {

class MessageBoxParameter
{
public:
    MessageBoxParameter& setTitle(String title) {
        this->title = title;
        return *this;
    }
    MessageBoxParameter& setMessage(String message) {
        this->message = message;
        return *this;
    }
    MessageBoxParameter& setDefaultButton(String buttonLabel, const Callback0& callback) {
        defaultButtonLabel = buttonLabel;
        defaultButtonCallback = callback;
        return *this;
    }
    MessageBoxParameter& setAlternativeButton(String buttonLabel, const Callback0& callback) {
        alternativeButtonLabel = buttonLabel;
        alternativeButtonCallback = callback;
        return *this;
    }
    MessageBoxParameter& setCancelButton(String buttonLabel) {
        cancelButtonLabel = buttonLabel;
        return *this;
    }
    
    MessageBoxParameter& setCancelButton(String buttonLabel, const Callback0& callback) {
        cancelButtonLabel = buttonLabel;
        cancelButtonCallback = callback;
        return *this;
    }
    
private:
    friend class MessageBox;

    String title;
    String message;

    String defaultButtonLabel;
    Callback0   defaultButtonCallback;

    String alternativeButtonLabel;
    Callback0   alternativeButtonCallback;

    String cancelButtonLabel;
    Callback0   cancelButtonCallback;
};

class MessageBox : public PanelDialogWin
{
public:
    typedef WeakPtr<MessageBox> Ptr;
    
    static MessageBox::Ptr create(TopWin* referingWindow, MessageBoxParameter p)
    {
        return transferOwnershipTo(new MessageBox(referingWindow, p),
                                   referingWindow);
    }
    
    static MessageBox::Ptr create(MessageBoxParameter p)
    {
        return transferOwnershipTo(new MessageBox(NULL, p),
                                   TopWinList::getInstance());
    }
    
    void closeWithoutAnyButtonActions() {
        PanelDialogWin::requestCloseWindow();
    }
    
private:
    MessageBox(TopWin* referingWindow, MessageBoxParameter p);
    
    virtual void requestCloseWindow();
    
    void handleButtonPressed(Button* button);

    Button::Ptr button1;
    Button::Ptr button2;
    Button::Ptr button3;
    Callback0 defaultButtonCallback;
    Callback0 alternativeButtonCallback;
    Callback0 cancelButtonCallback;
};

} // namespace LucED

#endif // MESSAGEBOX_H
