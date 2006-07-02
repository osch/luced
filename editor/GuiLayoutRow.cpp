#include "util.h"
#include "GuiLayoutRow.h"
#include "OwningPtr.h"
#include "GuiLayouter.h"
#include "GuiLayoutSpacer.h"

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
                minWidth == -1 ?  0 : minWidth, 0, 
                maxWidth, -1, 
                maxWidth == -1 ? -1 : maxWidth, 0);
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
    elements.append(GuiLayoutSpacer::create(0, 0, 0, 0, -1, 0));
}

void GuiLayoutRow::addSpacer(int minWidth, int maxWidth)
{
    elements.append(SpacerH::create(minWidth, maxWidth));
}


static void maximize(int *a, int b)
{
    if (*a != -1) {
        if (b == -1) {
            *a = -1;
        } else {
            util::maximize(a, b);
        }
    }
}

static void addimize(int *a, int b)
{
    if (*a != -1) {
        if (b == -1) {
            *a = -1;
        } else {
            *a += b;
        }
    }
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
