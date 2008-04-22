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

#ifndef EVENTDISPATCHER_HPP
#define EVENTDISPATCHER_HPP

#include <queue>

#include "headers.hpp"
#include "options.hpp"
#include "HeapObject.hpp"
#include "GuiWidget.hpp"
#include "TimeVal.hpp"
#include "Callback.hpp"
#include "CallbackContainer.hpp"
#include "ProcessHandler.hpp"
#include "HashMap.hpp"
#include "SingletonInstance.hpp"
#include "GuiRootProperty.hpp"
#include "RunningComponent.hpp"
#include "Seconds.hpp"
#include "MicroSeconds.hpp"
#include "FileDescriptorListener.hpp"
#include "RawPtr.hpp"

namespace LucED {

using std::priority_queue;

class EventDispatcher : public HeapObject
{
public:
    static EventDispatcher* getInstance();
    
    void registerEventReceiver(const GuiWidget::EventRegistration& registration);
    void removeEventReceiver(const GuiWidget::EventRegistration& registration);
    
    void registerEventReceiverForForeignWidget(const GuiWidget::EventRegistration& registration);
    bool isForeignWidget(WidgetId wid);

    void registerUpdateSource(Callback<>::Ptr updateCallback);
    void deregisterAllUpdateSourceCallbacksFor(WeakPtr<HeapObject> callbackObject);
    
    void registerTimerCallback(const TimeVal& when, Callback<>::Ptr callback) {
        timers.push(TimerRegistration(when, callback));
    }
    void registerTimerCallback(Seconds secs, MicroSeconds usecs, Callback<>::Ptr callback) {
        TimeVal when;
        when.setToCurrentTime().add(secs, usecs);
        registerTimerCallback(when, callback);
    }

    bool processEvent(XEvent* event);
    void doEventLoop();
    void requestProgramTermination() {
        doQuit = true;
    }
    
    void registerProcess(ProcessHandler::Ptr process);

    void registerEventReceiverForRootProperty(GuiRootProperty property,
                                              Callback<XEvent*>::Ptr callback);

    void registerRunningComponent(OwningPtr<RunningComponent> runningComponent);
    
    void deregisterRunningComponent(RunningComponent* runningComponent);

    void registerFileDescriptorListener(FileDescriptorListener::Ptr fileDescriptorListener);
    
    void registerForTerminatingChildProcess(pid_t childPid, Callback<int>::Ptr callback);
    
    Time getLastX11Timestamp() {
        return lastX11EventTime;
    }
    
private:
    friend class SingletonInstance<EventDispatcher>;
    static SingletonInstance<EventDispatcher> instance;
    
    class TimerRegistration
    {
    public:
        friend class EventDispatcher;
        TimerRegistration() {}
        TimerRegistration(const TimeVal when, Callback<>::Ptr callback):
            when(when), callback(callback) {}
        bool operator<(const TimerRegistration& t) const {
            return this->when.isLaterThan(t.when);
        }
        bool isValid() {
            return callback->isEnabled();
        }
    private:
        TimeVal when;
        Callback<>::Ptr callback;
    };
    
    EventDispatcher();
    
    TimerRegistration getNextTimer();
    
    void invokeAllUpdateCallbacks();
    
    ProcessHandler::Ptr getNextWaitingProcess();
    
    typedef HashMap< WidgetId, RawPtr<GuiWidget> > WidgetMap;
    WidgetMap widgetMap;
    WidgetMap foreignWidgetListeners;
    
    priority_queue<TimerRegistration> timers;
    
    bool doQuit;
    int  x11FileDescriptor;

    CallbackContainer<> updateCallbacks;
    
    ObjectArray<ProcessHandler::Ptr> processes;
    
    typedef HashMap< GuiRootProperty, Callback<XEvent*>::Ptr > RootPropertiesMap;
    RootPropertiesMap rootPropertyListeners;
    bool hasRootPropertyListeners;
    WidgetId rootWid;
    
    
    ObjectArray< OwningPtr<RunningComponent> > runningComponents;
    ObjectArray< WeakPtr  <RunningComponent> >   stoppingComponents;

    ObjectArray<FileDescriptorListener::Ptr> fileDescriptorListeners;
    
    typedef HashMap< pid_t, Callback<int>::Ptr > ProcessListenerMap;
    ProcessListenerMap childProcessListeners;
    Time               lastX11EventTime;
};


} // namespace  LucED


#endif // EVENTDISPATCHER_HPP
