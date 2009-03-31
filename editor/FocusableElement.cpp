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

#include "FocusableElement.hpp"

using namespace LucED;

FocusableElement::~FocusableElement()
{
    if (nextFocusWidget.isValid() && nextFocusWidget->prevFocusWidget == this) {
        nextFocusWidget->prevFocusWidget = prevFocusWidget;
    }
    if (prevFocusWidget.isValid() && prevFocusWidget->nextFocusWidget == this) {
        prevFocusWidget->nextFocusWidget = nextFocusWidget;
    }
}


void FocusableElement::notifyAboutHotKeyEventForOtherWidget()
{}

void FocusableElement::treatLostHotKeyRegistration(const KeyMapping::Id& id)
{}

void FocusableElement::treatNewHotKeyRegistration(const KeyMapping::Id& id)
{}

void FocusableElement::treatHotKeyEvent(const KeyMapping::Id& id)
{}


/*void FocusableElement::requestFocusFor(RawPtr<FocusableElement> w)
{
    if (parent.isValid()) parent->requestFocusFor(w);
}

void FocusableElement::requestHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w)
{
    if (parent != Null) parent->requestHotKeyRegistrationFor(id, w);
}

void FocusableElement::requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w)
{
    if (parent != Null) parent->requestRemovalOfHotKeyRegistrationFor(id, w);
}
*/
