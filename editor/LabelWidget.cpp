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

#include <stdio.h>
#include "String.hpp"

#include "LabelWidget.hpp"
#include "util.hpp"
#include "GlobalConfig.hpp"

using namespace LucED;

LabelWidget::LabelWidget(const String& labelText)
    : adjustment(VerticalAdjustment::TOP),
      layoutHeight(0),
      hasForcedMeasuresFlag(false)
{
    this->labelText.setToUtf8String(labelText);
}

void LabelWidget::setLayoutHeight(int height, VerticalAdjustment::Type adjust)
{
    layoutHeight = height;
    adjustment = adjust;
}


void LabelWidget::setDesiredMeasures(GuiElement::Measures m)
{
    hasForcedMeasuresFlag = true;
    forcedMeasures = m;
}

GuiElement::Measures LabelWidget::internalGetDesiredMeasures()
{
    if (hasForcedMeasuresFlag) {
        return forcedMeasures;
    } else {
        return getOwnDesiredMeasures();
    }
}

GuiElement::Measures LabelWidget::getOwnDesiredMeasures()
{
    int guiSpacing = GlobalConfig::getConfigData()->getGeneralConfig()->getGuiSpacing();
    int height = util::maximum(GuiWidget::getGuiTextHeight() + guiSpacing, layoutHeight);
    int width  = GuiWidget::getGuiTextStyle()->getTextWidth(labelText) + guiSpacing;
    return GuiElement::Measures(width, height, width, height, width, height);
}

void LabelWidget::processGuiWidgetRedrawEvent(Region redrawRegion)
{
    draw();
}

GuiWidget::ProcessingResult LabelWidget::processGuiWidgetEvent(const XEvent* event)
{
    switch (event->type)
    {
        case ButtonPress: {
            if (event->xbutton.button == Button2) {
                middleMouseButtonCallback->call();
            }
            break;
        }

        case ButtonRelease: {
            break;
        }

        case MotionNotify: {
            break;
        }
    }
    return getGuiWidget()->propagateEventToParentWidget(event);
}

void LabelWidget::processGuiWidgetCreatedEvent()
{
    getGuiWidget()->addToXEventMask(ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);
    getGuiWidget()->setBackgroundColor(getGuiRoot()->getGuiColor03());
}

void LabelWidget::draw()
{
    int guiSpacing = GlobalConfig::getConfigData()->getGeneralConfig()->getGuiSpacing();
    getGuiWidget()->drawRaisedSurface(0, 0, getPosition().w, getPosition().h);
    if (adjustment == VerticalAdjustment::TOP) {
        getGuiWidget()->drawGuiTextWChars(guiSpacing, guiSpacing, labelText);
    } else if (adjustment == VerticalAdjustment::BOTTOM) {
        getGuiWidget()->drawGuiTextWChars(guiSpacing, getPosition().h - GuiWidget::getGuiTextHeight(), labelText);
    } else {
        getGuiWidget()->drawGuiTextWChars(guiSpacing, (getPosition().h - GuiWidget::getGuiTextHeight()) / 2, labelText);
    }
}

