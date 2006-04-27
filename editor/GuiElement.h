#ifndef GUIELEMENT_H
#define GUIELEMENT_H

#include "HeapObject.h"
#include "Position.h"

namespace LucED {

class GuiElement : public HeapObject
{
public:
    typedef HeapObjectPtr<GuiElement> Ptr;

    class Measures
    {
    public:
        Measures() 
                : minWidth(0),  minHeight(0),
                  bestWidth(0), bestHeight(0),
                  maxWidth(0),  maxHeight(0)
        {}
        Measures(int minWidth, int minHeight, int bestWidth, int bestHeight, int maxWidth, int maxHeight) 
                : minWidth(minWidth),   minHeight(minHeight),
                  bestWidth(bestWidth), bestHeight(bestHeight),
                  maxWidth(maxWidth),   maxHeight(maxHeight)
        {}
        int bestWidth;
        int bestHeight;
        int minWidth;
        int minHeight;
        int maxWidth;
        int maxHeight;
    };
    
    virtual Measures getDesiredMeasures() { return Measures(0, 0, 0, 0, 0, 0); };
    virtual void setPosition(Position p) = 0;

protected:
    GuiElement() {}
};


} // namespace LucED

#endif // GUIELEMENT_H
