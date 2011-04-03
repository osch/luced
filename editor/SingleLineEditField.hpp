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

#ifndef SINGLE_LINE_EDIT_FIELD_HPP
#define SINGLE_LINE_EDIT_FIELD_HPP

#include "SingleLineEditorWidget.hpp"
#include "TextData.hpp"
#include "EditFieldGroup.hpp"
#include "RawPtr.hpp"
#include "FocusableContainerWidget.hpp"

namespace LucED
{

class SingleLineEditField : public FocusableContainerWidget
{
public:
    typedef FocusableContainerWidget       BaseClass;
    typedef OwningPtr<SingleLineEditField> Ptr;

    static Ptr create(LanguageMode::Ptr languageMode, 
                      TextData::Ptr     textData = TextData::Ptr())
    {
        return Ptr(new SingleLineEditField(languageMode, 
                                           textData));
    }

    void setLayoutHeight(int height, VerticalAdjustment::Type adjust);
    void setDesiredWidthInChars(int minWidth, int bestWidth, int maxWidth);

    virtual void treatFocusIn();
    virtual void treatFocusOut();

    TextData::Ptr getTextData() {
        return editorWidget->getTextData();
    }
    
    void showCursor() {
        if (!cursorStaysHidden) {
            editorWidget->showCursor();
        }
    }
    
    void hideCursor() {
        editorWidget->hideCursor();
        cursorStaysHidden = true;
    }
    
    void setCursorPosition(int position);
    
    void setToEditFieldGroup(EditFieldGroup::Ptr editFieldGroup) {
        this->editFieldGroup = editFieldGroup;
        editFieldGroup->registerCursorFocusLostHandler(newCallback(this, &SingleLineEditField::hideCursor));
    }
    
    void replaceTextWithPrimarySelection() {
        editorWidget->replaceTextWithPrimarySelection();
    }

    virtual void treatNotificationOfHotKeyEventForOtherWidget();
    
protected:
    virtual Measures internalGetDesiredMeasures();

    virtual void processGuiWidgetCreatedEvent();

protected: // GuiWidget::EventListener interface implementation
    virtual GuiWidget::ProcessingResult processGuiWidgetEvent(const XEvent* event);
    virtual void                        processGuiWidgetNewPositionEvent(const Position& newPosition);
    virtual void                        processGuiWidgetRedrawEvent(Region redrawRegion);

private:
    SingleLineEditField(LanguageMode::Ptr languageMode, TextData::Ptr textData);
    

    SingleLineEditorWidget::Ptr editorWidget;
    void draw();
    VerticalAdjustment::Type adjustment;
    int layoutHeight;
    int heightOffset;

    EditFieldGroup::Ptr editFieldGroup;
    bool cursorStaysHidden;
};

} // namespace LucED

#endif // SINGLE_LINE_EDIT_FIELD_HPP
