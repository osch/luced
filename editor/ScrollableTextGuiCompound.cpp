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

#include "ScrollableTextGuiCompound.hpp"
#include "GlobalConfig.hpp"

using namespace LucED;

ScrollableTextGuiCompound::ScrollableTextGuiCompound(TextEditorWidget::Ptr textWidget, 
                                                     Options               options)
    : textWidget(textWidget),
      dynamicScrollBarDisplayFlag(options.isSet(DYNAMIC_SCROLL_BAR_DISPLAY))
{
    GuiLayoutRow::Ptr rootRow = GuiLayoutRow::create();

    if (options.isSet(WITHOUT_OUTER_FRAME))
    {
        rootElement = rootRow;
    }
    else
    {
        FramedGuiCompound::Ptr rootFrame2 = FramedGuiCompound::create(rootRow, 
                                                                        FramedGuiCompound::Borders() 
                                                                      | FramedGuiCompound::TOP 
                                                                      | FramedGuiCompound::LEFT);
    
        FramedGuiCompound::Ptr rootFrame1 = FramedGuiCompound::create(rootFrame2,
                                                                        FramedGuiCompound::Borders() 
                                                                      | FramedGuiCompound::BOTTOM 
                                                                      | FramedGuiCompound::RIGHT, 
                                                                      GuiRoot::getInstance()->getGuiColor05());
        rootElement = rootFrame1;
    }
    addChildElement(rootElement);

    layoutColumn1 = GuiLayoutColumn::create();

    rootRow->addElement(layoutColumn1);

    verticalScrollBarLeftBorder = GuiLayoutWidget::create(1, 1, 1, 1, 1, INT_MAX);
    verticalScrollBarLeftBorder->hide();

    GuiLayoutRow::Ptr r1 = GuiLayoutRow::create();
    r1->addElement(textWidget);
    r1->addElement(verticalScrollBarLeftBorder);
    
    layoutColumn1->addElement(r1);

    textWidget->show();

    scrollBarV = ScrollBar::create(Orientation::VERTICAL);
    scrollBarH = ScrollBar::create(Orientation::HORIZONTAL);
    
    horizontalScrollBarFrame = FramedGuiCompound::create(scrollBarH, 
                                                           FramedGuiCompound::Borders() 
                                                         | FramedGuiCompound::TOP);
    horizontalScrollBarFrame->hide();
    layoutColumn1->addElement(horizontalScrollBarFrame);

    layoutColumn2 = GuiLayoutColumn::create();
    rootRow->addElement(layoutColumn2);

    int w = GlobalConfig::getInstance()->getScrollBarWidth();

    scrollBarSpacer = GuiLayoutWidget::create(w, w, w, w, w, w,
                                              GuiRoot::getInstance()->getGuiColor02());

    layoutColumn2->addElement(scrollBarV);
    layoutColumn2->addElement(scrollBarSpacer);
    layoutColumn2->hide();

    textWidget->connectToVerticalScrollBar  (scrollBarV);
    textWidget->connectToHorizontalScrollBar(scrollBarH);


    if (!dynamicScrollBarDisplayFlag)
    {
        layoutColumn2->show();
        horizontalScrollBarFrame->show();
        verticalScrollBarLeftBorder->show();
        scrollBarSpacer->show();
    }
}


GuiElement::Measures ScrollableTextGuiCompound::internalGetDesiredMeasures()
{
    Measures rslt = rootElement->getDesiredMeasures();

    return rslt;
}


void ScrollableTextGuiCompound::setPosition(const Position& p) 
{
    long numberOfLines = textWidget->getNumberOfLines();
    
    rootElement->setPosition(p);

    if (dynamicScrollBarDisplayFlag)
    {
        bool desiredMesasuresChanged = false;

        if (textWidget->getNumberOfVisibleLines() < numberOfLines) {
            if (!scrollBarV->isVisible()) {
                layoutColumn2->show();
                verticalScrollBarLeftBorder->show();
                scrollBarSpacer->hide();
                desiredMesasuresChanged = true;
            }
        } else {
            if (scrollBarV->isVisible()) {
                layoutColumn2->hide();
                verticalScrollBarLeftBorder->hide();
                desiredMesasuresChanged = true;
            }
        }
    
        if (   scrollBarV->isVisible()
            || textWidget->getPixWidth() < textWidget->getMaximalVisiblePixWidth())
        {
            if (!horizontalScrollBarFrame->isVisible()) {
                horizontalScrollBarFrame->show();
                desiredMesasuresChanged = true;
            }
        } else {
            if (horizontalScrollBarFrame->isVisible()) {
                horizontalScrollBarFrame->hide();
                desiredMesasuresChanged = true;
            }
        }
        if (horizontalScrollBarFrame->isVisible() && layoutColumn2->isVisible() && !scrollBarSpacer->isVisible()) {
            scrollBarSpacer->show();
            desiredMesasuresChanged = true;
        }
        else if ((!horizontalScrollBarFrame->isVisible() || !layoutColumn2->isVisible()) && scrollBarSpacer->isVisible()) {
            scrollBarSpacer->hide();
            desiredMesasuresChanged = true;
        }
        if (desiredMesasuresChanged) {
            throw GuiElement::DesiredMeasuresChangedException();
        }
    }
}

