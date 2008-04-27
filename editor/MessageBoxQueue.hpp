#ifndef MESSAGE_BOX_QUEUE_HPP
#define MESSAGE_BOX_QUEUE_HPP

#include "WeakPtrQueue.hpp"
#include "OwningPtr.hpp"
#include "TopWin.hpp"

namespace LucED
{

class MessageBox;

class MessageBoxQueue : public WeakPtrQueue<MessageBox>
{
public:
    typedef OwningPtr<MessageBoxQueue> Ptr;
    
    static Ptr create() {
        return Ptr(new MessageBoxQueue());
    }
    
    void closeQueued(TopWin::CloseReason reason = TopWin::CLOSED_SILENTLY);
};

} // namespace LucED

#include "MessageBox.hpp"

namespace LucED
{

inline void MessageBoxQueue::closeQueued(TopWin::CloseReason reason)
{
    while (getLength() > 0) {
        WeakPtr<MessageBox> m = getLast();
        if (m.isValid()) {
            m->requestCloseWindow(reason);
        }
        removeLast();
    }
}

} // namespace LucED

#endif // MESSAGE_BOX_QUEUE_HPP
