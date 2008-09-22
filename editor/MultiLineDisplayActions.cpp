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

#include <iostream>

#include <X11/keysym.h>
#include <X11/Xatom.h>

#include "Clipboard.hpp"
#include "FindUtil.hpp"
#include "KeyModifier.hpp"
#include "FileOpener.hpp"
#include "Regex.hpp"
#include "MultiLineDisplayActions.hpp"

using namespace LucED;


void MultiLineDisplayActions::scrollDown()
{
    e->scrollDown();
}


void MultiLineDisplayActions::scrollUp()
{
    e->scrollUp();
}

void MultiLineDisplayActions::scrollPageUp()
{
    e->scrollPageUp();
}


void MultiLineDisplayActions::scrollPageDown()
{
    e->scrollPageDown();
}
