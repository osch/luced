#ifndef GUILAYOUTROW_H
#define GUILAYOUTROW_H

#include "util.h"
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
    void addSpacer(int width);
    void addSpacer();
    void addSpacer(int minWidth, int maxWidth);
    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position p);
    
private:
    GuiLayoutRow() {}
    
    static void maximize(int *a, int b) {
        if (*a != INT_MAX) {
            if (b == INT_MAX) {
                *a = INT_MAX;
            } else {
                util::maximize(a, b);
            }
        }
    }

    static void addimize(int *a, int b) {
        if (*a != INT_MAX) {
            if (b == INT_MAX) {
                *a = INT_MAX;
            } else {
                *a += b;
            }
        }
    }

    ObjectArray<GuiElement::Ptr> elements;
    ObjectArray<Measures> columnMeasures;
};

} // namespace LucED

#endif // GUILAYOUTROW_H
