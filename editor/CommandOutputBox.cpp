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

#include "CommandOutputBox.hpp"
#include "GlobalConfig.hpp"
#include "GuiLayoutRow.hpp"
#include "GuiLayoutSpacer.hpp"
#include "GlobalConfig.hpp"
#include "LabelWidget.hpp"
#include "Callback.hpp"

using namespace LucED;

CommandOutputBox::CommandOutputBox(TopWin* referingWindow, TextData::Ptr textData)
    : PanelDialogWin(referingWindow),
      wasClosed(false)
{
    button1 = Button::create(this, "O]K");
    
    multiLineOut = TextDisplayGuiCompound::create(this, 
                                                  TextDisplayGuiCompound::STYLE_OUTPUT,
                                                  textData);
    

    LabelWidget::Ptr label0 = LabelWidget::create(this, "CommandOutputBox");
    GuiLayoutColumn::Ptr column0 = GuiLayoutColumn::create();
    GuiLayoutRow::Ptr row0 = GuiLayoutRow::create();

    GuiLayoutSpacerFrame::Ptr frame0 = GuiLayoutSpacerFrame::create(column0, 10);
    setRootElement(frame0);

    column0->setReportRasteringOptions(  GuiLayoutColumn::ReportRasteringOptions() 
                                       | GuiLayoutColumn::DO_NOT_REPORT_HORIZONTAL_RASTERING
                                       | GuiLayoutColumn::DO_NOT_REPORT_VERTICAL_RASTERING);
                                                                                 
    column0->addElement(multiLineOut, GuiElement::LayoutOptions() | GuiElement::LAYOUT_VERTICAL_RASTERING);
    column0->addElement(GuiLayoutSpacer::create(0, 0, 0, 10, INT_MAX, INT_MAX));
    column0->addElement(row0);
    column0->addSpacer();

    row0->addElement(GuiLayoutSpacer::create(0, 0, 0, 0, INT_MAX, 0));
    if (button1.isValid()) {
        row0->addElement(button1);
    }
    row0->addElement(GuiLayoutSpacer::create(0, 0, 0, 0, INT_MAX, 0));
    
    Callback<Button*>::Ptr buttonCallback = newCallback(this, &CommandOutputBox::handleButtonPressed);
    
    if (button1.isValid()) {
        button1->setButtonPressedCallback(buttonCallback);
    }

    label0->show();
    if (button1.isValid()) {
        button1->show();
    }
    
    if (button1.isValid()) {
        button1->setAsDefaultButton();
    }
    setTitle("Command Output");

    if (button1.isValid()) {
        setFocus(button1);
    }
    multiLineOut->show();
}


void CommandOutputBox::handleButtonPressed(Button* button)
{
#if 0
    if (!wasClosed)
    {

        if (button == button1 && defaultButtonCallback->isEnabled()) {
                PanelDialogWin::requestCloseWindow();
                wasClosed = true;
                defaultButtonCallback->call();
        } else {
            if (button == button1 && button3.isInvalid()) {
                PanelDialogWin::requestCloseWindow();
                wasClosed = true;
            }
            else if (button == button3 && button3.isValid()) {
                PanelDialogWin::requestCloseWindow();
                wasClosed = true;
                cancelButtonCallback->call();
            }
            else if (button == button2) {
                PanelDialogWin::requestCloseWindow();
                wasClosed = true;
                alternativeButtonCallback->call();
            }
        }
    }
#endif
}

void CommandOutputBox::requestCloseWindow()
{
    PanelDialogWin::requestCloseWindow();
#if 0
    if (!wasClosed) {
        if (closeCallback->isEnabled()) {
            closeCallback->call();
        } else {
            cancelButtonCallback->call();
        }
        wasClosed = true;
    }
#endif
}


