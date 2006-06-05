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

#include "FindDialog.h"
#include "GlobalConfig.h"
#include "GuiLayoutRow.h"
#include "GuiLayoutTable.h"
#include "GuiLayoutSpacer.h"

using namespace LucED;

FindDialog::FindDialog(TopWin* referingWindow, int x, int y, unsigned int width, unsigned int height)
    : DialogWin(referingWindow, x, y, width, height)
{
    button1 = Button::create(this, "Das ist ein Test");
    button2 = Button::create(this, "Abbruch");
    
    layout = GuiLayoutColumn::create();
    layout->addSpacer();
    
//    GuiLayoutRow::Ptr layoutR = GuiLayoutRow::create();
//    layoutR->addSpacer();
//    layoutR->addSpacer();
//    layoutR->addSpacer();
//    layoutR->addElement(button1);
//    layoutR->addSpacer(3, 10);
//    layoutR->addElement(button2);
//    layoutR->addSpacer();

    GuiLayoutTable::Ptr layoutR = GuiLayoutTable::create(1, 7);
    layoutR->setElement(0, 0, GuiLayoutSpacer::create(0, 0, 0, 0, -1, 0));
    layoutR->setElement(0, 1, GuiLayoutSpacer::create(0, 0, 0, 0, -1, 0));
    layoutR->setElement(0, 2, GuiLayoutSpacer::create(0, 0, 0, 0, -1, 0));
    layoutR->setElement(0, 3, button1);
    layoutR->setElement(0, 4, GuiLayoutSpacer::create(3, 0, 10, 0, 10, 0));
    layoutR->setElement(0, 5, button2);
    layoutR->setElement(0, 6, GuiLayoutSpacer::create(0, 0, 0, 0, -1, 0));
    
    layout->addElement(layoutR);
    layout->addSpacer(0, 10);

/*    layout->setPosition(Position(2, 2, width - 2, height - 2));
    Measures m = layout->getDesiredMeasures();
    setPosition(Position(x, y, m.bestWidth + 4, m.bestHeight + 4));
    setSizeHints(x, y, m.minWidth + 4, m.minHeight + 4, 1, 1);
*/
    setRootElement(layout);
        
    button1->show();    
    button2->show();    
}


