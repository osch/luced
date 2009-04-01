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

#ifndef GUI_ELEMENT_HPP
#define GUI_ELEMENT_HPP

#include <X11/Xlib.h>

#include "HeapObject.hpp"
#include "Position.hpp"
#include "OwningPtr.hpp"
#include "WeakPtr.hpp"
#include "Flags.hpp"
#include "BaseException.hpp"

namespace LucED
{

class GuiElement : public HeapObject
{
public:
    typedef OwningPtr<GuiElement> Ptr;

    enum LayoutOption
    {
        LAYOUT_HORIZONTAL_RASTERING,
        LAYOUT_VERTICAL_RASTERING,

        LAYOUT_VERTICAL_CENTER,
        LAYOUT_BOTTOM,

        LAYOUT_HORIZONTAL_CENTER,
        LAYOUT_RIGHT
    };
    
    enum Visibility
    {
        UNDEFINED_VISIBILITY,
        VISIBLE,
        HIDDEN
    };
    
    typedef Flags<LayoutOption> LayoutOptions;

    class Measures
    {
    public:
        Measures() 
                : minWidth(0),  minHeight(0),
                  bestWidth(0), bestHeight(0),
                  maxWidth(0),  maxHeight(0),
                  incrWidth(0), incrHeight(0)
        {}
        Measures(int minWidth, int minHeight, int bestWidth, int bestHeight, int maxWidth, int maxHeight) 
                : minWidth(minWidth),   minHeight(minHeight),
                  bestWidth(bestWidth), bestHeight(bestHeight),
                  maxWidth(maxWidth),   maxHeight(maxHeight),
                  incrWidth(1),         incrHeight(1)
        {}
        Measures(int minWidth, int minHeight, int bestWidth, int bestHeight, int maxWidth, int maxHeight,
                 int incrWidth,int incrHeight) 
                : minWidth(minWidth),   minHeight(minHeight),
                  bestWidth(bestWidth), bestHeight(bestHeight),
                  maxWidth(maxWidth),   maxHeight(maxHeight),
                  incrWidth(incrWidth), incrHeight(incrHeight)
        {}
        void maximize(const Measures& rhs);
        
        int bestWidth;
        int bestHeight;
        int minWidth;
        int minHeight;
        int maxWidth;
        int maxHeight;
        int incrWidth;
        int incrHeight;
    };
    
    
    class DesiredMeasuresChangedException : public BaseException
    {
    public:
        DesiredMeasuresChangedException()
            : BaseException("internal DesiredMeasuresChangedException")
        {}
        virtual const char* what() const throw();
    };

    class LayoutedElement
    {
    public:
        LayoutedElement(GuiElement::Ptr element,
                        LayoutOptions layoutOptions = LayoutOptions())
            : element(element),
              layoutOptions(layoutOptions)
        {}
        
        GuiElement* getPtr() const {
            return element;
        }
        
        LayoutOptions getLayoutOptions() const {
            return layoutOptions;
        }
        
    private:
        GuiElement::Ptr element;
        LayoutOptions layoutOptions;
    };


    bool isVisible() const {
        return visibility == VISIBLE;
    }
    
    Visibility getVisibility() const {
        return visibility;
    }
    
    virtual Measures getDesiredMeasures() { return Measures(0, 0, 0, 0, 0, 0); };
    virtual void setPosition(Position p) = 0;

    const Position& getPosition() const {
        return position;
    }

    virtual void show() {
        visibility = VISIBLE;
    }
    virtual void hide() {
        visibility = HIDDEN;
    }
    
protected:
    
    GuiElement()
        : visibility(VISIBLE) // UNDEFINED_VISIBILITY)
    {}
    
    GuiElement(int x, int y, unsigned int width, unsigned int height)
        : visibility(VISIBLE), // UNDEFINED_VISIBILITY),
          position(x, y, width, height)
    {}
    
    void treatNewWindowPosition(Position newPosition) {
        position = newPosition;
    }

private:
    Visibility visibility;
    Position position;
};


} // namespace LucED

#endif // GUI_ELEMENT_HPP
