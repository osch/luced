/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch@luced.de
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

#ifndef EVENTDISPATCHER_H
#define EVENTDISPATCHER_H

#include <queue>

#include "options.h"
#include "HeapObject.h"
#include "GuiWidget.h"
#include "TimeVal.h"
#include "Callback.h"
#include "CallbackContainer.h"
#include "ProcessHandler.h"
#include "HashMap.h"
#include "SingletonInstance.h"

namespace LucED {

using std::priority_queue;

class EventDispatcher : public HeapObject
{
public:
    typedef Callback0 TimerCallback;
    typedef Callback0 UpdateCallback;
    
    static EventDispatcher* getInstance();
    
    void registerEventReceiver(const GuiWidget::EventRegistration& registration);
    void removeEventReceiver(const GuiWidget::EventRegistration& registration);
    
    void registerEventReceiverForForeignWidget(const GuiWidget::EventRegistration& registration);
    bool isForeignWidget(Window wid);

    void registerUpdateSource(const UpdateCallback& updateCallback);
    
    void registerTimerCallback(const TimeVal& when, const TimerCallback& callback) {
        timers.push(TimerRegistration(when, callback));
    }
    void registerTimerCallback(long secs, long usecs, const TimerCallback& callback) {
        TimeVal when;
        when.setToCurrentTime().addSecs(secs, usecs);
        registerTimerCallback(when, callback);
    }

    void processEvent(XEvent *event);
    void doEventLoop();
    void requestProgramTermination() {
        doQuit = true;
    }
    
    void registerProcess(ProcessHandler process);
    
private:
    friend class SingletonInstance<EventDispatcher>;
    
    class TimerRegistration
    {
    public:
        friend class EventDispatcher;
        TimerRegistration() {}
        TimerRegistration(const TimeVal when, const TimerCallback& callback):
            when(when), callback(callback) {}
        bool operator<(const TimerRegistration& t) const {
            return this->when.isLaterThan(t.when);
        }
        bool isValid() {
            return callback.isValid();
        }
    private:
        TimeVal when;
        TimerCallback callback;
    };
    
    EventDispatcher();
    
    TimerRegistration getNextTimer();
    
    void invokeAllUpdateCallbacks();
    
    ProcessHandler getNextWaitingProcess();
    
    typedef HashMap<Window, GuiWidget*> WidgetMap;
    WidgetMap widgetMap;
    WidgetMap foreignWidgetListeners;
    
    priority_queue<TimerRegistration> timers;
    
    bool doQuit;
    int  x11FileDescriptor;

    Callback0Container updateCallbacks;
    
    ObjectArray<ProcessHandler> processes;
    
};


} // namespace  LucED


#endif // EVENTDISPATCHER_H
