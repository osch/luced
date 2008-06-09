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

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <X11/Xatom.h>

#include "util.hpp"
#include "EventDispatcher.hpp"
#include "GuiRoot.hpp"
#include "System.hpp"
#include "SystemException.hpp"

#ifdef DEBUG
    #include "TopWin.hpp"
#endif

using namespace LucED;

SingletonInstance<EventDispatcher> EventDispatcher::instance;

EventDispatcher* EventDispatcher::getInstance()
{
    return instance.getPtr();
}

static bool hasSignalHandlers = false;
static int sigChildPipeIn   = -1;
static int sigChildPipeOut  = -1;
static sigset_t enabledSignalBlockMask;
static sigset_t disabledSignalBlockMask;


static void sigchildHandler(int signal)
{
    switch (signal)
    {
        case SIGCHLD: {
            if (hasSignalHandlers) {
                char msg = 'x';
                ::write(sigChildPipeOut, &msg, 1);
            }
            break;
        }
    }
}

static inline void enableSignals()
{
    ASSERT(hasSignalHandlers);
    
    if (sigprocmask(SIG_SETMASK, &enabledSignalBlockMask, NULL) != 0) {
        throw SystemException(String() << "Could not call sigprocmask: " << strerror(errno));
    }
}

static inline void disableSignals()
{
    ASSERT(hasSignalHandlers);

    if (sigprocmask(SIG_SETMASK, &disabledSignalBlockMask, NULL) != 0) {
        throw SystemException(String() << "Could not call sigprocmask: " << strerror(errno));
    }
}

static inline int internalSelect(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, TimeVal* timeVal)
{
#if 0
  TimeVal diffTimeVal;
  if (timeVal != NULL) {
    diffTimeVal = *timeVal;
  }
  TimeVal oldTimeVal = TimeVal::NOW;
#endif

    enableSignals();
    int rslt = System::select(n, readfds, writefds, exceptfds, timeVal);
    disableSignals();

#if 0
  if (timeVal != NULL)
  {
    TimeVal newTimeVal = TimeVal::NOW;
    TimeVal oldTimeVal2 = oldTimeVal + diffTimeVal;
    if (oldTimeVal2 < newTimeVal)
    {
        MicroSeconds msecs = TimeVal::diffMicroSecs(oldTimeVal2, newTimeVal);
        if (msecs > 50 * 1000) {
            printf("--------------------- %ld --- (%ld:%ld) -> (%ld:%ld)  : (%ld:%ld)\n", (long) msecs, 
                                                                                          (long) oldTimeVal.getSeconds(),
                                                                                          (long) oldTimeVal.getMicroSeconds(),
                                                                                          (long) newTimeVal.getSeconds(),
                                                                                          (long) newTimeVal.getMicroSeconds(),
                                                                                          (long) diffTimeVal.getSeconds(),
                                                                                          (long) diffTimeVal.getMicroSeconds());
        }
    }
  }
#endif
    return rslt;
}


