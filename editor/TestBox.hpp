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

#ifndef TEST_BOX_HPP
#define TEST_BOX_HPP

#include "TopWin.hpp"
#include "EventDispatcher.hpp"
#include "Button.hpp"
#include "GuiLayoutColumn.hpp"
#include "DialogWin.hpp"
#include "PanelDialogWin.hpp"
#include "TopWinList.hpp"
#include "MultiLineOutputWidget.hpp"

namespace LucED
{

class TestBox : public PanelDialogWin
{
public:
    typedef WeakPtr<TestBox> Ptr;
    
    static Ptr create(TopWin* referingWindow, TextData::Ptr textData)
    {
        return transferOwnershipTo(new TestBox(referingWindow, textData),
                                   referingWindow);
    }
    
    static Ptr create(TextData::Ptr textData)
    {
        return transferOwnershipTo(new TestBox(NULL, textData),
                                   TopWinList::getInstance());
    }
    
    virtual void requestCloseWindow();

private:
    TestBox(TopWin* referingWindow, TextData::Ptr textData);
    
    
    void handleButtonPressed(Button* button);

    Button::Ptr button1;
    Callback<>::Ptr defaultButtonCallback;
    Callback<>::Ptr alternativeButtonCallback;
    Callback<>::Ptr cancelButtonCallback;
    Callback<>::Ptr closeCallback;
    bool wasClosed;
    
    MultiLineOutputWidget::Ptr multiLineOut;
};

} // namespace LucED

#endif // TEST_BOX_HPP
