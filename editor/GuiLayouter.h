#ifndef GUILAYOUTER_H

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
            if (a.getMaxValue() == -1) {
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
            if (a.getMaxValue() != -1) {
                int di = ((ROUNDED_DIV(d * a.getMaxValue(), (maxValue - totalFlexValue))) / a.getIncrValue()) * a.getIncrValue();
                a.setBestValue(a.getBestValue() + di);
                used += di;
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
                    if (a.getMaxValue() == -1 && a.getMinValue() > 0) {
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
                    if (a.getMaxValue() == -1 && a.getIncrValue() > 1 && a.getMinValue() > 0) {
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
                    if (a.getMaxValue() == -1 && a.getIncrValue() == 1 && a.getMinValue() > 0) {
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
                    if (a.getMaxValue() == -1 && a.getMinValue() == 0) {
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
                    if (a.getMaxValue() == -1 && a.getIncrValue() > 1 && a.getMinValue() == 0) {
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
                    if (a.getMaxValue() == -1 && a.getIncrValue() == 1 && a.getMinValue() == 0) {
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
                if (a.getMaxValue() == -1) {
                    int d = actualValue - used;
                    a.setBestValue(a.getBestValue() + d);
                    used += d;
                }
            }
        }
    }
};

} // namespace LucED

#endif // GUILAYOUTER_H
