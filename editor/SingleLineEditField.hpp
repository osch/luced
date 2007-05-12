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

#ifndef SINGLELINEEDITFIELD_H
#define SINGLELINEEDITFIELD_H

#include "SingleLineEditorWidget.hpp"
#include "TextData.hpp"
#include "EditFieldGroup.hpp"

namespace LucED
{

class SingleLineEditField : public GuiWidget
{
public:
    typedef OwningPtr<SingleLineEditField> Ptr;

    static SingleLineEditField::Ptr create(GuiWidget *parent, 
            LanguageMode::Ptr languageMode, TextData::Ptr textData = TextData::Ptr())
    {
        return SingleLineEditField::Ptr(new SingleLineEditField(parent, 
                languageMode, textData));
    }

    void setLayoutHeight(int height, VerticalAdjustment::Type adjust);
    void setDesiredWidthInChars(int minWidth, int bestWidth, int maxWidth);

    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position p);

    virtual bool isFocusable() { return true; }
    virtual FocusType getFocusType() { return NORMAL_FOCUS; }
    virtual void treatFocusIn();
    virtual void treatFocusOut();

    TextData* getTextData() {
        return editorWidget->getTextData();
    }
    
    bool hasFocus() const {
        return hasFocusFlag;
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
        editFieldGroup->registerCursorFocusLostHandler(Callback0(this, &SingleLineEditField::hideCursor));
    }

    virtual ProcessingResult processEvent(const XEvent *event);
    virtual ProcessingResult processKeyboardEvent(const XEvent *event);
    virtual void notifyAboutHotKeyEventForOtherWidget();
    
protected:    
    virtual void requestFocusFor(GuiWidget* w);

private:
    SingleLineEditField(GuiWidget *parent, LanguageMode::Ptr languageMode, TextData::Ptr textData);
    SingleLineEditorWidget::Ptr editorWidget;
    void draw();
    bool hasFocusFlag;
    VerticalAdjustment::Type adjustment;
    int layoutHeight;
    int heightOffset;

    EditFieldGroup::Ptr editFieldGroup;
    bool cursorStaysHidden;
};


} // namespace LucED


#endif // SINGLELINEEDITFIELD_H
