/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
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

#ifndef GUI_LAYOUTER_HPP
#define GUI_LAYOUTER_HPP

#include <limits.h>

#include "debug.hpp"
#include "util.hpp"
#include "ObjectArray.hpp"
#include "GuiElement.hpp"
#include "Position.hpp"

namespace LucED
{

class HorizontalAdapter
{
public:
    HorizontalAdapter(GuiElement::Measures& m)
        : m(m) {}

    int getMinValue()  { return m.minWidth;  }
    int getBestValue() { return m.bestWidth; }
    int getMaxValue()  { return m.maxWidth;  }
    int getIncrValue() { return m.incrWidth; }

    void setMinValue(int v)  { m.minWidth  = v; }
    void setBestValue(int v) { m.bestWidth = v; }
    void setMaxValue(int v)  { m.maxWidth  = v; }
    void setIncrValue(int v) { m.incrWidth = v; }

    int getMinCoValue()  { return m.minHeight;  }
    int getBestCoValue() { return m.bestHeight; }
    int getMaxCoValue()  { return m.maxHeight;  }
    int getIncrCoValue() { return m.incrHeight; }

    void setMinCoValue(int v)  { m.minHeight  = v; }
    void setBestCoValue(int v) { m.bestHeight  = v; }
    void setMaxCoValue(int v)  { m.maxHeight  = v; }
    void setIncrCoValue(int v) { m.incrHeight  = v; }

private:
    GuiElement::Measures& m;
};

class VerticalAdapter
{
public:
    VerticalAdapter(GuiElement::Measures& m)
        : m(m) {}

    int getMinValue()  { return m.minHeight;  }
    int getBestValue() { return m.bestHeight; }
    int getMaxValue()  { return m.maxHeight;  }
    int getIncrValue() { return m.incrHeight; }

    void setMinValue(int v)  { m.minHeight  = v; }
    void setBestValue(int v) { m.bestHeight = v; }
    void setMaxValue(int v)  { m.maxHeight  = v; }
    void setIncrValue(int v) { m.incrHeight = v; }

    int getMinCoValue()  { return m.minWidth;  }
    int getBestCoValue() { return m.bestWidth; }
    int getMaxCoValue()  { return m.maxWidth;  }
    int getIncrCoValue() { return m.incrWidth; }

