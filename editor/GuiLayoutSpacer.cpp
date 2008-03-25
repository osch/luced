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

#include "GuiLayoutSpacer.hpp"
#include "GuiLayoutColumn.hpp"
#include "GuiLayoutRow.hpp"

using namespace LucED;


GuiLayoutSpacerFrame::GuiLayoutSpacerFrame(GuiElement::Ptr member, int thickness)
{
    GuiLayoutColumn::Ptr column0 = GuiLayoutColumn::create();
    GuiLayoutRow::Ptr    row0    = GuiLayoutRow::create();
    this->root = column0;

    column0->addElement(GuiLayoutSpacer::create(0, 0, 0, thickness, INT_MAX, thickness));
    column0->addElement(row0);
    column0->addElement(GuiLayoutSpacer::create(0, 0, 0, thickness, INT_MAX, thickness));
    
    row0->addElement(GuiLayoutSpacer::create(0, 0, thickness, 0, thickness, INT_MAX));
    row0->addElement(member);
    row0->addElement(GuiLayoutSpacer::create(0, 0, thickness, 0, thickness, INT_MAX));
}


GuiElement::Measures GuiLayoutSpacerFrame::getDesiredMeasures()
{
    Measures rslt = root->getDesiredMeasures();
    return rslt;
}


void GuiLayoutSpacerFrame::setPosition(Position p)
{
    Measures desired = root->getDesiredMeasures();
    if (desired.maxHeight < p.h) {
        int d = p.h - desired.maxHeight;
        p.y += d/2;
        p.h -= d/2;
    }
    if (desired.maxWidth < p.w) {
        int d = p.w - desired.maxWidth;
        p.x += d/2;
        p.w -= d/2;
    }
    root->setPosition(p);
}


