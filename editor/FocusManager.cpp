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

#include "FocusManager.hpp"
#include "SingletonInstance.hpp"
#include "FocusableElement.hpp"

using namespace LucED;

class FocusManager::RequestQueue : public HeapObject
{
public:
    static RawPtr<RequestQueue> getInstance() { return instance.getPtr(); }

    enum RequestType { HOT_KEY_REGISTRATION_REQUEST,
                       HOT_KEY_REMOVAL_REQUEST,
                       FOCUS_REQUEST };
    
    void append(RequestType              requestType, 
                RawPtr<FocusableElement> element, 
                Nullable<KeyMapping::Id> keyMappingId = Null)
    {
        entries.append(Entry(requestType, element, keyMappingId));
    }
    
    void processQueueFor(RawPtr<FocusManager> focusManager)
    {
        ASSERT(focusManager.isValid());
        
        for (int i = 0; i < entries.getLength(); )
        {
            if (!entries[i].element.isValid()) {
                entries.remove(i);
            } else {
                if (entries[i].element->hasThisFocusManager(focusManager))
                {
                    switch (entries[i].requestType)
                    {
                        case HOT_KEY_REGISTRATION_REQUEST: 
                        {
                            focusManager->requestHotKeyRegistrationFor(entries[i].keyMappingId,
                                                                       entries[i].element);
                            break;
                        }
                        case HOT_KEY_REMOVAL_REQUEST:
                        {
                            focusManager->requestRemovalOfHotKeyRegistrationFor(entries[i].keyMappingId,
                                                                                entries[i].element);
                            break;
                        }
                        case FOCUS_REQUEST:
                        {
                            focusManager->requestFocusFor(entries[i].element);
                            break;
                        }
                    }
                    entries.remove(i);
                }
                else {
                    ++i;
                }
            }
        }
    }
private:
    friend class SingletonInstance<RequestQueue>;

    static SingletonInstance<RequestQueue> instance;

    RequestQueue()
    {}
    
    struct Entry
    {
        Entry(RequestType              requestType, 
              RawPtr<FocusableElement> element, 
              Nullable<KeyMapping::Id> keyMappingId)
            : requestType(requestType),
              element(element),
              keyMappingId(keyMappingId)
        {}
        RequestType               requestType;
        WeakPtr<FocusableElement> element;
        Nullable<KeyMapping::Id>  keyMappingId;
    };
    
    ObjectArray<Entry> entries;
};

SingletonInstance<FocusManager::RequestQueue> FocusManager::RequestQueue::instance;


void FocusManager::Access::queueHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w)
{
    RequestQueue::getInstance()->append(RequestQueue::HOT_KEY_REGISTRATION_REQUEST,
                                        w,
                                        id);
}
void FocusManager::Access::queueRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w)
{
    RequestQueue::getInstance()->append(RequestQueue::HOT_KEY_REMOVAL_REQUEST,
                                        w,
                                        id);
}
void FocusManager::Access::queueFocusFor(RawPtr<FocusableElement> w)
{
    RequestQueue::getInstance()->append(RequestQueue::FOCUS_REQUEST,
                                        w);
}
void FocusManager::internalProcessQueuedFor(RawPtr<FocusManager> focusManager)
{
    RequestQueue::getInstance()->processQueueFor(focusManager);
}


