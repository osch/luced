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

#include "FramedGuiCompound.hpp"

using namespace LucED;

FramedGuiCompound::FramedGuiCompound(GuiWidget*      parent, 
                                     GuiElement::Ptr rootElement,
                                     Borders         borders,
                                     GuiColor        color)

    : root(rootElement),
      ti(0),
      ri(0),
      bi(0),
      li(0)
{
    if (borders.isSet(RIGHT)) {
        right  = GuiLayoutWidget::create(parent, 1, 1, 1, 1, 1, INT_MAX);
        right ->setBackgroundColor(color);
        right->show();
        ri = 1;
    }
    if (borders.isSet(BOTTOM)) {
        bottom = GuiLayoutWidget::create(parent, 1, 1, 1, 1, INT_MAX, 1);
        bottom->setBackgroundColor(color);
        bottom->show();
        bi = 1;
    }

    if (borders.isSet(TOP)) {
        top  = GuiLayoutWidget::create(parent, 1, 1, 1, 1, INT_MAX, 1);
        top ->setBackgroundColor(color);
        top->show();
        ti = 1;
    }
    
    if (borders.isSet(LEFT)) {
        left = GuiLayoutWidget::create(parent, 1, 1, 1, 1, 1, INT_MAX);
        left->setBackgroundColor(color);
        left->show();
        li = 1;
    }
}


GuiElement::Measures FramedGuiCompound::getDesiredMeasures()
{
    Measures rslt = root->getDesiredMeasures();
    
    rslt.minWidth  += ri + li;
    rslt.bestWidth += ri + li;

    if (rslt.maxWidth < INT_MAX) {
        rslt.maxWidth += ri + li;
    }
    
    rslt.minHeight  += ti + bi;
    rslt.bestHeight += ti + bi;

    if (rslt.maxHeight < INT_MAX) {
        rslt.maxHeight += ti + bi;
    }

    return rslt;
}

void FramedGuiCompound::setPosition(Position p)
{
    Position p1;
    p1.x = p.x + li;
    p1.y = p.y + ti;
    p1.w = p.w - (li + ri);
    p1.h = p.h - (ti + bi);

    root->setPosition(p1);
    
    Position p2;
    
    if (right.isValid())
    {
        p2.x = p.x + p.w - 1;
        p2.y = p.y;
        p2.w = 1;
        p2.h = p.h;
        right->setPosition(p2);
    }
    
    if (bottom.isValid())
    {
        p2.x = p.x;
        p2.y = p.y + p.h - 1;
        p2.w = p.w;
        p2.h = 1;
        bottom->setPosition(p2);
    }

    if (top.isValid())
    {
        p2.x = p.x;
        p2.y = p.y;
        p2.w = p.w;
        p2.h = 1;
        top->setPosition(p2);
    }

    if (left.isValid())
    {
        p2.x = p.x;
        p2.y = p.y;
        p2.w = 1;
        p2.h = p.h;
        left->setPosition(p2);
    }
}


void FramedGuiCompound::show()
{
    GuiElement::show();

    root->show();
    
    if (right.isValid())
    {
        right->show();
    }
    
    if (bottom.isValid())
    {
        bottom->show();
    }

    if (top.isValid())
    {
        top->show();
    }

    if (left.isValid())
    {
        left->show();
    }
}


void FramedGuiCompound::hide()
{
    GuiElement::hide();

    root->hide();

    if (right.isValid())
    {
        right->hide();
    }
    
    if (bottom.isValid())
    {
        bottom->hide();
    }

    if (top.isValid())
    {
        top->hide();
    }

    if (left.isValid())
    {
        left->hide();
    }
}


