/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <X11/Xatom.h>

#include "EventDispatcher.hpp"
#include "GuiRoot.hpp"

using namespace LucED;

SingletonInstance<EventDispatcher> EventDispatcher::instance;

EventDispatcher* EventDispatcher::getInstance()
{
    return instance.getPtr();
}

EventDispatcher::EventDispatcher()
    : doQuit(false),
      hasRootPropertyListeners(false)
{
    x11FileDescriptor = ConnectionNumber(GuiRoot::getInstance()->getDisplay());
}

void EventDispatcher::registerEventReceiver(const GuiWidget::EventRegistration& registration)
{
    widgetMap.set(registration.wid, registration.guiWidget);
}

void EventDispatcher::registerEventReceiverForForeignWidget(const GuiWidget::EventRegistration& registration)
{
    foreignWidgetListeners.set(registration.wid, registration.guiWidget);
}

void EventDispatcher::removeEventReceiver(const GuiWidget::EventRegistration& registration)
{
    widgetMap.remove(registration.wid);
    foreignWidgetListeners.remove(registration.wid);
}


bool EventDispatcher::isForeignWidget(Window wid)
{
    WidgetMap::Value foundWidget = widgetMap.get(wid);
    return !foundWidget.isValid();
}


EventDispatcher::TimerRegistration EventDispatcher::getNextTimer()
{
    EventDispatcher::TimerRegistration rslt;
    while (true) {
        if (timers.empty()) {
            return TimerRegistration();
        } else {
            rslt = timers.top();
                   timers.pop();
            if (rslt.isValid()) {
                return rslt;
            }
        }
    }
}


void EventDispatcher::processEvent(XEvent *event)
{
    if (event->type == MappingNotify) {
        if (event->xmapping.request == MappingKeyboard
                || event->xmapping.request == MappingModifier) {
            XRefreshKeyboardMapping(&event->xmapping);
        }
    } else {
        if (hasRootPropertyListeners && event->xany.window == rootWid
                                     && event->type == PropertyNotify)
        {
            GuiRootProperty property(event->xproperty.atom);
            RootPropertiesMap::Value foundListener = rootPropertyListeners.get(property);
            if (foundListener.isValid()) {
                Callback1<XEvent*> callback = foundListener.get();
                if (callback.isValid()) {
                    callback.call(event);
                } else {
                    rootPropertyListeners.remove(property);
                }
            }
        }
        else
        {
            WidgetMap::Value foundWidget = widgetMap.get(event->xany.window);
            if (foundWidget.isValid()) {
                foundWidget.get()->processEvent(event);
            } else {
                foundWidget = foreignWidgetListeners.get(event->xany.window);
                if (foundWidget.isValid()){
                    foundWidget.get()->processEvent(event);
                }   
            }
        }
    }
}

