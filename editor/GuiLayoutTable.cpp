#include "GuiLayoutSpacer.h"
#include "GuiLayoutTable.h"
#include "util.h"

using namespace LucED;


GuiLayoutTable::GuiLayoutTable(int numberRows, int numberColumns)
        : numberRows(numberRows), numberColumns(numberColumns)
{
    elements.appendAmount(numberRows * numberColumns);
    columnMeasures.appendAmount(numberColumns);
    columnPositions.appendAmount(numberColumns);
    rowMeasures.appendAmount(numberRows);
    rowPositions.appendAmount(numberRows);
}

static void maximize(int *a, int b)
{
    if (*a != -1) {
        if (b == -1) {
            *a = -1;
        } else {
            util::maximize(a, b);
        }
    }
}

static void maximize(int *a, int min, int best, int max)
{
    if (*a != -1) {
        if (max == -1) {
            if (best == -1) {
                if (min == -1) {
                    *a = -1;
                } else {
                    util::maximize(a, min);
                }
            } else {
                util::maximize(a, best);
            }
        } else {
            util::maximize(a, max);
        }
    }
}

static void addimize(int *a, int b)
{
    if (*a != -1) {
        if (b == -1) {
            *a = -1;
        } else {
            *a += b;
        }
    }
}

static void ensureBetween(int *a, int min, int max)
{
    if (*a != -1) {
        if (*a < min) {
            *a = min;
        }
        if (max != -1) {
            if (*a > max) {
                *a = max;
            }
        }
    } else {
        if (max != -1) {
            *a = max;
        }
    }
}

GuiElement::Measures GuiLayoutTable::getDesiredMeasures()
{
    int minWidth = 0;
    int bestWidth = 0;
    int maxWidth = 0;

    int minHeight = 0;
    int bestHeight = 0;
    int maxHeight = 0;
    
    for (int r = 0; r < numberRows; ++r)
    {
        int rowMinWidth = 0;
        int rowBestWidth = 0;
        int rowMaxWidth = 0;

        int rowMinHeight = 0;
        int rowBestHeight = 0;
        int rowMaxHeight = 0;

        for (int c = 0; c < numberColumns; ++c)
        {
            if (getElement(r, c).isInvalid()) {
                setElement(r, c, GuiLayoutSpacer::create());
            }
            Measures m = getElement(r, c)->getDesiredMeasures();

            addimize(&rowMinWidth,  m.minWidth);
            addimize(&rowBestWidth, m.bestWidth);
            addimize(&rowMaxWidth,  m.maxWidth);

            maximize(&rowMinHeight,  m.minHeight);
            addimize(&rowBestHeight, m.bestHeight);
            maximize(&rowMaxHeight,  m.maxHeight);
        }
        if (numberColumns > 0 && rowBestHeight != -1) {
            rowBestHeight /= numberColumns;
        }
        ensureBetween(&rowBestHeight, rowMinHeight, rowMaxHeight);

        maximize(&minWidth,  rowMinWidth);
        addimize(&bestWidth, rowBestWidth);
        maximize(&maxWidth,  rowMaxWidth);
        
        addimize(&minHeight,  rowMinHeight);
        addimize(&bestHeight, rowBestHeight);
        addimize(&maxHeight,  rowMaxHeight);
    }
    if (numberRows > 0 && bestWidth != -1) {
        bestWidth /= numberRows;
    }
    ensureBetween(&bestWidth, minWidth, maxWidth);
    return Measures(minWidth, minHeight, bestWidth, bestHeight, maxWidth, maxHeight);
}

