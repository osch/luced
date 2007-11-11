#ifndef WIDGET_ID_HPP
#define WIDGET_ID_HPP

#include "headers.hpp"

namespace LucED
{

class WidgetId
{
#ifdef WIN32_GUI

public:
    WidgetId()
        : hWnd()
    {}
    
    explicit WidgetId(HWND hWnd)
        : hWnd(hWnd)
    {}
    
    operator HWND() const {
        return hWnd;
    }
private:
    HWND hWnd;

#else

public:

    WidgetId()
        : wid()
    {}
    explicit WidgetId(Window wid)
        : wid(wid)
    {}
    
    operator Window() const {
        return wid;
    }
private:
    Window wid;

#endif
};

template<class T> class HashFunction;

template<> class HashFunction<WidgetId>
{
public:
    size_t operator()(const WidgetId& key) const
    {
    #ifdef WIN32_GUI
        return HWND(key);
    #else
        return Window(key);
    #endif
    }
};

} // namespace LucED

#endif // WIDGET_ID_HPP
