#ifndef BUTTON_H
#define BUTTON_H

#include <string>

#include "GuiWidget.h"
#include "OwningPtr.h"

namespace LucED {

using std::string;

class Button : public GuiWidget
{
public:
    typedef OwningPtr<Button> Ptr;
    
    static Ptr create(GuiWidget* parent, string buttonText) {
        return Ptr(new Button(parent, buttonText));
    }

    virtual bool processEvent(const XEvent *event);
    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position newPosition);
    
private:
    void drawButton();
    bool isMouseInsideButtonArea(int mouseX, int mouseY);
    
    Button(GuiWidget* parent, string buttonText);
    Position position;
    string buttonText;
    bool isButtonPressed;
    bool isMouseButtonPressed;
    bool isMouseOverButton;
};

} // namespace LucED

#endif // BUTTON_H