EventDispatcher::EventDispatcher()
    : doQuit(false),
      hasRootPropertyListeners(false),
      lastX11EventTime(CurrentTime)
{
    
    x11FileDescriptor = ConnectionNumber(GuiRoot::getInstance()->getDisplay());

    System::setCloseOnExecFlag(x11FileDescriptor);

    if (!hasSignalHandlers)
    {
        int sigChildPipe[2];
    
        if (::pipe(sigChildPipe) != 0) {
            throw SystemException(String() << "Could not create pipe: " << strerror(errno));
        }
        
        sigChildPipeIn  = sigChildPipe[0];
        sigChildPipeOut = sigChildPipe[1];
    
        System::setCloseOnExecFlag(sigChildPipeIn);
        System::setCloseOnExecFlag(sigChildPipeOut);
        {
            struct sigaction handler;
       
            handler.sa_handler = sigchildHandler;
            ::sigfillset(&handler.sa_mask);
            handler.sa_flags = SA_NOCLDSTOP;
           
        #ifdef SA_RESTART
            handler.sa_flags |= SA_RESTART;
        #endif
        #ifdef SA_INTERRUPT
            handler.sa_flags &= ~SA_INTERRUPT;
        #endif
       
            if (::sigaction(SIGCHLD, &handler, NULL) != 0) {
                throw SystemException(String() << "Could not call sigaction: " << strerror(errno));
            }
            
            sigfillset(&enabledSignalBlockMask);
            sigfillset(&disabledSignalBlockMask);
            
            sigdelset(&enabledSignalBlockMask, SIGCHLD);
            
        }
        hasSignalHandlers = true;
    }
    disableSignals();
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


bool EventDispatcher::isForeignWidget(WidgetId wid)
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


bool EventDispatcher::processEvent(XEvent* event)
{
    bool hasSomethingDone = false;
    
    if (event->type == MappingNotify) {
        if (event->xmapping.request == MappingKeyboard
                || event->xmapping.request == MappingModifier) {
            XRefreshKeyboardMapping(&event->xmapping);
        }
    } else {
        WidgetId widgetId = WidgetId(event->xany.window);
        
        if (hasRootPropertyListeners && widgetId == rootWid
                                     && event->type == PropertyNotify)
        {
            lastX11EventTime = event->xproperty.time;
            GuiRootProperty property(event->xproperty.atom);
            RootPropertiesMap::Value foundListener = rootPropertyListeners.get(property);
            if (foundListener.isValid()) {
                Callback<XEvent*>::Ptr callback = foundListener.get();
                if (callback->isEnabled()) {
                    callback->call(event);
                    hasSomethingDone = true;
                } else {
                    rootPropertyListeners.remove(property);
                }
            }
        }
        else
        {
            switch (event->type) {
                case KeyPress:
                case KeyRelease:       lastX11EventTime = event->xkey.time; 
                                       break;
                case ButtonPress:
                case ButtonRelease:    lastX11EventTime = event->xbutton.time; 
                                       break;
                case MotionNotify:     lastX11EventTime = event->xmotion.time; 
                                       break;
                case EnterNotify:
                case LeaveNotify:      lastX11EventTime = event->xcrossing.time; 
                                       break;
                case SelectionNotify:  lastX11EventTime = event->xselection.time; 
                                       break;
                case SelectionClear:   lastX11EventTime = event->xselectionclear.time; 
                                       break;
                case SelectionRequest: lastX11EventTime = event->xselectionrequest.time; 
                                       break;
            }
            WidgetMap::Value foundWidget = widgetMap.get(widgetId);
            if (foundWidget.isValid()) {
                foundWidget.get()->processEvent(event);
                hasSomethingDone = true;
            } else {
                foundWidget = foreignWidgetListeners.get(widgetId);
                if (foundWidget.isValid()){
                    foundWidget.get()->processEvent(event);
                    hasSomethingDone = true;
                }   
            }
        }
    }
    return hasSomethingDone;
}

void EventDispatcher::doEventLoop()
{
    fd_set             readfds;
    fd_set             writefds;
    fd_set             exceptfds;

    XEvent             event;
    TimeVal            remainingTime;
    Display*           display = GuiRoot::getInstance()->getDisplay();
    
    bool hasSomethingDone = true;
    
    while (!doQuit)
    {
        if (hasSomethingDone) {
            invokeAllUpdateCallbacks();
            hasSomethingDone = false;   
        }

        if (XEventsQueued(display, QueuedAfterFlush) > 0) {
            XNextEvent(display, &event);
            hasSomethingDone = processEvent(&event);
        } 
        else if (stoppingComponents.getLength() == 0)
        {
            XFlush(display);
            //XSync(display, False); <-- not this here!

            TimerRegistration nextTimer = getNextTimer();
            
            FD_ZERO(&readfds);
            FD_ZERO(&writefds);
            FD_ZERO(&exceptfds);
            FD_SET(x11FileDescriptor, &readfds);
            FD_SET(x11FileDescriptor, &exceptfds);
            int maxFileDescriptor = x11FileDescriptor;
            
            util::maximize(&maxFileDescriptor, sigChildPipeIn);
            FD_SET(sigChildPipeIn, &readfds);
            
            for (int i = 0; i < fileDescriptorListeners.getLength();)
            {
                FileDescriptorListener::Ptr listener = fileDescriptorListeners[i]; 
                if (listener->isWaitingForRead())
                {
                    util::maximize(&maxFileDescriptor, listener->getFileDescriptor());
                    FD_SET(listener->getFileDescriptor(), &readfds);
                    if (listener->isWaitingForWrite()) {
                        FD_SET(listener->getFileDescriptor(), &writefds);
                    }
                    ++i;
                }
                else if (listener->isWaitingForWrite())
                {
                    util::maximize(&maxFileDescriptor, listener->getFileDescriptor());
                    FD_SET(listener->getFileDescriptor(), &writefds);
                    ++i;
                }
                else {
                    fileDescriptorListeners.remove(i);
                }
            }

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
            
            int selectResult;
            bool wasSelectInvoked = false;
            bool wasTimerInvoked = false;
            
            {
                if (nextTimer.isValid()) {
#if 0
    TopWin::checkTopWinFocus();
#endif
                    if (hasWaitingProcess) {
                        TimeVal now = TimeVal::now();
                        if (nextTimer.when > now) {
                            long diffTime = TimeVal::diffMicroSecs(now, nextTimer.when);
                            if (diffTime > 100 * 1000) {
                                diffTime = 100 * 1000;
                            }
                            ProcessHandler::Ptr h = processes[p];
                            processes.remove(p);
                            processes.append(h);
                            h->execute(diffTime);
                            hasSomethingDone = true;
                            
                            if (nextTimer.when < TimeVal::now() + MicroSeconds(100 * 1000)) {
                                remainingTime.setToRemainingTimeUntil(nextTimer.when);
                                selectResult = internalSelect(maxFileDescriptor + 1, &readfds, &writefds, NULL, &remainingTime);
                                wasSelectInvoked = true;
                            }
                        } else {
                            remainingTime.setToRemainingTimeUntil(nextTimer.when);
                            selectResult = internalSelect(maxFileDescriptor + 1, &readfds, &writefds, NULL, &remainingTime);
                            wasSelectInvoked = true;
                        }
                    } else {
                        remainingTime.setToRemainingTimeUntil(nextTimer.when);
                            
                        selectResult = internalSelect(maxFileDescriptor + 1, &readfds, &writefds, NULL, &remainingTime);
                        
                        wasSelectInvoked = true;
                    }
                } else {
                    if (hasWaitingProcess) {
                        ProcessHandler::Ptr h = processes[p];
                        processes.remove(p);
                        processes.append(h);
                        h->execute(100 * 1000);
                        hasSomethingDone = true;
                    } else {
                    
                        selectResult = internalSelect(maxFileDescriptor + 1, &readfds, &writefds, NULL, NULL);
                        wasSelectInvoked = true;
                    }
                }
            }
            
            if (wasSelectInvoked) {
                if (selectResult > 0) {
                    if (FD_ISSET(x11FileDescriptor, &readfds)) {
                        XNextEvent(display, &event);
                        hasSomethingDone = processEvent(&event);
                    }
                    for (int i = 0; i < fileDescriptorListeners.getLength(); ++i)
                    {
                        FileDescriptorListener::Ptr listener = fileDescriptorListeners[i];
                        int                         fd       = listener->getFileDescriptor();
                    
                        if (FD_ISSET(fd, &readfds)) {
                            listener->handleReading();
                            hasSomethingDone = true;
                        }
                        if (FD_ISSET(fd, &writefds)) {
                            listener->handleWriting();
                            hasSomethingDone = true;
                        }
                    }
                    if (FD_ISSET(sigChildPipeIn, &readfds)) {
                        char buffer[40];
                        int readCounter = ::read(sigChildPipeIn, buffer, sizeof(buffer));

                        int status;
                        pid_t pid = 0;

                        do {
                            pid = waitpid(-1, &status, WNOHANG);
                            if (pid == -1) {
                                if (errno != ECHILD) {
                                    throw SystemException(String() << "Error while calling waitpid: " << strerror(errno));
                                }
                                pid = 0;
                            }
                            if (pid != 0)
                            {
                                ProcessListenerMap::Value foundListener = childProcessListeners.get(pid);
                                if (foundListener.isValid())
                                {
                                    int returnCode = -1;
                                    if (WIFEXITED(status)) {
                                        returnCode = WEXITSTATUS(status);
                                    }
                                    foundListener.get()->call(returnCode);
                                    childProcessListeners.remove(pid);
                                    hasSomethingDone = true;
                                }
                            }
                        } while (pid != 0);
                    }
                } else {
                    if (nextTimer.isValid()) {
                        if (nextTimer.when < TimeVal::now()) {
                            nextTimer.callback->call();
                            hasSomethingDone = true;
                            wasTimerInvoked = true;
                        }
                    }
                }
            }
            if (nextTimer.isValid() && !wasTimerInvoked) {
                timers.push(nextTimer); // nextTimer was not executed -> queue it again
            }
        }
        
        if (stoppingComponents.getLength() > 0)
        {
            for (int i = 0; i < stoppingComponents.getLength(); ++i)
            {
                if (stoppingComponents[i].isValid())
                {
                    for (int j = 0; j < runningComponents.getLength();)
                    {
                        if (runningComponents[j].isInvalid()) {
                            runningComponents.remove(j);
                        } else if (runningComponents[j] == stoppingComponents[i]) {
                            runningComponents.remove(j);
                        } else {
                            ++j;
                        }
                    }
                }
            }
            stoppingComponents.clear();
            
            if (runningComponents.getLength() == 0) {
                requestProgramTermination();
            }
            hasSomethingDone = true;
        }
    }
    doQuit = false;
}

void EventDispatcher::registerUpdateSource(Callback<>::Ptr updateCallback)
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
                                                           Callback<XEvent*>::Ptr callback)
{
    if (!hasRootPropertyListeners) {
        GuiRoot* root = GuiRoot::getInstance();
        rootWid = root->getRootWid();
        XSelectInput(root->getDisplay(), rootWid, PropertyChangeMask);    
        hasRootPropertyListeners = true;
    }
    rootPropertyListeners.set(property, callback);
}



void EventDispatcher::registerRunningComponent(OwningPtr<RunningComponent> runningComponent)
{
    runningComponents.append(runningComponent);
}


void EventDispatcher::deregisterRunningComponent(RunningComponent* runningComponent)
{
    stoppingComponents.append(runningComponent);
}

void EventDispatcher::registerFileDescriptorListener(FileDescriptorListener::Ptr fileDescriptorListener)
{
    fileDescriptorListeners.append(fileDescriptorListener);
}

void EventDispatcher::registerForTerminatingChildProcess(pid_t childPid, Callback<int>::Ptr callback)
{
    childProcessListeners.set(childPid, callback);
}

