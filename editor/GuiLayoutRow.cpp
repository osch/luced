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


void GuiLayoutRow::addElement(GuiElement::Ptr element, LayoutOptions layoutOptions)
{
    element->setLayoutOptions(layoutOptions);
    childElements.append(element);
}

void GuiLayoutRow::addSpacer(int width)
{
    childElements.append(SpacerH::create(width, width));
}

void GuiLayoutRow::addSpacer()
{
    childElements.append(GuiLayoutSpacer::create(0, 0, 0, 0, INT_MAX, 0));
}

void GuiLayoutRow::addSpacer(int minWidth, int maxWidth)
{
    childElements.append(SpacerH::create(minWidth, maxWidth));
}


GuiElement::Measures GuiLayoutRow::getDesiredMeasures()
{
    Measures rslt = GuiLayouter<HorizontalAdapter>::getDesiredMeasures(childElements);

    if (reportRasteringOptions.isSet(DO_NOT_REPORT_HORIZONTAL_RASTERING)) {
        rslt.incrWidth = 1;
    }
    if (reportRasteringOptions.isSet(DO_NOT_REPORT_VERTICAL_RASTERING)) {
        rslt.incrHeight = 1;
    }
    return rslt;
}

void GuiLayoutRow::setPosition(Position p)
{
    bool doItAgain = true;
    
    while (doItAgain)
    {
        try
        {
            columnMeasures.clear();
            for (int i = 0; i < childElements.getLength(); ++i)
            {
                Measures m = childElements[i]->getDesiredMeasures();
                if (!childElements[i]->isVisible()) {
                    m = Measures();
                }
                columnMeasures.append(m);
            }
            
            GuiLayouter<HorizontalAdapter>::adjust(columnMeasures, p.w);
        
            int x = p.x;
            for (int i = 0; i < childElements.getLength(); ++i)
            {
                if (childElements[i]->isVisible())
                {
                    Measures& m = columnMeasures[i];
            
                    int h = p.h;
                    int w = m.bestWidth;
              
                    LayoutOptions layoutOptions = childElements[i]->getLayoutOptions();
                    
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
            
                    childElements[i]->setPosition(Position(x, p.y, w, h));
            
                    x += m.bestWidth;
                }
            }
            doItAgain = false;
        }
        catch (GuiElement::DesiredMeasuresChangedException& ex) {
            doItAgain = true;
        }
    }
}
void GuiLayoutRow::show()
{
    for (int i = 0; i < childElements.getLength(); ++i) {
        childElements[i]->show();
    }
    GuiElement::show();
}

void GuiLayoutRow::hide()
{
    for (int i = 0; i < childElements.getLength(); ++i) {
        childElements[i]->hide();
    }
    GuiElement::hide();
}

