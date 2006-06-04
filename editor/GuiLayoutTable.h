#ifndef GUILAYOUTTABLE_H
#define GUILAYOUTTABLE_H

#include "GuiElement.h"
#include "ObjectArray.h"
#include "debug.h"
#include "OwningPtr.h"

namespace LucED {

class GuiLayoutTable : public GuiElement
{
public:
    typedef OwningPtr<GuiLayoutTable> Ptr;

    static Ptr create(int numberRows, int numberColumns) {
        return Ptr(new GuiLayoutTable(numberRows, numberColumns));
    }

    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position p);

    GuiElement::Ptr getElement(int row, int column) {
        ASSERT(0 <= row && row < numberRows);
        ASSERT(0 <= column && column < numberColumns);
        return elements[row * numberColumns + column];
    }

    void setElement(int row, int column, GuiElement::Ptr element) {
        ASSERT(0 <= row && row < numberRows);
        ASSERT(0 <= column && column < numberColumns);
        elements[row * numberColumns + column] = element;
    }

private:
    GuiLayoutTable(int numberRows, int numberColumns);
    
    int numberRows;
    int numberColumns;
    ObjectArray<GuiElement::Ptr> elements;
    ObjectArray<Measures> columnMeasures;
    ObjectArray<Position> columnPositions;
    ObjectArray<Measures> rowMeasures;
    ObjectArray<Position> rowPositions;
};

} // namespace LucED

#endif // GUILAYOUTTABLE_H
