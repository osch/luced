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

#include <limits.h>

#include "util.hpp"
#include "GuiLayoutColumn.hpp"
#include "GuiLayoutSpacer.hpp"
#include "OwningPtr.hpp"
#include "GuiLayouter.hpp"

using namespace LucED;

void GuiLayoutColumn::addSpacer(int height)
{
    elements.append(GuiLayoutSpacer::create(0, 0, 0, height, 0, INT_MAX));
}

void GuiLayoutColumn::addSpacer()
{
    elements.append(GuiLayoutSpacer::create(0, 0, 0, 0, 0, INT_MAX));
}


GuiElement::Measures GuiLayoutColumn::getDesiredMeasures()
{
    return GuiLayouter<VerticalAdapter>::getDesiredMeasures(elements);
}

void GuiLayoutColumn::setPosition(Position p)
{
    rowMeasures.clear();
    
    for (int i = 0; i < elements.getLength(); ++i)
    {
        rowMeasures.append(elements[i]->getDesiredMeasures());
    }
    
    GuiLayouter<VerticalAdapter>::adjust(rowMeasures, p.h);
    
    int y = p.y;
    for (int i = 0; i < elements.getLength(); ++i)
    {
        Measures& m = rowMeasures[i];
        
        int h = m.bestHeight;
        int w = p.w;
        
        if (m.onlyRasteredValues)
        {
            if (m.incrHeight > 1 && (h - m.minHeight) % m.incrHeight != 0)
            {
                h = m.minHeight + (h - m.minHeight) / m.incrHeight * m.incrHeight;
            }
            if (   m.onlyRasteredValues
                && m.incrWidth > 1 && (w - m.minWidth) > 0 && (w - m.minWidth) % m.incrWidth != 0)
            {
                w = m.minWidth + (w - m.minWidth) / m.incrWidth * m.incrWidth;
            }
        }
        elements[i]->setPosition(Position(p.x, y, w, h));

        y += m.bestHeight;
    }
}
