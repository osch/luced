/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#include "GuiLayoutWidget.hpp"
#include "GuiRoot.hpp"

using namespace LucED;

GuiLayoutWidget::GuiLayoutWidget(GuiWidget* parent, const Measures& m)
      : GuiWidget(parent, 0, 0, 1, 1, 0),
        position(0, 0, 1, 1),
        measures(m)
{
    setBackgroundColor(getGuiRoot()->getGuiColor01());
    this->show();
}

GuiElement::Measures GuiLayoutWidget::getDesiredMeasures()
{
    return measures;
}


void GuiLayoutWidget::setPosition(Position newPosition)
{
    if (position != newPosition) {
        GuiWidget::setPosition(newPosition);
        this->position = newPosition;
    }
}

