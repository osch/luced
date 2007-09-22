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

#include "SingleLineEditorWidget.hpp"

using namespace LucED;

static const int BORDER_WIDTH = 2;

SingleLineEditorWidget::SingleLineEditorWidget(GuiWidget *parent, 
            TextStyles::Ptr textStyles, HilitedText::Ptr hilitedText)
    : TextEditorWidget(parent, textStyles, hilitedText, BORDER_WIDTH),
      standardActions(StandardEditActions::createSingleLineActions(this))
{
    getTextData()->setInsertFilterCallback(newCallback(this, &SingleLineEditorWidget::filterInsert));
}


void SingleLineEditorWidget::filterInsert(const byte** buffer, long* length)
{
    filterBuffer.clear();
    const long len = *length;
    const byte* buf = *buffer;
    for (int i = 0; i < len; ++i) {
        if (buf[i] == '\t') {
            filterBuffer.appendCStr("\\t");
        } else if (buf[i] == '\n') {
            filterBuffer.appendCStr("\\n");
        } else if (buf[i] == '\r') {
            filterBuffer.appendCStr("\\r");
        } else {
            filterBuffer.append(buf[i]);
        }
    }
    *buffer = filterBuffer.getPtr(0);
    *length = filterBuffer.getLength();
}


