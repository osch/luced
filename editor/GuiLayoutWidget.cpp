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

#include "GuiLayoutWidget.hpp"
#include "GuiRoot.hpp"

using namespace LucED;

GuiLayoutWidget::GuiLayoutWidget(const Measures& m, GuiColor color)
      : measures(m),
        color(color)
{}

void GuiLayoutWidget::processGuiWidgetCreatedEvent()
{
    getGuiWidget()->setBackgroundColor(color);
}

GuiElement::Measures GuiLayoutWidget::internalGetDesiredMeasures()
{
    return measures;
}


