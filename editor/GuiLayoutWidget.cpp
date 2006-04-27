#include "GuiLayoutWidget.h"
#include "GuiRoot.h"

using namespace LucED;

GuiLayoutWidget::GuiLayoutWidget(GuiWidget* parent, const Measures& m)
      : GuiWidget(parent, 0, 0, 1, 1, 0),
        position(0, 0, 1, 1),
        measures(m)
{
    setBackgroundColor(getGuiRoot()->getGuiColor01());
    this->show();
}

GuiElement::Measures GuiLayoutWidget::getDesiredMeasures()
{
    return measures;
}


void GuiLayoutWidget::setPosition(Position newPosition)
{
    if (position != newPosition) {
        GuiWidget::setPosition(newPosition);
        this->position = newPosition;
    }
}