void EventDispatcher::doEventLoop()
{
    fd_set             readfds;
    fd_set             exceptfds;

    TimeVal            remainingTime;
    Display*           display = GuiRoot::getInstance()->getDisplay();
    XEvent             event;
    
    while (!doQuit)
    {
        
        invokeAllUpdateCallbacks();

        if (XEventsQueued(display, QueuedAfterFlush) > 0) {
            XNextEvent(display, &event);
            processEvent(&event);
        } else 
        {
            TimerRegistration nextTimer = getNextTimer();

            FD_ZERO(&readfds);
            FD_ZERO(&exceptfds);
            FD_SET(x11FileDescriptor, &readfds);
            FD_SET(x11FileDescriptor, &exceptfds);
            XFlush(display);
            //XSync (XGlobal_display, False);

            int p = 0;
            bool hasWaitingProcess = false;
            for (; p < processes.getLength();) {
                if (processes[p]->isEnabled()) {
                    if (processes[p]->needsProcessing()) {
                        hasWaitingProcess = true;
                        break;
                    }
                    ++p;
                } else {
                    processes.remove(p);
                }
            }
            
            int rslt;
            bool wasSelectInvoked = false;
            bool wasTimerInvoked = false;
            
            if (nextTimer.isValid()) {
                if (hasWaitingProcess) {
                    TimeVal now;
                    now.setToCurrentTime();
                    if (nextTimer.when.isLaterThan(now)) {
                        long diffTime = TimeVal::diffMicroSecs(now, nextTimer.when);
                        if (diffTime > 100 * 1000) {
                            diffTime = 100 * 1000;
                        }
                        ProcessHandler::Ptr h = processes[p];
                        processes.remove(p);
                        processes.append(h);
                        h->execute(diffTime);
                        
                        now.setToCurrentTime();
                        if (now.addMicroSecs(100 * 1000).isLaterThan(nextTimer.when)) {
                            remainingTime.setToRemainingTimeUntil(nextTimer.when);
                            rslt = LucED::select(x11FileDescriptor + 1, &readfds, NULL, NULL, &remainingTime);
                            wasSelectInvoked = true;
                        }
                    } else {
                        remainingTime.setToRemainingTimeUntil(nextTimer.when);
                        rslt = LucED::select(x11FileDescriptor + 1, &readfds, NULL, NULL, &remainingTime);
                        wasSelectInvoked = true;
                    }
                } else {
                    remainingTime.setToRemainingTimeUntil(nextTimer.when);
                    rslt = LucED::select(x11FileDescriptor + 1, &readfds, NULL, NULL, &remainingTime);
                    wasSelectInvoked = true;
                }
            } else {
                if (hasWaitingProcess) {
                    ProcessHandler::Ptr h = processes[p];
                    processes.remove(p);
                    processes.append(h);
                    h->execute(100 * 1000);
                } else {
                    rslt = LucED::select(x11FileDescriptor + 1, &readfds, NULL, NULL, NULL);
                    wasSelectInvoked = true;
                }
            }
            if (wasSelectInvoked) {
                if (rslt > 0) {
                    XNextEvent(display, &event);
                    processEvent(&event);
                } else {
                    if (nextTimer.isValid()) {
                        TimeVal now; 
                        now.setToCurrentTime();
                        if (now.isLaterThan(nextTimer.when)) {
                            nextTimer.callback.call();
                            wasTimerInvoked = true;
                        }
                    }
                }
            }
            if (nextTimer.isValid() && !wasTimerInvoked) {
                timers.push(nextTimer); // nextTimer was not executed -> queue it again
            }
        }
    }
    doQuit = false;
}

void EventDispatcher::registerUpdateSource(const UpdateCallback& updateCallback)
{
    updateCallbacks.registerCallback(updateCallback);
}

void EventDispatcher::invokeAllUpdateCallbacks()
{
    updateCallbacks.invokeAllCallbacks();
}

void EventDispatcher::registerProcess(ProcessHandler::Ptr process)
{
    processes.append(process);
    if (process->needsProcessing()) {process->execute(100 * 1000); process->getMicroSecs();}
}

ProcessHandler::Ptr EventDispatcher::getNextWaitingProcess()
{
    for (int i = 0; i < processes.getLength(); ++i) {
        if (processes[i]->needsProcessing()) {
            ProcessHandler::Ptr rslt = processes[i];
            processes.remove(i);
            processes.append(rslt);
            return rslt;
        }
    }
    return ProcessHandler::Ptr();
}

void EventDispatcher::deregisterAllUpdateSourceCallbacksFor(WeakPtr<HeapObject> callbackObject)
{
    updateCallbacks.deregisterAllCallbacksFor(callbackObject);
}


void EventDispatcher::registerEventReceiverForRootProperty(GuiRootProperty property,
                                                           Callback1<XEvent*> callback)
{
    if (!hasRootPropertyListeners) {
        GuiRoot* root = GuiRoot::getInstance();
        rootWid = root->getRootWid();
        XSelectInput(root->getDisplay(), rootWid, PropertyChangeMask);    
        hasRootPropertyListeners = true;
    }
    rootPropertyListeners.set(property, callback);
}



