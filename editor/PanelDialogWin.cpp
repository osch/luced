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

#include "PanelDialogWin.hpp"
#include "GlobalConfig.hpp"
#include "GuiLayoutRow.hpp"
#include "GuiLayoutSpacer.hpp"
#include "TextData.hpp"
#include "GlobalConfig.hpp"
#include "LabelWidget.hpp"
#include "DialogPanel.hpp"

using namespace LucED;

PanelDialogWin::PanelDialogWin(TopWin* referingWindow)
    : DialogWin(referingWindow)
{
    dialogPanel = DialogPanel::create(this);
    DialogWin::setRootElement(dialogPanel);
    dialogPanel->show();
}


void PanelDialogWin::treatFocusIn()
{
    DialogWin::treatFocusIn();
    dialogPanel->treatFocusIn();
}

void PanelDialogWin::treatFocusOut()
{
    DialogWin::treatFocusOut();
    dialogPanel->treatFocusOut();
}



void PanelDialogWin::requestHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w)
{
    dialogPanel->requestHotKeyRegistrationFor(id, w);
}

void PanelDialogWin::requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w)
{
    dialogPanel->requestRemovalOfHotKeyRegistrationFor(id, w);
}

