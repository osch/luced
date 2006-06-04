#ifndef GUILAYOUTROW_H
#define GUILAYOUTROW_H

#include "GuiElement.h"
#include "ObjectArray.h"
#include "OwningPtr.h"

namespace LucED {

class GuiLayoutRow : public GuiElement
{
public:
    typedef OwningPtr<GuiLayoutRow> Ptr;
    
    static Ptr create() {
        return Ptr(new GuiLayoutRow());
    }
    
    void addElement(GuiElement::Ptr element);
    void addSpacer(int width = -1);
    void addSpacer(int minWidth, int maxWidth);
    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position p);
    
private:
    GuiLayoutRow() {}
    
    ObjectArray<GuiElement::Ptr> elements;
};

} // namespace LucED

#endif // GUILAYOUTROW_H
