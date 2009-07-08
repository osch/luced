/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#include "GuiLayoutAdapter.hpp"

#ifndef GUI_ELEMENT_HPP
#define GUI_ELEMENT_HPP

#include <X11/Xlib.h>

#include "HeapObject.hpp"
#include "Position.hpp"
#include "OwningPtr.hpp"
#include "WeakPtr.hpp"
#include "Flags.hpp"
#include "BaseException.hpp"
#include "ObjectArray.hpp"
#include "RawPtr.hpp"
#include "GuiRoot.hpp"
                
namespace LucED
{

class FocusManager;
class GuiWidget;

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
        VISIBLE,
        HIDDEN
    };
    
    typedef Flags<LayoutOption> LayoutOptions;
    typedef GuiMeasures Measures;
    
    class DesiredMeasuresChangedException : public BaseException
    {
    public:
        DesiredMeasuresChangedException()
            : BaseException("internal DesiredMeasuresChangedException")
        {}
        virtual const char* what() const throw();
    };
    
    LayoutOptions getLayoutOptions() const {
        return layoutOptions;
    }

    bool isVisible() const {
        return visibility == VISIBLE;
    }
    
    Visibility getVisibility() const {
        return visibility;
    }
    
    Measures getDesiredMeasures() {
        if (layoutAdapter.isValid()) {
            return layoutAdapter->getDesiredMeasures();
        } else {
            return internalGetDesiredMeasures();
        }
    }
    
    class GuiLayoutAdapterAccess
    {
        friend class GuiLayoutAdapter;
        
        static Measures internalGetDesiredMeasures(RawPtr<GuiElement> element) {
            return element->internalGetDesiredMeasures();
        }
    };
    
    virtual void setPosition(const Position& p);

    const Position& getPosition() const {
        return position;
    }
    
    virtual void adopt(RawPtr<GuiElement>   parentElement,
                       RawPtr<GuiWidget>    parentWidget,
                       RawPtr<FocusManager> focusManagerForChilds);

    virtual void show();
    virtual void hide();

    int getNumberOfChildElements() {
        return childElements.getLength();
    }
    GuiElement::Ptr getChildElement(int index) {
        return childElements[index];
    }
    GuiElement::Ptr getRootElement() {
        return rootElement;
    }
    
    void setLayoutOptions(LayoutOptions layoutOptions) {
        this->layoutOptions = layoutOptions;
    }
    void setLayoutAdapter(OwningPtr<GuiLayoutAdapter> layoutAdapter) {
        this->layoutAdapter = layoutAdapter;
    }
    
    bool wasAdopted() const {
        return wasAdoptedFlag;
    }
    
    bool wasAdopted(RawPtr<GuiElement>   parentElement,
                    RawPtr<GuiWidget>    parentWidget) const
    {
        if (wasAdoptedFlag) {
            ASSERT(this->parentElement == parentElement);
            ASSERT(this->parentWidget  == parentWidget);
        }
        return wasAdoptedFlag;
    }

protected:
    static Display* getDisplay() { return GuiRoot::getInstance()->getDisplay(); }
    static GuiRoot* getGuiRoot() { return GuiRoot::getInstance(); }

    GuiElement(Visibility defaultVisibility = VISIBLE, int x = 0, int y = 0, int width = 0, int height = 0)
        : position(x, y, width, height),
          visibility(defaultVisibility),
          wasAdoptedFlag(false)
    {}
    
    virtual Measures internalGetDesiredMeasures() = 0;

    GuiElement::Ptr addChildElement(GuiElement::Ptr child) {
        if (wasAdoptedFlag) {
            internalAdoptChild(child);
        }
        childElements.append(child);
        return child;
    }
    
    GuiElement::Ptr setChildElement(int index, GuiElement::Ptr child) {
        if (wasAdoptedFlag) {
            internalAdoptChild(child);
        }
        childElements[index] = child;
        return child;
    }
    GuiElement::Ptr setRootElement(GuiElement::Ptr child) {
        if (wasAdoptedFlag) {
            internalAdoptChild(child);
        }
        rootElement = child;
        return child;
    }
    
    GuiElement::Ptr insertChildElement(int index, GuiElement::Ptr child) {
        if (wasAdoptedFlag) {
            internalAdoptChild(child);
        }
        childElements.insert(index, child);
        return child;
    }
    
    void removeChildElement(int index) {
        childElements.remove(index);
    }
    
private:
    void internalAdoptChild(RawPtr<GuiElement> child);

    Visibility visibility;
    Position position;
    
    LayoutOptions layoutOptions;

    bool wasAdoptedFlag;
    RawPtr<GuiElement>   parentElement;
    RawPtr<GuiWidget>    parentWidget;
    RawPtr<FocusManager> focusManagerForChilds;

    OwningPtr<GuiElement>                rootElement;
    ObjectArray< OwningPtr<GuiElement> > childElements;

    OwningPtr<GuiLayoutAdapter> layoutAdapter;
};


} // namespace LucED

#endif // GUI_ELEMENT_HPP
