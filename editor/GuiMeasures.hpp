/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#ifndef GUI_MEASURES_HPP
#define GUI_MEASURES_HPP

namespace LucED
{

class GuiMeasures
{
public:
    GuiMeasures() 
            : minWidth(0),  minHeight(0),
              bestWidth(0), bestHeight(0),
              maxWidth(0),  maxHeight(0),
              incrWidth(0), incrHeight(0)
    {}
    GuiMeasures(int minWidth, int minHeight, int bestWidth, int bestHeight, int maxWidth, int maxHeight) 
            : minWidth(minWidth),   minHeight(minHeight),
              bestWidth(bestWidth), bestHeight(bestHeight),
              maxWidth(maxWidth),   maxHeight(maxHeight),
              incrWidth(1),         incrHeight(1)
    {}
    GuiMeasures(int minWidth, int minHeight, int bestWidth, int bestHeight, int maxWidth, int maxHeight,
                int incrWidth,int incrHeight) 
            : minWidth(minWidth),   minHeight(minHeight),
              bestWidth(bestWidth), bestHeight(bestHeight),
              maxWidth(maxWidth),   maxHeight(maxHeight),
              incrWidth(incrWidth), incrHeight(incrHeight)
    {}
    void maximize(const GuiMeasures& rhs);
    
    int bestWidth;
    int bestHeight;
    int minWidth;
    int minHeight;
    int maxWidth;
    int maxHeight;
    int incrWidth;
    int incrHeight;
};



} // namespace LucED

#endif // GUI_MEASURES_HPP
