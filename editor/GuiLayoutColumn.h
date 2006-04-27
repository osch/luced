#ifndef GUILAYOUTCOLUMN_H
#define GUILAYOUTCOLUMN_H

#include "GuiElement.h"
#include "ObjectArray.h"

namespace LucED {

class GuiLayoutColumn : public GuiElement
{
public:
    typedef HeapObjectPtr<GuiLayoutColumn> Ptr;
    
    static Ptr create() {
        return Ptr(new GuiLayoutColumn());
    }
    
    void addElement(GuiElement::Ptr element);
    void addSpacer(int height = -1);
    void addSpacer(int minHeight, int maxHeight);
    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position p);
    
private:
    GuiLayoutColumn() {}
    
    ObjectArray<GuiElement::Ptr> elements;
};

} // namespace LucED

#endif // GUILAYOUTCOLUMN_H
