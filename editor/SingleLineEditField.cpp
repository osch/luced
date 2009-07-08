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

#include "SingleLineEditField.hpp"
#include "GlobalConfig.hpp"
#include "util.hpp"

using namespace LucED;

SingleLineEditField::SingleLineEditField(LanguageMode::Ptr languageMode, TextData::Ptr textData)
    : adjustment(VerticalAdjustment::TOP),
      layoutHeight(0),
      heightOffset(0),
      cursorStaysHidden(false)
{
    if (textData.isInvalid()) {
        textData = TextData::create();
    }
    editorWidget = SingleLineEditorWidget::create(HilitedText::create(textData, languageMode));
    
    editorWidget->setDesiredMeasuresInChars( 5, 1, 
                                            20, 1, 
                                            40, 1);
    addChildElement(editorWidget);
    
    setKeyActionHandler(editorWidget->getKeyActionHandler());
}

void SingleLineEditField::processGuiWidgetCreatedEvent()
{
    getGuiWidget()->addToXEventMask(ExposureMask);
    getGuiWidget()->setBackgroundColor(getGuiRoot()->getGuiColor03());

    BaseClass::processGuiWidgetCreatedEvent();
}


void SingleLineEditField::setLayoutHeight(int height, VerticalAdjustment::Type adjust)
{
    layoutHeight = height;
    adjustment = adjust;
}

GuiWidget::ProcessingResult SingleLineEditField::processGuiWidgetEvent(const XEvent* event)
{
    switch (event->type)
    {
        case GraphicsExpose:
            if (event->xgraphicsexpose.count > 0) {
                break;
            }
        case Expose: {
            if (event->xexpose.count > 0) {
                break;
            }
            draw();
            return GuiWidget::EVENT_PROCESSED;
        }
    }
    return getGuiWidget()->propagateEventToParentWidget(event);
}

void SingleLineEditField::setDesiredWidthInChars(int minWidth, int bestWidth, int maxWidth)
{
    editorWidget->setDesiredMeasuresInChars(minWidth,  1, 
                                            bestWidth, 1, 
                                            maxWidth,  1);
}

static const int BORDER = 1;

void SingleLineEditField::draw()
{
    if (getGuiWidget().isValid())
    {
        int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();
        int ud = heightOffset;
        int ld = (adjustment == VerticalAdjustment::CENTER) ? heightOffset : 0;
        
        int x = guiSpacing;
        int y = ud + guiSpacing;
        int w = getPosition().w - guiSpacing;
        int h = getPosition().h - ud - ld - guiSpacing;
        
        if (hasFocus()) {
            getGuiWidget()->drawActiveSunkenFrame(  x, y, w, h);
        } else {
            getGuiWidget()->drawInactiveSunkenFrame(x, y, w, h);
        }
    }
}

GuiElement::Measures SingleLineEditField::internalGetDesiredMeasures()
{
    int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();

    Measures rslt = editorWidget->getDesiredMeasures();

    rslt.minWidth += 2 * BORDER + guiSpacing;
    rslt.bestWidth += 2 * BORDER + guiSpacing;
    if (rslt.maxWidth != INT_MAX)
        rslt.maxWidth += 2 * BORDER + guiSpacing;
        
    rslt.minHeight += 2 * BORDER + guiSpacing;
    rslt.bestHeight += 2 * BORDER + guiSpacing;
    if (rslt.maxHeight != INT_MAX)
        rslt.maxHeight += 2 * BORDER + guiSpacing;
    
    rslt.minHeight  = util::maximum(rslt.minHeight,  layoutHeight);
    rslt.bestHeight = util::maximum(rslt.bestHeight, layoutHeight);
    rslt.maxHeight  = util::maximum(rslt.maxHeight,  layoutHeight);

    return rslt;
}

void SingleLineEditField::processGuiWidgetNewPositionEvent(const Position& newPosition) 
{
    Position p = newPosition;
    
    int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();

    Measures m = editorWidget->getDesiredMeasures();

    p.x  = BORDER + guiSpacing;
    p.y  = BORDER + guiSpacing;
    p.h -= 2 * BORDER + guiSpacing;
    p.w -= 2 * BORDER + guiSpacing;
    
    heightOffset = 0;
    if (p.h > m.bestHeight) {
        int d = p.h - m.bestHeight;
        if (adjustment == VerticalAdjustment::CENTER) {
            heightOffset = d/2;
            p.y += d/2;
            p.h -= d - d/2;
        } else if (adjustment == VerticalAdjustment::BOTTOM) {
            heightOffset = d;
            p.y += d;
        }
    }
    editorWidget->setPosition(p);

    BaseClass::processGuiWidgetNewPositionEvent(newPosition);
}


void SingleLineEditField::treatFocusIn()
{
    setFocusFlag(true);
    cursorStaysHidden = false;
    draw();
    editorWidget->treatFocusIn();
    if (editFieldGroup.isValid()) {
        editFieldGroup->invokeAllCursorFocusLostExceptFor(this);
    }
}

void SingleLineEditField::treatFocusOut()
{
    setFocusFlag(false);
    draw();
    editorWidget->treatFocusOut();
}

void SingleLineEditField::treatNotificationOfHotKeyEventForOtherWidget()
{
    if (hasFocus() && editorWidget->isCursorBlinking()) {
        editorWidget->startCursorBlinking(); // redraw Cursor while Hotkey for other widget
    }
}

void SingleLineEditField::setCursorPosition(int position)
{
    editorWidget->moveCursorToTextPositionAndAdjustVisibility(position);
}

