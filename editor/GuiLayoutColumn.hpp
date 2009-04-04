/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2007 Oliver Schmidt, oliver at luced dot de
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
#include "Flags.hpp"

namespace LucED {

class GuiLayoutColumn : public GuiElement
{
public:
    typedef OwningPtr<GuiLayoutColumn> Ptr;
    
    static Ptr create() {
        return Ptr(new GuiLayoutColumn());
    }
    
    int addElement(GuiElement::Ptr element, LayoutOptions layoutOptions = LayoutOptions());

    void removeElementAtPosition(int i) {
        childElements.remove(i);
    }
    int getElementIndex(GuiElement::Ptr element) {
        for (int i = 0; i < childElements.getLength(); ++i) {
            if (childElements[i] == element) {
                return i;
            }
        }
        return -1;
    }
    bool removeElement(GuiElement::Ptr element) {
        int i = getElementIndex(element);
        if (i >= 0) {
            childElements.remove(i);
            return true;
        } else {
            return false;
        }
    }
    void insertElementAtPosition(GuiElement::Ptr element, int i) {
        childElements.insert(i, element);
    }
    bool insertElementBeforeElement(GuiElement::Ptr e1, GuiElement::Ptr e2) {
        int i = getElementIndex(e2);
        if (i >= 0) {
            childElements.insert(i, e1);
            return true;
        } else {
            return false;
        }
    }
    bool insertElementAfterElement(GuiElement::Ptr e1, GuiElement::Ptr e2) {
        int i = getElementIndex(e2);
        if (i >= 0) {
            childElements.insert(i + 1, e1);
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
    
    enum ReportRasteringOption {
        DO_NOT_REPORT_HORIZONTAL_RASTERING,
        DO_NOT_REPORT_VERTICAL_RASTERING
    };
    
    typedef Flags<ReportRasteringOption> ReportRasteringOptions;
    
    void setReportRasteringOptions(ReportRasteringOptions reportRasteringOptions) {
        this->reportRasteringOptions = reportRasteringOptions;
    }

    virtual void show();
    virtual void hide();

private:
    GuiLayoutColumn() {}
    
    ObjectArray<Measures> rowMeasures;

    ReportRasteringOptions reportRasteringOptions;
};

} // namespace LucED

#endif // GUILAYOUTCOLUMN_H
