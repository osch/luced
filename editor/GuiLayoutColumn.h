#ifndef GUILAYOUTCOLUMN_H
#define GUILAYOUTCOLUMN_H

#include <limits.h>

#include "util.h"
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
    
    int addElement(GuiElement::Ptr element) {
        elements.append(element);
        return elements.getLength() - 1;
    }
    void removeElementAtPosition(int i) {
        elements.remove(i);
    }
    int getElementIndex(GuiElement::Ptr element) {
        for (int i = 0; i < elements.getLength(); ++i) {
            if (elements[i] == element) {
                return i;
            }
        }
        return -1;
    }
    bool removeElement(GuiElement::Ptr element) {
        int i = getElementIndex(element);
        if (i >= 0) {
            elements.remove(i);
            return true;
        } else {
            return false;
        }
    }
    void insertElementAtPosition(GuiElement::Ptr element, int i) {
        elements.insert(i, element);
    }
    bool insertElementBeforeElement(GuiElement::Ptr e1, GuiElement::Ptr e2) {
        int i = getElementIndex(e2);
        if (i >= 0) {
            elements.insert(i, e1);
            return true;
        } else {
            return false;
        }
    }
    bool insertElementAfterElement(GuiElement::Ptr e1, GuiElement::Ptr e2) {
        int i = getElementIndex(e2);
        if (i >= 0) {
            elements.insert(i + 1, e1);
            return true;
        } else {
            return false;
        }
    }
    void addSpacer();
    void addSpacer(int height);
    void addSpacer(int minHeight, int maxHeight);
    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position p);
    
private:
    GuiLayoutColumn() {}
    

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
    ObjectArray<Measures> rowMeasures;
};

} // namespace LucED

#endif // GUILAYOUTCOLUMN_H
