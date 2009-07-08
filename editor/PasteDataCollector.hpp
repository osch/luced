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

#ifndef PASTE_DATA_COLLECTOR_HPP
#define PASTE_DATA_COLLECTOR_HPP

#include "String.hpp"
#include "PasteDataReceiver.hpp"
#include "OwningPtr.hpp"

namespace LucED
{

template
<
    class T
>
class PasteDataCollector : public PasteDataReceiver::ContentHandler
{
public:
    typedef OwningPtr<PasteDataCollector> Ptr;

    static Ptr create(RawPtr<T> referingWidget) {
        return Ptr(new PasteDataCollector(referingWidget));
    }
    
    virtual void notifyAboutBeginOfPastingData() {
        collectedData.clear();
    }
    virtual void notifyAboutReceivedPasteData(const byte* data, long length) {
        collectedData.append(data, length);
    }
    virtual void notifyAboutEndOfPastingData() {
        String rslt = collectedData;
                      collectedData.clear();
        referingWidget->treatCollectedSelectionData(rslt);
    }
private:
    PasteDataCollector(RawPtr<T> referingWidget)
        : referingWidget(referingWidget)
    {}
    RawPtr<T> referingWidget;
    String collectedData;
};

} // namespace LucED

#endif // PASTE_DATA_COLLECTOR_HPP
