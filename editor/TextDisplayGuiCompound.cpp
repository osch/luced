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

#include "util.hpp"
#include "GlobalConfig.hpp"
#include "TextDisplayGuiCompound.hpp"
#include "MultiLineDisplayActions.hpp"
#include "SingleLineDisplayActions.hpp"

using namespace LucED;


TextDisplayGuiCompound::Ptr TextDisplayGuiCompound::create(GuiWidget* parent, Style style, TextData::Ptr textData)
{
    TextStyles::Ptr textStyles = TextStyles::create();

    switch (style)
    {
        case STYLE_GUI: {
            textStyles->appendNewStyle(GlobalConfig::getInstance()->getGuiFont(), 
                                       GlobalConfig::getInstance()->getGuiFontColor());
            break;
        }
        default: ASSERT(false);
        case STYLE_OUTPUT: {
            textStyles->appendNewStyle(GlobalConfig::getInstance()->getTextStyles()->get(0)->getFontName(),
                                       GlobalConfig::getInstance()->getTextStyles()->get(0)->getColorName());
            break;
        }
    }
    
    
    TextEditorWidget::Ptr textWidget = TextEditorWidget::create(parent, 
                                                                textStyles, 
                                                                HilitedText::create(textData, GlobalConfig::getInstance()->getDefaultLanguageMode()),
                                                                TextWidget::CreateOptions() | TextWidget::READ_ONLY
                                                                                            | TextWidget::NEVER_SHOW_CURSOR);
    textWidget->setBackgroundColor(GuiRoot::getInstance()->getGuiColor04());
    
    textWidget->setLastEmptyLineStrategy(TextWidget::IGNORE_EMPTY_LAST_LINE);
    
    long numberOfLines = textWidget->getNumberOfLines();
    
    textWidget->setDesiredMeasuresInChars(10, util::minimum(numberOfLines,  4L),
                                          80, util::minimum(numberOfLines, 25L),
                                          INT_MAX, /*numberOfLines*/ INT_MAX);

    textWidget->setVerticalAdjustmentStrategy  (TextWidget::NOT_STRICT_TOP_LINE_ANCHOR);
    textWidget->setHorizontalAdjustmentStrategy(TextWidget::NOT_STRICT_LEFT_COLUMN_ANCHOR);


    textWidget->hideCursor();
    textWidget->show();


    return Ptr(new TextDisplayGuiCompound(parent, textWidget,   ScrollableTextGuiCompound::Options() 
                                                              | ScrollableTextGuiCompound::DYNAMIC_SCROLL_BAR_DISPLAY));
}


TextDisplayGuiCompound::TextDisplayGuiCompound(GuiWidget*                         parent, 
                                               TextEditorWidget::Ptr              textWidget, 
                                               ScrollableTextGuiCompound::Options options)

    : ScrollableTextGuiCompound(parent, textWidget, options)
{
    addActionMethods(SingleLineDisplayActions::create(textWidget));
    addActionMethods(MultiLineDisplayActions::create(textWidget));
}


void TextDisplayGuiCompound::addActionMethods(ActionMethods::Ptr methods)
{
    actionMethods.append(methods);
}

bool TextDisplayGuiCompound::invokeActionMethod(ActionId actionId)
{
    for (int i = actionMethods.getLength() - 1; i >= 0; --i) {
        if (actionMethods[i]->invokeActionMethod(actionId)) {
            ASSERT(hasActionMethod(actionId));
            return true;
        }
    }
    return false;
}

bool TextDisplayGuiCompound::hasActionMethod(ActionId actionId)
{
    for (int i = actionMethods.getLength() - 1; i >= 0; --i) {
        if (actionMethods[i]->hasActionMethod(actionId)) {
            return true;
        }
    }
    return false;
}

