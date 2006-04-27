#include "Button.h"
#include "GuiRoot.h"

using namespace LucED;

Button::Button(GuiWidget* parent, string label)
      : GuiWidget(parent, 0, 0, 1, 1, 0),
        position(0, 0, 1, 1),
        label(label)
{
    addToXEventMask(ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);
    setBackgroundColor(getGuiRoot()->getGuiColor03());
}

GuiElement::Measures Button::getDesiredMeasures()
{
    int minWidth = getGuiTextStyle()->getTextWidth(label) + 2 * getRaisedBoxBorderWidth() + 6;
    int minHeight = getGuiTextHeight() + 2 * getRaisedBoxBorderWidth() + 6;

    int bestWidth = minWidth + 4 * getGuiTextStyle()->getSpaceWidth();
    int bestHeight = minHeight;

    return Measures(minWidth, minHeight, bestWidth, bestHeight, bestWidth, bestHeight);
}


void Button::setPosition(Position newPosition)
{
    if (position != newPosition) {
        GuiWidget::setPosition(newPosition);
        this->position = newPosition;
    }
}

bool Button::processEvent(const XEvent *event)
{
    if (GuiWidget::processEvent(event)) {
        return true;
    } else {
        
        switch (event->type) {
            
            case GraphicsExpose:
                if (event->xgraphicsexpose.count > 0) {
                    break;
                }
            case Expose: {
                if (event->xexpose.count > 0) {
                    break;
                }
                drawRaisedBox(0, 0, position.w, position.h);
                int w = getGuiTextStyle()->getTextWidth(label);
                int x = (position.w - w) / 2;
                if (x < 0) { x = 0; }
                int y = (position.h - getGuiTextHeight()) / 2;
                if (y < 0) { y = 0; }
                drawGuiText(x, y, label);
                return true;
            }

            case ButtonPress: {
                break;
            }

            case ButtonRelease: {
                break;
            }

            case MotionNotify: {
                break;
            }
        }
        return propagateEventToParentWidget(event);
    }
}


