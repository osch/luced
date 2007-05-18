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

#ifndef GUIROOTPROPERTY_H
#define GUIROOTPROPERTY_H

#include <X11/Xatom.h>

#include "OwningPtr.hpp"
#include "GuiRoot.hpp"

namespace LucED {



class GuiRootProperty
{
public:
    
    GuiRootProperty()
        : atom(0)
    {}
        
    explicit GuiRootProperty(String propertyName)
    {
        GuiRoot* root = GuiRoot::getInstance();
        atom = XInternAtom(root->getDisplay(), propertyName.toCString(), False);
    }
    
    explicit GuiRootProperty(Atom atom)
        : atom(atom)
    {}
    
    void setValue(String value)
    {
        if (atom != 0) {
            GuiRoot* root = GuiRoot::getInstance();
            XChangeProperty(root->getDisplay(), root->getRootWid(), atom, XA_STRING, 8,
                PropModeReplace, (unsigned char *)value.toCString(), value.getLength());
        }
    }
    
    String getValue() const
    {
        return getValue(false);
    }

    String getValueAndRemove() const
    {
        return getValue(true);
    }


    bool exists() const
    {
        bool rslt = false;

        if (atom != 0)
        {
            GuiRoot*       root = GuiRoot::getInstance();
            Atom           actualTypeAtom;
            int            format;
            unsigned long  numberItems;
            unsigned long  numberBytesLeft;
            unsigned char* propertyValue;

            if (XGetWindowProperty(root->getDisplay(), root->getRootWid(), atom, 0, 0, False,
                                   XA_STRING, &actualTypeAtom, &format, &numberItems, &numberBytesLeft,
                                   &propertyValue)
                 == Success)
            {
                if (actualTypeAtom == XA_STRING && format == 8) {
                    rslt = true;
                }
                XFree(propertyValue);
            }
        }
        return rslt;
    }

    void remove()
    {
        if (atom != 0)
        {
            GuiRoot* root = GuiRoot::getInstance();
            XDeleteProperty(root->getDisplay(), root->getRootWid(), atom);
        }
    }

    static void listAllProperties()
    {
        GuiRoot*  root = GuiRoot::getInstance();
        int       numberProperties;
        
        Atom* atoms = XListProperties(root->getDisplay(), root->getRootWid(), &numberProperties);
        
        for (int i = 0; i < numberProperties; ++i)
        {
            Atom atom = atoms[i];
            char* name = XGetAtomName(root->getDisplay(), atom); 
            
            printf("Property '%s' = '%s'\n", name, GuiRootProperty(name).getValue().toCString());
            
            XFree(name);
        }
        
        XFree(atoms);
    }
    
    Atom getAtom() const
    {
        return atom;
    }
    
    bool operator==(const GuiRootProperty& rhs) const
    {
        return atom == rhs.atom;
    }
    
private:
    String getValue(bool remove) const
    {
        String rslt;
    
        if (atom != 0)
        {
            GuiRoot*       root = GuiRoot::getInstance();
            Atom           actualTypeAtom;
            int            format;
            unsigned long  numberItems;
            unsigned long  numberBytesLeft;
            unsigned char* propertyValue;

            if (XGetWindowProperty(root->getDisplay(), root->getRootWid(), atom, 0, INT_MAX, remove,
                                   XA_STRING, &actualTypeAtom, &format, &numberItems, &numberBytesLeft,
                                   &propertyValue)
                 == Success)
            {
                if (actualTypeAtom == XA_STRING && format == 8) {
                    rslt = String((const char*)propertyValue, numberItems);
                }
                XFree(propertyValue);
            }
        }
        return rslt;
    }

    Atom atom;
};


template<class T> class HashFunction;

template<> class HashFunction<GuiRootProperty>
{
public:
    size_t operator()(GuiRootProperty key) const {
        return (size_t) key.getAtom();
    }
};

} // namespace LucED

#endif // GUIROOTPROPERTY_H
