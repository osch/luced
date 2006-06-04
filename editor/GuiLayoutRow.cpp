#include "util.h"
#include "GuiLayoutRow.h"
#include "OwningPtr.h"

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
    
    for (int i = 0; i < elements.getLength(); ++i)
    {
        Measures m = elements[i]->getDesiredMeasures();

        maximize(&minHeight,  m.minHeight);
        maximize(&bestHeight, m.bestHeight);
        maximize(&maxHeight,  m.maxHeight);
        
        addimize(&minWidth,  m.minWidth);
        addimize(&bestWidth, m.bestWidth);
        addimize(&maxWidth,  m.maxWidth);
        
    }
    return Measures(minWidth, minHeight, bestWidth, bestHeight, maxWidth, maxHeight);
}

void GuiLayoutRow::setPosition(Position p)
{
    int minWidth = 0;
    int bestWidth = 0;
    int maxWidth = 0;
    int numberFlex = 0;

    for (int i = 0; i < elements.getLength(); ++i)
    {
        Measures m = elements[i]->getDesiredMeasures();
        
        addimize(&minWidth,  m.minWidth);
        addimize(&bestWidth, m.bestWidth);
        addimize(&maxWidth,  m.maxWidth);

        if (m.maxWidth == -1) {
            ++numberFlex;
        }
    }
    if (bestWidth != -1 && bestWidth <= p.w) {
        minWidth = bestWidth;
    } else {
        numberFlex = 0;
    }
    int addNonFlexWidth = 0;
    int flexWidth = 0;
    if (minWidth < p.w) {
        if (numberFlex > 0) {
            flexWidth = ROUNDED_DIV(p.w - minWidth, numberFlex);
        } else {
            addNonFlexWidth = p.w - minWidth;
        }
    }
    int x = p.x;
    for (int i = 0; i < elements.getLength(); ++i)
    {
        Measures m = elements[i]->getDesiredMeasures();
        
        int w = 0;
        if (m.maxWidth == -1) {
            if (m.bestWidth != -1) {
                w = m.bestWidth + flexWidth;
            } else {
                w = flexWidth;
            }
        } else {
            if (bestWidth <= p.w) {
                w = m.bestWidth;
            } else {
                w = m.minWidth + ROUNDED_DIV(addNonFlexWidth * (m.bestWidth - m.minWidth), (bestWidth - minWidth));
            }
            if (w > m.bestWidth) {
                w = m.bestWidth;
            }
        }
        elements[i]->setPosition(Position(x, p.y, w, p.h));
        x += w;
    }
}
