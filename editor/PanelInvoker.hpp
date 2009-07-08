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

#ifndef PANEL_INVOKER_HPP
#define PANEL_INVOKER_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "DialogPanel.hpp"
#include "Callback.hpp"
#include "RawPtr.hpp"

namespace LucED
{

class PanelInvoker : public HeapObject
{
public:
    typedef OwningPtr<PanelInvoker> Ptr;

    virtual void invokePanel(DialogPanel::Ptr panel) = 0;
    virtual bool hasInvokedPanel() = 0;
    virtual void closeInvokedPanel() = 0;
    virtual void closePanel(RawPtr<DialogPanel> panel) = 0;
    
    Callback< RawPtr<DialogPanel> >::Ptr getCloseCallback() {
        if (!closeCallback.isValid()) {
            closeCallback = newCallback(this, &PanelInvoker::closePanel);
        }
        return closeCallback;
    }
    
protected:
    PanelInvoker()
    {}
    
    Callback< RawPtr<DialogPanel> >::Ptr closeCallback;    
};

} // namespace LucED

#endif // PANEL_INVOKER_HPP