    void setMinCoValue(int v)  { m.minWidth  = v; }
    void setBestCoValue(int v) { m.bestWidth  = v; }
    void setMaxCoValue(int v)  { m.maxWidth  = v; }
    void setIncrCoValue(int v) { m.incrWidth  = v; }

private:
    GuiElement::Measures& m;
};


template<class Adapter> class GuiLayouter
{
public:

    static GuiElement::Measures getDesiredMeasures(ObjectArray<GuiElement::Ptr>& elements)
    {
        int bestCoValue = 0;
        int bestValue   = 0;
        int minCoValue  = 0;
        int minValue    = 0;
        int maxCoValue  = INT_MAX;
        int maxValue    = 0;
        int incrCoValue = 1;

        int minVisibleValue = 0;
        int bestVisibleValue = 0;

        int incrValue   = 1;

        int numberOfRasteredValues = 0;
        
        for (int i = 0; i < elements.getLength(); ++i)
        {
            GuiElement::Measures m = elements[i]->getDesiredMeasures();
            Adapter a(m);

            if (elements[i]->isVisible())
            {
                addimize(&minVisibleValue,   a.getMinValue());
                addimize(&bestVisibleValue,  a.getBestValue());
                
                addimize(&maxValue,  a.getMaxValue());

                minimize(&maxCoValue,        a.getMaxCoValue());
//            }
            addimize(&bestValue, a.getBestValue());
            
            maximize(&bestCoValue, a.getBestCoValue());
            
    
            maximize(&incrCoValue,  a.getIncrCoValue());

            if (a.getIncrValue() > 1) {
                ++numberOfRasteredValues;
                if (numberOfRasteredValues == 1) {
                    incrValue = a.getIncrValue();
                }
            }
            
            } // test
            maximize(&minCoValue,  a.getMinCoValue());
            addimize(&minValue,  a.getMinValue());
        }
        if (numberOfRasteredValues > 1) {
            incrValue = 1;
        }
#if 0
        if (incrValue > 1 && (bestVisibleValue != bestValue)) {
            ASSERT(bestVisibleValue < bestValue);
            bestValue = bestVisibleValue + ROUNDED_DIV(bestValue - bestVisibleValue, incrValue) * incrValue;
        }
#endif        
        if (bestCoValue < minCoValue) {
            bestCoValue = minCoValue;
        }
        if (maxCoValue < bestCoValue) {
            maxCoValue = bestCoValue;
        }
        if (bestValue < minValue) {
            bestValue = minValue;
        }
        if (maxValue < bestValue) {
            maxValue = bestValue;
        }
        minCoValue  = bestCoValue  - ((bestCoValue  - minCoValue) / incrCoValue)  * incrCoValue;
        minValue = bestValue - ((bestValue - minValue)/ incrValue) * incrValue;
        

        GuiElement::Measures rslt;
        Adapter a(rslt);

        a.setMinValue (minValue);
        a.setBestValue(bestValue);
        a.setMaxValue (maxValue);
        a.setIncrValue(incrValue);

        a.setMinCoValue (minCoValue);
        a.setBestCoValue(bestCoValue);
        a.setMaxCoValue (maxCoValue);
        a.setIncrCoValue(incrCoValue);

        return rslt;
    }


    static void adjust(ObjectArray<GuiElement::Measures>& measures, const int actualValue)
    {
        int bestValue = 0;
        int minValue = 0;
        int maxValue = 0;

        int maxValueWithoutFlex = 0;
        int numberNonFlex = 0;
        int numberFlex = 0;
        
        for (int i = 0; i < measures.getLength(); ++i)
        {
            Adapter a(measures[i]);

            addimize(&minValue,  a.getMinValue());
            addimize(&bestValue, a.getBestValue());
            addimize(&maxValue,  a.getMaxValue());

            if (a.getMaxValue() == INT_MAX) {
                ++numberFlex;
                addimize(&maxValueWithoutFlex, a.getBestValue());
            } else {
                ++numberNonFlex;
                addimize(&maxValueWithoutFlex, a.getMaxValue());
            }
        }
        
        int targetMinValue;
        bool isBest;
        
        if (bestValue <= actualValue) {
            targetMinValue = bestValue;
            isBest = true;
        } else {
            targetMinValue = minValue;
            isBest = false;
        }
        
        int targetValue;
        
        if (targetMinValue < actualValue) {
            targetValue = actualValue;
        } else {
            targetValue = targetMinValue;
        }
        if (targetValue > maxValue) {
            targetValue = maxValue;
        }

        ASSERT(targetMinValue <= targetValue);

        int totalRestValue = targetValue - targetMinValue;
        
        ASSERT(totalRestValue >= 0);
        
        int totalRealValue = 0;
        
        if (isBest)
        {
            int totalFlexRestValue;
            int totalNonFlexRestValue;
                        
            if (maxValueWithoutFlex < targetValue) {
                totalFlexRestValue    = targetValue - maxValueWithoutFlex;
                totalNonFlexRestValue = maxValueWithoutFlex - targetMinValue;
            } else {
                totalFlexRestValue    = 0;
                totalNonFlexRestValue = totalRestValue;
            }
            
            ASSERT(totalFlexRestValue + totalNonFlexRestValue == totalRestValue);
            ASSERT(totalFlexRestValue    >= 0);
            ASSERT(totalNonFlexRestValue >= 0);
            
            int singleFlexRestValue;
            
            int remainingTotalNonFlexRestValue = totalNonFlexRestValue;
            
            if (numberFlex > 0) {
                singleFlexRestValue = ROUNDED_DIV(totalFlexRestValue, numberFlex);
            } else {
                singleFlexRestValue = 0;
            }
            
            for (int i = 0; i < measures.getLength(); ++i)
            {
                Adapter a(measures[i]);
    
                int v = a.getBestValue();
                
                if (a.getMaxValue() == INT_MAX) {
                    v += singleFlexRestValue;
                }
                else if (maxValueWithoutFlex - bestValue > 0)
                {
                    int add =  ROUNDED_DIV((a.getMaxValue() - a.getBestValue()) * totalNonFlexRestValue, maxValueWithoutFlex - bestValue);

                    if (add <= remainingTotalNonFlexRestValue)
                    {
                        v += add;
                        remainingTotalNonFlexRestValue -= add;
    
                        if (a.getIncrValue() > 1)
                        {
                            int rasteredV = a.getBestValue() + ROUNDED_UP_DIV(v - a.getBestValue(), a.getIncrValue()) * a.getIncrValue();
                            ASSERT(rasteredV >= v);

                            int diff = rasteredV - v;
                            
                            if (diff <= remainingTotalNonFlexRestValue) {
                                v = rasteredV;
                                totalNonFlexRestValue          -= diff;
                                remainingTotalNonFlexRestValue -= diff;
                            }
                        }
                    }
                }
                a.setBestValue(v);
                totalRealValue += v;
            }
        }
        else
        {
            ASSERT(!isBest);
            
            int bestRestValue = bestValue - minValue;
            
            ASSERT(bestRestValue >= 0);
            
            for (int i = 0; i < measures.getLength(); ++i)
            {
                Adapter a(measures[i]);
        
                int v = a.getMinValue();

                if (bestRestValue > 0) {
                    v += ROUNDED_DIV((a.getBestValue() - a.getMinValue()) * totalRestValue, bestRestValue);
                }
                
                a.setBestValue(v);
                totalRealValue += v;
            }
        }
#if 0
        while (totalRealValue < targetValue)
        {
            // handle rounding errors
            
            bool wasSomethingIncreased = false;
            
            for (int i = measures.getLength() - 1; i >= 0  && totalRealValue < targetValue; --i)
            {
                Adapter a(measures[i]);
                
                int v = a.getBestValue();
                
                if (v < a.getMaxValue())
                {
                    a.setBestValue(v + 1);
                    totalRealValue += 1;
                    wasSomethingIncreased = true;
                }
            }
            if (!wasSomethingIncreased) {
                break;
            }
        }
#endif
    }

private:

    static void maximize(int* a, int b) {
        if (*a != INT_MAX) {
            if (b == INT_MAX) {
                *a = INT_MAX;
            } else {
                util::maximize(a, b);
            }
        }
    }

    static void minimize(int* a, int b) {
        util::minimize(a, b);
    }

    static void addimize(int* a, int b) {
        if (*a != INT_MAX) {
            if (b == INT_MAX) {
                *a = INT_MAX;
            } else {
                *a += b;
            }
        }
    }

};

} // namespace LucED

#endif // GUI_LAYOUTER_HPP
