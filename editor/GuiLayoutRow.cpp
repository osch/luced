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

#include "util.hpp"
#include "GuiLayoutRow.hpp"
#include "OwningPtr.hpp"
#include "GuiLayouter.hpp"
#include "GuiLayoutSpacer.hpp"

using namespace LucED;

class SpacerH : public GuiElement
{
public:
    typedef OwningPtr<SpacerH> Ptr;
    static Ptr create(int minWidth, int maxWidth) {
        return Ptr(new SpacerH(minWidth, maxWidth));
    }
    virtual Measures getDesiredMeasures() {
        return Measures(
                minWidth == INT_MAX ?  0 : minWidth,      0, 
                maxWidth,                                 0, 
                maxWidth == INT_MAX ? INT_MAX : maxWidth, 0);
    }
    virtual void setPosition(Position p) {}
private:
    SpacerH(int minWidth, int maxWidth) : minWidth(minWidth), maxWidth(maxWidth) {}
    int minWidth;
    int maxWidth;
};


void GuiLayoutRow::addElement(GuiElement::Ptr element)
{
    elements.append(element);
}

void GuiLayoutRow::addSpacer(int width)
{
    elements.append(SpacerH::create(width, width));
}

void GuiLayoutRow::addSpacer()
{
    elements.append(GuiLayoutSpacer::create(0, 0, 0, 0, INT_MAX, 0));
}

void GuiLayoutRow::addSpacer(int minWidth, int maxWidth)
{
    elements.append(SpacerH::create(minWidth, maxWidth));
}


GuiElement::Measures GuiLayoutRow::getDesiredMeasures()
{
    int bestWidth = 0;
    int bestHeight = 0;
    int minWidth = 0;
    int minHeight = 0;
    int maxWidth = 0;
    int maxHeight = 0;
    int incrWidth = 1;
    int incrHeight = 1;
    
    for (int i = 0; i < elements.getLength(); ++i)
    {
        Measures m = elements[i]->getDesiredMeasures();

        maximize(&minHeight,  m.minHeight);
        maximize(&bestHeight, m.bestHeight);
        maximize(&maxHeight,  m.maxHeight);
        
        addimize(&minWidth,  m.minWidth);
        addimize(&bestWidth, m.bestWidth);
        addimize(&maxWidth,  m.maxWidth);
        
        util::maximize(&incrWidth,  m.incrWidth);
        util::maximize(&incrHeight, m.incrHeight);
    }
    minWidth  = bestWidth  - ((bestWidth  - minWidth) / incrWidth)  * incrWidth;
    minHeight = bestHeight - ((bestHeight - minHeight)/ incrHeight) * incrHeight;
    return Measures(minWidth, minHeight, bestWidth, bestHeight, maxWidth, maxHeight,
                    incrWidth, incrHeight);
}

void GuiLayoutRow::setPosition(Position p)
{
    columnMeasures.clear();
    for (int i = 0; i < elements.getLength(); ++i)
    {
        columnMeasures.append(elements[i]->getDesiredMeasures());
    }
    
    GuiLayouter<HorizontalAdapter>::adjust(columnMeasures, p.w);

    int x = p.x;
    for (int i = 0; i < elements.getLength(); ++i)
    {
        Measures& m = columnMeasures[i];
        elements[i]->setPosition(Position(x, p.y, m.bestWidth, p.h));
        x += m.bestWidth;
    }
}
