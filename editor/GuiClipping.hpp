/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2011 Oliver Schmidt, oliver at luced dot de
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

#ifndef GUI_CLIPPING_HPP
#define GUI_CLIPPING_HPP

#include "headers.hpp"

#include "NonCopyable.hpp"
#include "RawPointable.hpp"
#include "MemArray.hpp"

namespace LucED
{

class GuiClipping : public RawPointable,
                    public NonCopyable
{
public:
    explicit GuiClipping(Display* display,
                         GC       gcid)
        : display(display),
          gcid(gcid)
    {}
    
    void clear() {
        if (regionStack.getLength() > 0) {
            for (int i = 0; i < regionStack.getLength(); ++i) {
                XDestroyRegion(regionStack[i]);
            }
            regionStack.clear();
            XSetClipMask(display, gcid, None);
        }
    }
    
    ~GuiClipping() {
        clear();
    }
    
    class Holder : public NonCopyable
    {
    public:
        Holder(RawPtr<GuiClipping> clipping, Region additionalRegion)
            : clipping(clipping)
        {
            clipping->push(additionalRegion);
        }
        
        Holder(RawPtr<GuiClipping> clipping, int x, int y, int w, int h)
            : clipping(clipping)
        {
            XRectangle r;
                       r.x = x;
                       r.y = y;
                       r.width  = w;
                       r.height = h;
            Region additionalRegion = XCreateRegion();
            {
                XUnionRectWithRegion(&r, additionalRegion, additionalRegion);
                clipping->push(additionalRegion);
            }
            XDestroyRegion(additionalRegion);
        }

        ~Holder() {
            clipping->pop();
        }
    private:
        friend class GuiClipping;
        
        RawPtr<GuiClipping> clipping;
    };
    

private:    
    void push(Region additionalRegion) 
    {
        Region newRegion = XCreateRegion();
        
        if (regionStack.getLength() > 0) {
            XUnionRegion(newRegion, regionStack.getLast(), newRegion);
            XIntersectRegion(newRegion, additionalRegion, newRegion);
        } else {
            XUnionRegion(newRegion, additionalRegion, newRegion);
        }
        regionStack.append(newRegion);
        XSetRegion(display, gcid, newRegion);
    }
    
    void pop() {
        if (regionStack.getLength() > 0) {
            regionStack.removeLast();
        }
        if (regionStack.getLength() > 0) {
            XSetRegion(display, gcid, regionStack.getLast());
        } else {
            XSetClipMask(display, gcid, None);
        }
    }
    
private:
    Display* display;
    GC gcid;
    MemArray<Region> regionStack;
};

} // namespace LucED

#endif // GUI_CLIPPING_HPP
