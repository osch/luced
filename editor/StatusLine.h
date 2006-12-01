/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch at luced dot de
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

#include <string>

#include "GuiWidget.h"
#include "Callback.h"
#include "Slot.h"
#include "OwningPtr.h"

namespace LucED {

using std::string;

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

    void setLineAndColumn(long line, long column);
    void setFileName(const string& fileName);
    void setFileLength(long length);
    
    Slot2<long,long>     slotForSetLineAndColumn;
    Slot1<const string&> slotForSetFileName;
    Slot1<long>          slotForSetFileLength;

    
private:

    StatusLine(GuiWidget* parent);

    void drawArea();
    void drawFileName();
    void drawFileLength();
    void drawLineAndColumn();
    
    Position position;
    string fileName;
    long fileLength;
    long lengthPos;
    
    long line;
    long column;
    int lineAndColumnWidth;
};


} // namespace LucED


#endif // STATUSLINE_H
