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

#ifndef FOCUS_MANAGER_HPP
#define FOCUS_MANAGER_HPP

#include "KeyMapping.hpp"
#include "RawPtr.hpp"
#include "RawPointable.hpp"
#include "NonCopyable.hpp"

namespace LucED
{

class FocusManager;
class FocusableElement;

class FocusManager : public RawPointable
{
public:
    virtual void requestHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w) = 0;
    virtual void requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w) = 0;
    virtual void requestFocusFor(RawPtr<FocusableElement> w) = 0;
    virtual void reportMouseClickFrom(RawPtr<FocusableElement> w) = 0;

    
    class Access
    {
        friend class FocusableElement;
        
        static bool isReady(RawPtr<FocusManager> focusManager) {
            return focusManager->queueTransactionCounter == 0;
        }
        static void queueHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w);
        static void queueRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w);
        static void queueFocusFor(RawPtr<FocusableElement> w);

        class QueueTransaction : public NonCopyable
        {
        public: 
            QueueTransaction(RawPtr<FocusManager> focusManager)
                : focusManager(focusManager)
            {
                if (focusManager.isValid()) {
                    focusManager->queueTransactionCounter += 1;
                }
            }
            
            void execute() const {
                if (focusManager.isValid() && focusManager->queueTransactionCounter == 1) {
                    internalProcessQueuedFor(focusManager);
                }
            }
            
            ~QueueTransaction()
            {
                if (focusManager.isValid()) {
                    focusManager->queueTransactionCounter -= 1;
                }
            }

        private:
            RawPtr<FocusManager> focusManager;
        };
    };
    
protected:
    FocusManager()
        : queueTransactionCounter(0)
    {}

private:
    class RequestQueue;
    
    static void internalProcessQueuedFor(RawPtr<FocusManager> focusManager);
    
    int queueTransactionCounter;
};

} // namespace LucED

#endif // FOCUS_MANAGER_HPP
