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

#include <limits.h>

#include "util.h"
#include "GuiLayoutColumn.h"
#include "GuiLayoutSpacer.h"
#include "OwningPtr.h"
#include "GuiLayouter.h"

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
    int bestWidth = 0;
    int bestHeight = 0;
    int minWidth = 0;
    int minHeight = 0;
    int maxWidth = 0;
    int maxHeight = 0;
    int incrWidth = 1;
    int incrHeight = INT_MAX;
    
    for (int i = 0; i < elements.getLength(); ++i)
    {
        Measures m = elements[i]->getDesiredMeasures();

        addimize(&minHeight,  m.minHeight);
        addimize(&bestHeight, m.bestHeight);
        addimize(&maxHeight,  m.maxHeight);
        
        maximize(&minWidth,  m.minWidth);
        maximize(&bestWidth, m.bestWidth);
        maximize(&maxWidth,  m.maxWidth);

        maximize(&incrWidth,  m.incrWidth);
        if (m.maxHeight > m.bestHeight && m.incrHeight < incrHeight) {
            incrHeight = m.incrHeight;
        }
    }
    if (incrHeight == INT_MAX) {
        incrHeight = 1;
    }
    minWidth  = bestWidth  - ((bestWidth  - minWidth) / incrWidth)  * incrWidth;
    minHeight = bestHeight - ((bestHeight - minHeight)/ incrHeight) * incrHeight;
    return Measures(minWidth, minHeight, bestWidth, bestHeight, maxWidth, maxHeight,
                    incrWidth, incrHeight);
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
        elements[i]->setPosition(Position(p.x, y, p.w, m.bestHeight));
        y += m.bestHeight;
    }
}
