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

#include "GuiElement.hpp"
#include "GuiWidget.hpp"
#include "FocusManager.hpp"

using namespace LucED;


const char* GuiElement::DesiredMeasuresChangedException::what() const throw()
{
    static const char* whatString = "DesiredMeasuresChangedException";
    return whatString;
}



void GuiElement::Measures::maximize(const GuiElement::Measures& rhs)
{
    if (bestWidth < rhs.bestWidth) {
        bestWidth = rhs.bestWidth;
    }
    if (bestHeight < rhs.bestHeight) {
        bestHeight = rhs.bestHeight;
    }
    if (minWidth < rhs.minWidth) {
        minWidth = rhs.minWidth;
    }
    if (minHeight < rhs.minHeight) {
        minHeight = rhs.minHeight;
    }
    if (maxWidth < rhs.maxWidth) {
        maxWidth = rhs.maxWidth;
    }
    if (maxHeight < rhs.maxHeight) {
        maxHeight = rhs.maxHeight;
    }
}

void GuiElement::internalAdoptChild(RawPtr<GuiElement> child)
{
    ASSERT(wasAdoptedFlag);
    
    child->adopt(parentElement, parentWidget, focusManagerForChilds,
                                              focusManagerForChilds);
}


void GuiElement::adopt(RawPtr<GuiElement>   parentElement,
                       RawPtr<GuiWidget>    parentWidget,
                       RawPtr<FocusManager> focusManagerForThis,
                       RawPtr<FocusManager> focusManagerForChilds)
{
    if (wasAdoptedFlag && this->parentElement == parentElement) {
        return;
    }
    ASSERT(!wasAdoptedFlag);
    wasAdoptedFlag = true;
    this->parentElement = parentElement;
    this->parentWidget  = parentWidget;
    this->focusManagerForChilds  = focusManagerForChilds;
    
    if (rootElement.isValid()) {
        rootElement->adopt(parentElement, parentWidget, focusManagerForChilds,
                                                        focusManagerForChilds);
    }
    for (int i = 0; i < childElements.getLength(); ++i)
    {
        if (childElements[i].isValid())
        {
            childElements[i]->adopt(parentElement, parentWidget, focusManagerForChilds,
                                                                 focusManagerForChilds);
        }
    }
}

void GuiElement::show()
{
    visibility = VISIBLE;
}

void GuiElement::hide()
{
    visibility = HIDDEN;
}


void GuiElement::setPosition(const Position& p)
{
    this->position = p;
}

