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

#include "SingleLineEditField.h"
#include "GlobalConfig.h"

using namespace LucED;

SingleLineEditField::SingleLineEditField(GuiWidget *parent, LanguageMode::Ptr languageMode)
    : GuiWidget(parent, 0, 0, 1, 1, 0),
      hasFocus(false)
{
    addToXEventMask(ExposureMask);
    setBackgroundColor(getGuiRoot()->getGuiColor03());
    editorWidget = SingleLineEditorWidget::create(
                       this, GlobalConfig::getInstance()->getTextStyles(), 
                       HilitedText::create(TextData::create(), languageMode));
    editorWidget->show();
}


GuiElement::ProcessingResult SingleLineEditField::processEvent(const XEvent *event)
{
    if (GuiWidget::processEvent(event) == EVENT_PROCESSED) {
        return EVENT_PROCESSED;
    } else {
        
        switch (event->type) {
            
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
    int d = 0;
    if (hasFocus) {
        drawActiveSunkenFrame(d, d, getPosition().w - 2 * d, getPosition().h - 2 * d);
    } else {
        drawInactiveSunkenFrame(d, d, getPosition().w - 2 * d, getPosition().h - 2 * d);
    }
}

GuiElement::Measures SingleLineEditField::getDesiredMeasures()
{
    Measures rslt = editorWidget->getDesiredMeasures();

    rslt.minWidth += 2 * BORDER;
    rslt.bestWidth += 2 * BORDER;
    if (rslt.maxWidth != -1)
        rslt.maxWidth += 2 * BORDER;
        
    rslt.minHeight += 2 * BORDER;
    rslt.bestHeight += 2 * BORDER;
    if (rslt.maxHeight != -1)
        rslt.maxHeight += 2 * BORDER;
        
    return rslt;
}

void SingleLineEditField::setPosition(Position p) 
{
    GuiWidget::setPosition(p);
    p.x  = BORDER;
    p.y  = BORDER;
    p.h -= 2 * BORDER;
    p.w -= 2 * BORDER;
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
    hasFocus = true;
    draw();
    editorWidget->treatFocusIn();
}

void SingleLineEditField::treatFocusOut()
{
    hasFocus = false;
    draw();
    editorWidget->treatFocusOut();
}

