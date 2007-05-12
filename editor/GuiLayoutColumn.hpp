/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#ifndef GUILAYOUTCOLUMN_H
#define GUILAYOUTCOLUMN_H

#include <limits.h>

#include "util.hpp"
#include "GuiElement.hpp"
#include "ObjectArray.hpp"
#include "OwningPtr.hpp"

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
