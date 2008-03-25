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

#include "SingleLineEditField.hpp"
#include "GlobalConfig.hpp"
#include "util.hpp"

using namespace LucED;

SingleLineEditField::SingleLineEditField(GuiWidget *parent, LanguageMode::Ptr languageMode, TextData::Ptr textData)
    : GuiWidget(parent, 0, 0, 1, 1, 0),
      hasFocusFlag(false),
      adjustment(VerticalAdjustment::TOP),
      layoutHeight(0),
      heightOffset(0),
      cursorStaysHidden(false)
{
    addToXEventMask(ExposureMask);
    setBackgroundColor(getGuiRoot()->getGuiColor03());
    if (textData.isInvalid()) {
        textData = TextData::create();
    }
    editorWidget = SingleLineEditorWidget::create(this, 
                                                  GlobalConfig::getInstance()->getTextStyles(), 
                                                  HilitedText::create(textData, languageMode));
    editorWidget->setDesiredMeasuresInChars( 5, 1, 
                                            20, 1, 
                                            40, 1);
    editorWidget->show();
}

void SingleLineEditField::setLayoutHeight(int height, VerticalAdjustment::Type adjust)
{
    layoutHeight = height;
    adjustment = adjust;
}

GuiElement::ProcessingResult SingleLineEditField::processEvent(const XEvent *event)
{
    if (GuiWidget::processEvent(event) == EVENT_PROCESSED) {
        return EVENT_PROCESSED;
    } else {
        
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
                return EVENT_PROCESSED;
            }
        }
        return propagateEventToParentWidget(event);
    }
}

GuiElement::ProcessingResult SingleLineEditField::processKeyboardEvent(const XEvent *event)
{
    return editorWidget->processKeyboardEvent(event);
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
    int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();
    int ud = heightOffset;
    int ld = (adjustment == VerticalAdjustment::CENTER) ? heightOffset : 0;
    
    int x = guiSpacing;
    int y = ud + guiSpacing;
    int w = getPosition().w - guiSpacing;
    int h = getPosition().h - ud - ld - guiSpacing;
    
    if (hasFocusFlag) {
        drawActiveSunkenFrame(  x, y, w, h);
    } else {
        drawInactiveSunkenFrame(x, y, w, h);
    }
}

GuiElement::Measures SingleLineEditField::getDesiredMeasures()
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

void SingleLineEditField::setPosition(Position p) 
{
    int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();

    GuiWidget::setPosition(p);

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
}

void SingleLineEditField::requestFocusFor(GuiWidget* w)
{
    if (editorWidget.getRawPtr() == w) {
        GuiWidget::requestFocusFor(this);
    }
}

void SingleLineEditField::treatFocusIn()
{
    hasFocusFlag = true;
    cursorStaysHidden = false;
    draw();
    editorWidget->treatFocusIn();
    if (editFieldGroup.isValid()) {
        editFieldGroup->invokeAllCursorFocusLostExceptFor(this);
    }
}

void SingleLineEditField::treatFocusOut()
{
    hasFocusFlag = false;
    draw();
    editorWidget->treatFocusOut();
}

void SingleLineEditField::notifyAboutHotKeyEventForOtherWidget()
{
    if (hasFocusFlag && editorWidget->isCursorBlinking()) {
        editorWidget->startCursorBlinking(); // redraw Cursor while Hotkey for other widget
    }
}

void SingleLineEditField::setCursorPosition(int position)
{
    editorWidget->moveCursorToTextPositionAndAdjustVisibility(position);
}

