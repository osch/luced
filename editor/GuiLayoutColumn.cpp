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

int GuiLayoutColumn::addElement(GuiElement::Ptr element, LayoutOptions layoutOptions)
{
    elements.append(LayoutedElement(element, layoutOptions));
    return elements.getLength() - 1;
}

void GuiLayoutColumn::addSpacer(int height)
{
    elements.append(LayoutedElement(GuiLayoutSpacer::create(0, 0, 0, height, INT_MAX, INT_MAX)));
}

void GuiLayoutColumn::addSpacer()
{
    elements.append(LayoutedElement(GuiLayoutSpacer::create(0, 0, 0, 0, INT_MAX, INT_MAX)));
}


GuiElement::Measures GuiLayoutColumn::getDesiredMeasures()
{
    Measures rslt =  GuiLayouter<VerticalAdapter>::getDesiredMeasures(elements);

    if (reportRasteringOptions.isSet(DO_NOT_REPORT_HORIZONTAL_RASTERING)) {
        rslt.incrWidth = 1;
    }
    if (reportRasteringOptions.isSet(DO_NOT_REPORT_VERTICAL_RASTERING)) {
        rslt.incrHeight = 1;
    }
    return rslt;
}

void GuiLayoutColumn::setPosition(Position p)
{
    bool doItAgain = true;
    
    while (doItAgain)
    {
        try
        {
            rowMeasures.clear();
            
            for (int i = 0; i < elements.getLength(); ++i)
            {
                Measures m = elements[i].getPtr()->getDesiredMeasures();
                if (!elements[i].getPtr()->isVisible()) {
                    m = Measures();
                }
                rowMeasures.append(m);
            }
            
            GuiLayouter<VerticalAdapter>::adjust(rowMeasures, p.h);
            
            int y = p.y;
            for (int i = 0; i < elements.getLength(); ++i)
            {
                if (elements[i].getPtr()->isVisible())
                {
                    Measures& m = rowMeasures[i];
                    
                    int h = m.bestHeight;
                    int w = p.w;
                    
                    LayoutOptions layoutOptions = elements[i].getLayoutOptions();
            
                    if (layoutOptions.isSet(LAYOUT_VERTICAL_RASTERING))
                    {
                        if (m.incrHeight > 1 && (h - m.minHeight) % m.incrHeight != 0)
                        {
                            h = m.minHeight + (h - m.minHeight) / m.incrHeight * m.incrHeight;
                        }
                    }
                    if (layoutOptions.isSet(LAYOUT_HORIZONTAL_RASTERING))
                    {
                        if (m.incrWidth > 1 && (w - m.minWidth) > 0 && (w - m.minWidth) % m.incrWidth != 0)
                        {
                            w = m.minWidth + (w - m.minWidth) / m.incrWidth * m.incrWidth;
                        }
                    }
                    elements[i].getPtr()->setPosition(Position(p.x, y, w, h));
            
                    y += m.bestHeight;
                }
            }
            doItAgain = false;
        }
        catch (GuiElement::DesiredMeasuresChangedException& ex) {
            doItAgain = true;
        }
    }
}

void GuiLayoutColumn::show()
{
    for (int i = 0; i < elements.getLength(); ++i) {
        elements[i].getPtr()->show();
    }
    GuiElement::show();
}

void GuiLayoutColumn::hide()
{
    for (int i = 0; i < elements.getLength(); ++i) {
        elements[i].getPtr()->hide();
    }
    GuiElement::hide();
}

