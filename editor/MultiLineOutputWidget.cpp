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

#include "GlobalConfig.hpp"
#include "MultiLineOutputWidget.hpp"

using namespace LucED;


MultiLineOutputWidget::MultiLineOutputWidget(GuiWidget* parent, Style style, TextData::Ptr textData)
    : GuiWidget(parent, 0, 0, GlobalConfig::getInstance()->getTextStyles()->get(0)->getSpaceWidth()*40, 
                              GlobalConfig::getInstance()->getTextStyles()->get(0)->getLineHeight()*2,
                              0),
      style(style),
      textData(textData)
{
    textStyles = TextStyles::create();

    if (style == STYLE_OUTPUT) {
        textStyles->appendNewStyle(GlobalConfig::getInstance()->getTextStyles()->get(0)->getFontName(),
                                   GlobalConfig::getInstance()->getTextStyles()->get(0)->getColorName());
    }
    else if (style == STYLE_GUI) {
        textStyles->appendNewStyle(GlobalConfig::getInstance()->getGuiFont(), 
                                   GlobalConfig::getInstance()->getGuiFontColor());
    }
    else {
        ASSERT(false);
    }
    
    
    textWidget = TextEditorWidget::create(this, 
                                          textStyles, 
                                          HilitedText::create(textData, GlobalConfig::getInstance()->getDefaultLanguageMode()),
                                          TextEditorWidget::READ_ONLY);
//    textWidget->disableCursorChanges();

//    textWidget->setBackgroundColor(getGuiRoot()->getGuiColor03());
    textWidget->setBackgroundColor(getGuiRoot()->getGuiColor05());
    textWidget->hideCursor();
    textWidget->show();
}

GuiElement::Measures MultiLineOutputWidget::getDesiredMeasures()
{
    Measures rslt = textWidget->getDesiredMeasures();

    return rslt;
}

void MultiLineOutputWidget::setPosition(Position p) 
{
    GuiWidget::setPosition(p);

    p.x = 0;
    p.y = 0;
    textWidget->setPosition(p);
}

