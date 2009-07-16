/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#ifndef FOCUSABLE_CONTAINER_WIDGET_HPP
#define FOCUSABLE_CONTAINER_WIDGET_HPP

#include "FocusableWidget.hpp"
#include "FocusManager.hpp"

namespace LucED
{

class FocusableContainerWidget : public FocusableWidget,
                                 public FocusManager
{
public:
    // FocusManager methods
    virtual void requestHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w);
    virtual void requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w);
    virtual void requestFocusFor(RawPtr<FocusableElement> w);
    virtual void reportMouseClickFrom(RawPtr<FocusableElement> w);

public:
    // FocusableWidget methods
    virtual void adopt(RawPtr<GuiElement>   parentElement,
                       RawPtr<GuiWidget>    parentWidget,
                       RawPtr<FocusManager> focusManagerForThis,
                       RawPtr<FocusManager> focusManagerForChilds);
protected:
    FocusableContainerWidget(Visibility   defaultVisibility = VISIBLE, 
                             int          borderWidth = 0)
        : FocusableWidget(defaultVisibility, borderWidth)
    {}
    
private:
    RawPtr<FocusManager> focusManagerForThis;
    RawPtr<FocusManager> focusManagerForChilds;
};

} // namespace LucED

#endif // FOCUSABLE_CONTAINER_WIDGET_HPP
