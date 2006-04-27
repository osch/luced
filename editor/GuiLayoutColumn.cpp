#include "util.h"
#include "GuiLayoutColumn.h"

using namespace LucED;

class SpacerV : public GuiElement
{
public:
    typedef HeapObjectPtr<SpacerV> Ptr;
    static Ptr create(int minHeight, int maxHeight) {
        return Ptr(new SpacerV(minHeight, maxHeight));
    }
    virtual Measures getDesiredMeasures() {
        return Measures(
                -1, minHeight == -1 ?  0 : minHeight, 
                 0, maxHeight == -1 ? -1 : maxHeight, 
                -1, maxHeight == -1 ? -1 : maxHeight);
    }
    virtual void setPosition(Position p) {}
private:
    SpacerV(int minHeight, int maxHeight) : minHeight(minHeight), maxHeight(maxHeight) {}
    int minHeight;
    int maxHeight;
};


void GuiLayoutColumn::addElement(GuiElement::Ptr element)
{
    elements.append(element);
}

void GuiLayoutColumn::addSpacer(int height)
{
    elements.append(SpacerV::create(height, height));
}

void GuiLayoutColumn::addSpacer(int minHeight, int maxHeight)
{
    elements.append(SpacerV::create(minHeight, maxHeight));
}


GuiElement::Measures GuiLayoutColumn::getDesiredMeasures()
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
        
        if (m.bestHeight == -1) {
            bestHeight += m.minHeight;
            maxHeight = -1;
        } else {
            bestHeight += m.bestHeight;
            if (maxHeight != -1) {
                maxHeight += m.maxHeight;
            }
        }
        if (m.bestWidth != -1) {
            util::maximize(&bestWidth, m.bestWidth);
        }
        minHeight += m.minHeight;
        util::maximize(&minWidth, m.minWidth);
    }
    return Measures(minWidth, minHeight, bestWidth, bestHeight, bestWidth, bestHeight);
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

static void ensureBetween(int *a, int min, int max)
{
    if (*a != -1) {
        if (*a < min) {
            *a = min;
        }
        if (max != -1) {
            if (*a > max) {
                *a = max;
            }
        }
    } else {
        if (max != -1) {
            *a = max;
        }
    }
}

void GuiLayoutColumn::setPosition(Position p)
{
printf("GuiLayoutColumn::setPosition %d %d %d %d\n", p.x, p.y, p.w, p.h);

    int bestHeight = 0;
    int minHeight = 0;
    int maxHeight = 0;
    int numberFlex = 0;

    for (int i = 0; i < elements.getLength(); ++i)
    {
        Measures m = elements[i]->getDesiredMeasures();
        
        addimize(&minHeight,  m.minHeight);
        addimize(&bestHeight, m.bestHeight);
        addimize(&maxHeight,  m.maxHeight);

printf("GuiLayoutColumn::setPosition: bestHeight(%d)=%d\n", i, m.bestHeight);
        
        if (m.maxHeight == -1) {
            ++numberFlex;
        }
    }
printf("GuiLayoutColumn::setPosition: numberFlexA %d, bestHeight %d\n", numberFlex, bestHeight);
    if (bestHeight != -1 && bestHeight <= p.h) {
        minHeight = bestHeight;
    } else {
        numberFlex = 0;
    }
    int addNonFlexHeight = 0;
    int flexHeight = 0;
    if (minHeight < p.h) {
        if (numberFlex > 0) {
            flexHeight = ROUNDED_DIV(p.h - minHeight, numberFlex);
        } else {
            addNonFlexHeight = p.h - minHeight;
        }
    }
printf("GuiLayoutColumn::setPosition: numberFlexB %d, flexHeight %d\n", numberFlex, flexHeight);

    int y = p.y;
    for (int i = 0; i < elements.getLength(); ++i)
    {
        Measures m = elements[i]->getDesiredMeasures();
        
        int h = 0;
        if (m.maxHeight == -1) {
            if (m.bestHeight != -1) {
                h = m.bestHeight + flexHeight;
            } else {
                h = flexHeight;
            }
        } else {
            if (bestHeight <= p.h) {
                h = m.bestHeight;
            } else {
                h = m.minHeight + ROUNDED_DIV(addNonFlexHeight * (m.bestHeight - m.minHeight), (bestHeight - minHeight));
            }
            if (h > m.bestHeight) {
                h = m.bestHeight;
            }
        }
printf("GuiLayoutColumn::setPosition for Row %d ---> %d %d %d %d\n", i, p.x, y, p.w, h);
        elements[i]->setPosition(Position(p.x, y, p.w, h));
        y += h;
    }
}