void GuiLayoutTable::setPosition(Position p)
{
    int minWidth = 0;
    int bestWidth = 0;
    int maxWidth = 0;

    int numberFlexColumns = 0;

    for (int c = 0; c < numberColumns; ++c)
    {
        int columnMinWidth = 0;
        int columnBestWidth = 0;
        int columnMaxWidth = 0;

        for (int r = 0; r < numberRows; ++r)
        {
            if (getElement(r, c).isInvalid()) {
                setElement(r, c, GuiLayoutSpacer::create());
            }
            Measures m = getElement(r, c)->getDesiredMeasures();

            maximize(&columnMinWidth,  m.minWidth);
            addimize(&columnBestWidth, m.bestWidth);
            maximize(&columnMaxWidth,  m.maxWidth);
        }
        if (numberRows > 0 && columnBestWidth != -1) {
            columnBestWidth /= numberRows;
        }
        ensureBetween(&columnBestWidth, columnMinWidth, columnMaxWidth);

        columnMeasures[c].minWidth  = columnMinWidth;
        columnMeasures[c].bestWidth = columnBestWidth;
        columnMeasures[c].maxWidth  = columnMaxWidth;

        addimize(&minWidth,  columnMinWidth);
        addimize(&bestWidth, columnBestWidth);
        addimize(&maxWidth,  columnMaxWidth);
        
        if (columnMaxWidth == -1) {
            ++numberFlexColumns;
        }
    }
    if (bestWidth != -1 && bestWidth <= p.w) {
        minWidth = bestWidth;
    } else {
        numberFlexColumns = 0;
    }
    int addNonFlexWidth = 0;
    int flexWidth = 0;
    int remainingFlexWidth = 0;
    if (minWidth < p.w) {
        if (numberFlexColumns > 0) {
            flexWidth = (p.w - minWidth) / numberFlexColumns;
            remainingFlexWidth = (p.w - minWidth) - flexWidth * numberFlexColumns;
        } else {
            addNonFlexWidth = p.w - minWidth;
        }
    }

    int x = p.x;
    for (int c = 0; c < numberColumns; ++c)
    {
        Measures& m = columnMeasures[c];
        
        int w = 0;
        if (m.maxWidth == -1) {
            if (m.bestWidth != -1) {
                w = m.bestWidth + flexWidth;
            } else {
                w = flexWidth;
            }
            if (c < remainingFlexWidth) {
                w += 1;
            }
        } else {
            if (bestWidth <= p.w) {
                w = m.bestWidth;
            } else {
                w = m.minWidth + ROUNDED_DIV(addNonFlexWidth * (m.bestWidth - m.minWidth), (bestWidth - minWidth));
            }
            if (w > m.bestWidth) {
                w = m.bestWidth;
            }
        }
        columnPositions[c].x = x;
        columnPositions[c].y = p.y;
        columnPositions[c].w = w;
        columnPositions[c].h = p.h;
        x += w;
    }
    
    
    int minHeight = 0;
    int bestHeight = 0;
    int maxHeight = 0;
    
    int numberFlexRows = 0;

    for (int r = 0; r < numberRows; ++r)
    {
        int rowMinHeight = 0;
        int rowBestHeight = 0;
        int rowMaxHeight = 0;

        for (int c = 0; c < numberColumns; ++c)
        {
            Measures m = getElement(r, c)->getDesiredMeasures();

            maximize(&rowMinHeight,  m.minHeight);
            addimize(&rowBestHeight, m.bestHeight);
            maximize(&rowMaxHeight,  m.maxHeight);
//            maximize(&rowMaxHeight,  m.minHeight, m.bestHeight, m.maxHeight);
        }
        if (numberColumns > 0 && rowBestHeight != -1) {
            rowBestHeight /= numberColumns;
        }
        ensureBetween(&rowBestHeight, rowMinHeight, rowMaxHeight);

        rowMeasures[r].minHeight  = rowMinHeight;
        rowMeasures[r].bestHeight = rowBestHeight;
        rowMeasures[r].maxHeight  = rowMaxHeight;
        addimize(&minHeight,  rowMinHeight);
        addimize(&bestHeight, rowBestHeight);
        addimize(&maxHeight,  rowMaxHeight);
        
        if (rowMaxHeight == -1) {
            ++numberFlexRows;
        }
    }
    if (bestHeight != -1 && bestHeight <= p.h) {
        minHeight = bestHeight;
    } else {
        numberFlexRows = 0;
    }

    int addNonFlexHeight = 0;
    int flexHeight = 0;
    int remainingFlexHeight = 0;
    if (minHeight < p.h) {
        if (numberFlexRows > 0) {
            flexHeight = (p.h - minHeight) / numberFlexRows;
            remainingFlexHeight = (p.h - minHeight) - flexHeight * numberFlexRows;
        } else {
            addNonFlexHeight = p.h - minHeight;
        }
    }

    int y = p.y;
    for (int r = 0; r < numberRows; ++r)
    {
        Measures& m = rowMeasures[r];
        
        int h = 0;
        if (m.maxHeight == -1) {
            if (m.bestHeight != -1) {
                h = m.bestHeight + flexHeight;
            } else {
                h = flexHeight;
            }
            if (r < remainingFlexHeight) {
                h += 1;
            }
        } else {
            if (bestHeight <= p.h) {
                h = m.bestHeight;
            } else {
                h = m.minHeight + ROUNDED_DIV(addNonFlexHeight * (m.bestHeight - m.minHeight), (bestHeight - minHeight));
            }
            if (h > m.bestHeight) {
                h = m.bestHeight;
            }
        }
        rowPositions[r].x = p.x;
        rowPositions[r].y = y;
        rowPositions[r].w = p.w;
        rowPositions[r].h = h;
        y += h;
    }
    
    
    for (int c = 0; c < numberColumns; ++c)
    {
        for (int r = 0; r < numberRows; ++r)
        {
            getElement(r, c)->setPosition(Position(
                    columnPositions[c].x, rowPositions[r].y, 
                    columnPositions[c].w, rowPositions[r].h));
        }
    }
}
