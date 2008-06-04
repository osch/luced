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

#ifndef COMMAND_OUTPUT_BOX_HPP
#define COMMAND_OUTPUT_BOX_HPP

#include "TopWin.hpp"
#include "EventDispatcher.hpp"
#include "Button.hpp"
#include "GuiLayoutColumn.hpp"
#include "DialogWin.hpp"
#include "PanelDialogWin.hpp"
#include "TopWinList.hpp"
#include "TextDisplayGuiCompound.hpp"

namespace LucED
{

class CommandOutputBox : public PanelDialogWin
{
public:
    typedef WeakPtr<CommandOutputBox> Ptr;
    
    static Ptr create(TopWin* referingWindow, TextData::Ptr textData)
    {
        return transferOwnershipTo(new CommandOutputBox(referingWindow, textData),
                                   referingWindow);
    }
    
    static Ptr create(TextData::Ptr textData)
    {
        return transferOwnershipTo(new CommandOutputBox(NULL, textData),
                                   TopWinList::getInstance());
    }
    
    virtual void requestCloseWindow(TopWin::CloseReason reason);

private:
    CommandOutputBox(TopWin* referingWindow, TextData::Ptr textData);
    
    
    void handleButtonPressed(Button* button, Button::ActivationVariant variant);

    Button::Ptr button1;
    Callback<>::Ptr cancelButtonCallback;
    Callback<>::Ptr closeCallback;
    bool wasClosed;
    
    TextDisplayGuiCompound::Ptr multiLineOut;
};

} // namespace LucED

#endif // COMMAND_OUTPUT_BOX_HPP
