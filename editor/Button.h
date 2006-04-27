#ifndef BUTTON_H
#define BUTTON_H

#include <string>

#include "GuiWidget.h"

namespace LucED {

using std::string;

class Button : public GuiWidget
{
public:
    typedef HeapObjectPtr<Button> Ptr;
    
    static Ptr create(GuiWidget* parent, string label) {
        return Ptr(new Button(parent, label));
    }

    virtual bool processEvent(const XEvent *event);
    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position newPosition);
    
private:
    Button(GuiWidget* parent, string label);
    Position position;
    string label;
};

} // namespace LucED

#endif // BUTTON_H
