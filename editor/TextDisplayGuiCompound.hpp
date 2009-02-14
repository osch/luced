/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
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

#ifndef TEXT_DISPLAY_GUI_COMPOUND_HPP
#define TEXT_DISPLAY_GUI_COMPOUND_HPP

#include "GuiWidget.hpp"
#include "OwningPtr.hpp"
#include "TextEditorWidget.hpp"
#include "GuiLayoutColumn.hpp"
#include "GuiLayoutRow.hpp"
#include "ScrollBar.hpp"
#include "GuiLayoutSpacer.hpp"
#include "GuiLayoutWidget.hpp"
#include "FramedGuiCompound.hpp"
#include "ScrollableTextGuiCompound.hpp"

namespace LucED
{

class TextDisplayGuiCompound : public ScrollableTextGuiCompound
{
public:
    typedef OwningPtr<TextDisplayGuiCompound> Ptr;

    static Ptr create(GuiWidget* parent, TextData::Ptr textData);

    virtual bool invokeActionMethod(ActionId actionId);
    virtual bool hasActionMethod(ActionId actionId);
    
private:
    TextDisplayGuiCompound(GuiWidget*                         parent, 
                           TextEditorWidget::Ptr              textWidget, 
                           ScrollableTextGuiCompound::Options options);

    virtual void addActionMethods(ActionMethods::Ptr methods);

    ObjectArray<ActionMethods::Ptr> actionMethods;
};

} // namespace LucED

#endif // TEXT_DISPLAY_GUI_COMPOUND_HPP
