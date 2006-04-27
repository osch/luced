#ifndef GUILAYOUTWIDGET_H
#define GUILAYOUTWIDGET_H

#include <string>

#include "GuiWidget.h"

namespace LucED {

using std::string;

class GuiLayoutWidget : public GuiWidget
{
public:
    typedef HeapObjectPtr<GuiLayoutWidget> Ptr;
    
    static Ptr create(GuiWidget* parent, int minWidth, int minHeight, int bestWidth, int bestHeight, int maxWidth, int maxHeight) {
        return Ptr(new GuiLayoutWidget(parent, Measures(minWidth, minHeight, bestWidth, bestHeight, maxWidth, maxHeight)));
    }

    //virtual bool processEvent(const XEvent *event);
    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position newPosition);
    
private:
    GuiLayoutWidget(GuiWidget* parent, const Measures& m);
    Measures measures;
    Position position;
};

} // namespace LucED

#endif // GUILAYOUTWIDGET_H
