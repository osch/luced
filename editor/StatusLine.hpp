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

#ifndef STATUSLINE_H
#define STATUSLINE_H

#include "String.hpp"

#include "GuiWidget.hpp"
#include "Callback.hpp"
#include "OwningPtr.hpp"
#include "CursorPositionData.hpp"

namespace LucED {



class StatusLine : public GuiWidget
{
public:

    typedef OwningPtr<StatusLine> Ptr;

    static StatusLine::Ptr create(GuiWidget* parent)
    {
        return StatusLine::Ptr(new StatusLine(parent));
    }

    virtual ProcessingResult processEvent(const XEvent *event);
    virtual void setPosition(Position newPosition);
    virtual Measures getDesiredMeasures();

    void setCursorPositionData(CursorPositionData data);

    void setFileName(const String& fileName);
    void setFileLength(long length);
    
private:

    StatusLine(GuiWidget* parent);

    int calcWidth(long value);
    
    void drawArea();
    void drawFileName();
    void drawFileLength();
    void drawLineAndColumn();
    
    Position position;
    String fileName;
    long fileLength;
    long selectionLength;
    long lengthPos;
    
    long line;
    long column;
    long pos;
    int lineAndColumnWidth;
    
    int smallWidth;
    int middleWidth;
    int bigWidth;

    int labelSWidth;    
    int labelPWidth;
    int labelLWidth;
    int labelCWidth;
    int spaceWidth;
};


} // namespace LucED


#endif // STATUSLINE_H
