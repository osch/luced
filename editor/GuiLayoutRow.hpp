/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#ifndef GUI_LAYOUT_ROW_HPP
#define GUI_LAYOUT_ROW_HPP

#include "util.hpp"
#include "GuiElement.hpp"
#include "ObjectArray.hpp"
#include "OwningPtr.hpp"
#include "Flags.hpp"

namespace LucED
{

class GuiLayoutRow : public GuiElement
{
public:
    typedef OwningPtr<GuiLayoutRow> Ptr;
    
    static Ptr create() {
        return Ptr(new GuiLayoutRow());
    }
    
    void addElement(GuiElement::Ptr element, LayoutOptions layoutOptions = LayoutOptions());
    void addSpacer(int width);
    void addSpacer();
    void addSpacer(int minWidth, int maxWidth);
    virtual void setPosition(const Position& p);
    
    void removeElementAtPosition(int i) {
        GuiElement::removeChildElement(i);
    }
    void insertElementAtPosition(GuiElement::Ptr element, int i) {
        GuiElement::insertChildElement(i, element);
    }
    
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
    GuiLayoutRow()
    {}
    
    virtual Measures internalGetDesiredMeasures();

    ObjectArray<Measures> columnMeasures;

    ReportRasteringOptions reportRasteringOptions;
};

} // namespace LucED

#endif // GUI_LAYOUT_ROW_HPP
