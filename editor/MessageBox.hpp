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

#include "MessageBoxQueue.hpp"
#include "MessageBoxParameter.hpp"

#ifndef MESSAGE_BOX_HPP
#define MESSAGE_BOX_HPP

#include "TopWin.hpp"
#include "EventDispatcher.hpp"
#include "Button.hpp"
#include "GuiLayoutColumn.hpp"
#include "DialogWin.hpp"
#include "PanelDialogWin.hpp"
#include "TopWinList.hpp"

namespace LucED
{

class MessageBox : public PanelDialogWin
{
public:
    typedef WeakPtr<MessageBox> Ptr;
    
    static MessageBox::Ptr create(TopWin* referingWindow, const MessageBoxParameter& p)
    {
        return transferOwnershipTo(new MessageBox(referingWindow, p),
                                   referingWindow);
    }
    
    static MessageBox::Ptr create(MessageBoxParameter p)
    {
        return transferOwnershipTo(new MessageBox(NULL, p),
                                   TopWinList::getInstance());
    }
    
    virtual ProcessingResult processKeyboardEvent(const XEvent *event);
    
    virtual void requestCloseWindow(TopWin::CloseReason reason);

    virtual void show();

private:
    MessageBox(TopWin* referingWindow, const MessageBoxParameter& p);
    
    
    void handleButtonPressed(Button* button);

    Button::Ptr button1;
    Button::Ptr button2;
    Button::Ptr button3;
    Callback<>::Ptr defaultButtonCallback;
    Callback<>::Ptr alternativeButtonCallback;
    Callback<>::Ptr cancelButtonCallback;
    Callback<>::Ptr closeCallback;
    bool wasClosed;
    KeyMapping::Ptr keyMapping;

    Callback<TopWin*>::Ptr invokeNotifyCallback;
    Callback<TopWin*>::Ptr closeNotifyCallback;
};

} // namespace LucED

#endif // MESSAGE_BOX_HPP
