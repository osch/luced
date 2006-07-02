#ifndef GUILAYOUTCOLUMN_H
#define GUILAYOUTCOLUMN_H

#include "GuiElement.h"
#include "ObjectArray.h"
#include "OwningPtr.h"

namespace LucED {

class GuiLayoutColumn : public GuiElement
{
public:
    typedef OwningPtr<GuiLayoutColumn> Ptr;
    
    static Ptr create() {
        return Ptr(new GuiLayoutColumn());
    }
    
    void addElement(GuiElement::Ptr element);
    void addSpacer();
    void addSpacer(int height);
    void addSpacer(int minHeight, int maxHeight);
    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position p);
    
private:
    GuiLayoutColumn() {}
    
    ObjectArray<GuiElement::Ptr> elements;
    ObjectArray<Measures> rowMeasures;
};

} // namespace LucED

#endif // GUILAYOUTCOLUMN_H
