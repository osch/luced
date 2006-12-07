/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#ifndef GUILAYOUTER_H
#define GUILAYOUTER_H

#include <limits.h>

#include "debug.h"
#include "util.h"
#include "ObjectArray.h"
#include "GuiElement.h"
#include "Position.h"

namespace LucED {

class HorizontalAdapter
{
public:
    HorizontalAdapter(GuiElement::Measures &m)
        : m(m) {}
    int getMinValue()  { return m.minWidth;  }
    int getBestValue() { return m.bestWidth; }
    int getMaxValue()  { return m.maxWidth;  }
    int getIncrValue() { return m.incrWidth; }
    void setBestValue(int v) { m.bestWidth = v; }
    void setMaxValue(int v)  { m.maxWidth = v; }
private:
    GuiElement::Measures &m;
};

class VerticalAdapter
{
public:
    VerticalAdapter(GuiElement::Measures &m)
        : m(m) {}
    int getMinValue()  { return m.minHeight;  }
    int getBestValue() { return m.bestHeight; }
    int getMaxValue()  { return m.maxHeight;  }
    int getIncrValue() { return m.incrHeight; }
    void setBestValue(int v) { m.bestHeight = v; }
    void setMaxValue(int v)  { m.maxHeight = v; }
private:
    GuiElement::Measures &m;
};


template<class Adapter> class GuiLayouter
{
public:
    static void adjust(ObjectArray<GuiElement::Measures> &measures, const int actualValue)
    {
        int minValue  = 0;
        int bestValue = 0;
        int maxValue  = 0;
        int totalFlexValue = 0;
         
        for (int i = 0; i < measures.getLength(); ++i) {
            Adapter a(measures[i]);
            minValue  += a.getMinValue();
            bestValue += a.getBestValue();
            if (a.getMaxValue() == INT_MAX) {
                maxValue += a.getBestValue();
                totalFlexValue += a.getBestValue();
            } else {
                maxValue += a.getMaxValue();
            }
        }
        
        int targetValue = actualValue;
        
        if (targetValue < minValue) {
            targetValue = minValue;
            if (targetValue == 0) 
                targetValue = 1;
        } else if (targetValue > maxValue) {
            targetValue = maxValue;
        }

        if (targetValue < bestValue) {
            for (int i = 0; i < measures.getLength(); ++i) {
                Adapter a(measures[i]);
                a.setBestValue(a.getMinValue());
            }
            bestValue = minValue;
        }
        
        ASSERT(bestValue <= targetValue);
        
        // enlarge from bestValue to targetValue, ideal value is maxValue
        
        int d = targetValue - bestValue;
        int used = bestValue;
        int numberFlexWithoutContent =  0;
        int numberFlexWithContent = 0;
        for (int i = 0; i < measures.getLength(); ++i) {
            Adapter a(measures[i]);
            if (a.getMaxValue() != INT_MAX) {
                if (d > 0 && maxValue != totalFlexValue) {
                    int di = ((ROUNDED_DIV(d * a.getMaxValue(), (maxValue - totalFlexValue))) / a.getIncrValue()) * a.getIncrValue();
                    a.setBestValue(a.getBestValue() + di);
                    used += di;
                }
                if (a.getBestValue() > a.getMaxValue()) {
                    used -= (a.getBestValue() - a.getMaxValue());
                    a.setBestValue(a.getMaxValue());
                }
            } else {
                if (a.getMinValue() > 0) {
                    numberFlexWithContent += 1;
                } else {
                    numberFlexWithoutContent += 1;
                }
            }
        }
        
        if (numberFlexWithContent > 0)
        {
            if (used < actualValue)
            {
                int di = (actualValue - used) / numberFlexWithContent;
                for (int i = 0; used < actualValue && i < measures.getLength(); ++i) {
                    Adapter a(measures[i]);
                    if (a.getMaxValue() == INT_MAX && a.getMinValue() > 0) {
                        int mdi = (di / a.getIncrValue()) * a.getIncrValue();
                        a.setBestValue(a.getBestValue() + mdi);
                        used += mdi;
                    }
                }
            }
            if (used < actualValue)
            {
                for (int i = measures.getLength() - 1; used < actualValue && i >= 0; --i) {
                    Adapter a(measures[i]);
                    if (a.getMaxValue() == INT_MAX && a.getIncrValue() > 1 && a.getMinValue() > 0) {
                        if (a.getIncrValue() >= actualValue - used) {
                            int d = ((actualValue - used) / a.getIncrValue()) * a.getIncrValue();
                            a.setBestValue(a.getBestValue() + d);
                            used += d;
                        }
                    }
                }
            }
            if (used < actualValue)
            {
                for (int i = measures.getLength() - 1; used < actualValue && i >= 0; --i) {
                    Adapter a(measures[i]);
                    if (a.getMaxValue() == INT_MAX && a.getIncrValue() == 1 && a.getMinValue() > 0) {
                        int d = actualValue - used;
                        a.setBestValue(a.getBestValue() + d);
                        used += d;
                    }
                }
            }
        }
        if (numberFlexWithoutContent > 0)
        {
            if (used < actualValue)
            {
                int di = (actualValue - used) / numberFlexWithoutContent;
                for (int i = 0; used < actualValue && i < measures.getLength(); ++i) {
                    Adapter a(measures[i]);
                    if (a.getMaxValue() == INT_MAX && a.getMinValue() == 0) {
                        int mdi = (di / a.getIncrValue()) * a.getIncrValue();
                        a.setBestValue(a.getBestValue() + mdi);
                        used += mdi;
                    }
                }
            }
            if (used < actualValue)
            {
                for (int i = measures.getLength() - 1; used < actualValue && i >= 0; --i) {
                    Adapter a(measures[i]);
                    if (a.getMaxValue() == INT_MAX && a.getIncrValue() > 1 && a.getMinValue() == 0) {
                        if (a.getIncrValue() >= actualValue - used) {
                            int d = ((actualValue - used) / a.getIncrValue()) * a.getIncrValue();
                            a.setBestValue(a.getBestValue() + d);
                            used += d;
                        }
                    }
                }
            }
            if (used < actualValue)
            {
                for (int i = measures.getLength() - 1; used < actualValue && i >= 0; --i) {
                    Adapter a(measures[i]);
                    if (a.getMaxValue() == INT_MAX && a.getIncrValue() == 1 && a.getMinValue() == 0) {
                        int d = actualValue - used;
                        a.setBestValue(a.getBestValue() + d);
                        used += d;
                    }
                }
            }
        }
        if ((used < actualValue) && (numberFlexWithoutContent > 0 || numberFlexWithContent > 0))
        {
            for (int i = measures.getLength() - 1; used < actualValue && i >= 0; --i) {
                Adapter a(measures[i]);
                if (a.getMaxValue() == INT_MAX) {
                    int d = actualValue - used;
                    a.setBestValue(a.getBestValue() + d);
                    used += d;
                }
            }
        }
    }




//    static void adjust(ObjectArray<GuiElement::Measures> &measures, const int actualValue)
//    {
//    printf("GuiLayoutColumn::setPosition %d\n", actualValue);
//
//        int bestHeight = 0;
//        int minHeight = 0;
//        int maxHeight = 0;
//        int numberFlex = 0;
//
//        for (int i = 0; i < measures.getLength(); ++i)
//        {
//            Adapter a(measures[i]);
//
//            addimize(&minHeight,  a.getMinValue());
//            addimize(&bestHeight, a.getBestValue());
//            addimize(&maxHeight,  a.getMaxValue());
//
//    printf("GuiLayoutColumn::setPosition: bestHeight(%d)=%d\n", i, a.getBestValue());
//
//            if (a.getMaxValue() == INT_MAX) {
//                ++numberFlex;
//            }
//        }
//    printf("GuiLayoutColumn::setPosition: numberFlexA %d, bestHeight %d\n", numberFlex, bestHeight);
//        if (bestHeight != INT_MAX && bestHeight <= actualValue) {
//            minHeight = bestHeight;
//        } else {
//            numberFlex = 0;
//        }
//        int addNonFlexHeight = 0;
//        int flexHeight = 0;
//        if (minHeight < actualValue) {
//            if (numberFlex > 0) {
//                flexHeight = ROUNDED_DIV(actualValue - minHeight, numberFlex);
//            } else {
//                addNonFlexHeight = actualValue - minHeight;
//            }
//        }
//    printf("GuiLayoutColumn::setPosition: numberFlexB %d, flexHeight %d\n", numberFlex, flexHeight);
//
//        for (int i = 0; i < measures.getLength(); ++i)
//        {
//            Adapter a(measures[i]);
//
//            int h = 0;
//            if (a.getMaxValue() == INT_MAX) {
//                if (a.getBestValue() != INT_MAX) {
//                    h = a.getBestValue() + flexHeight;
//                } else {
//                    h = flexHeight;
//                }
//            } else {
//                if (bestHeight <= actualValue) {
//                    h = a.getBestValue();
//                } else {
//                    if (bestHeight > minHeight) {
//                        h = a.getMinValue() + ROUNDED_DIV(addNonFlexHeight * (a.getBestValue() - a.getMinValue()), 
//                                                          (bestHeight - minHeight));
//                    } else {
//                        h = a.getMinValue();
//                    }
//                }
//                if (h > a.getBestValue()) {
//                    h = a.getBestValue();
//                }
//            }
//    printf("GuiLayoutColumn::setPosition for Row %d ---> %d\n", i, h);
//            a.setBestValue(h);
//        }
//    }
//
//private:
//    static void addimize(int *a, int b) {
//        if (*a != INT_MAX) {
//            if (b == INT_MAX) {
//                *a = INT_MAX;
//            } else {
//                *a += b;
//            }
//        }
//    }

};

} // namespace LucED

#endif // GUILAYOUTER_H
