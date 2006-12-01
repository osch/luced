/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch@luced.de
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

#include "SingleLineEditorWidget.h"
#include "TextData.h"

namespace LucED
{

class SingleLineEditField : public GuiWidget
{
public:
    typedef OwningPtr<SingleLineEditField> Ptr;

    static SingleLineEditField::Ptr create(GuiWidget *parent, 
            LanguageMode::Ptr languageMode)
    {
        return SingleLineEditField::Ptr(new SingleLineEditField(parent, 
                languageMode));
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

    virtual ProcessingResult processEvent(const XEvent *event);
    virtual ProcessingResult processKeyboardEvent(const XEvent *event);
    virtual void notifyAboutHotKeyEventForOtherWidget();
    
protected:    
    virtual void requestFocusFor(GuiWidget* w);

private:
    SingleLineEditField(GuiWidget *parent, LanguageMode::Ptr languageMode);
    SingleLineEditorWidget::Ptr editorWidget;
    void draw();
    bool hasFocus;
    VerticalAdjustment::Type adjustment;
    int layoutHeight;
    int heightOffset;
};


} // namespace LucED


#endif // SINGLELINEEDITFIELD_H
