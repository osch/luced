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

#ifndef VIEW_LUA_INTERFACE_HPP
#define VIEW_LUA_INTERFACE_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "RawPtr.hpp"
#include "TextEditorWidget.hpp"
#include "LuaCFunctionResult.hpp"
#include "LuaCFunctionArguments.hpp"
#include "FindUtil.hpp"
#include "LuaException.hpp"

namespace LucED
{

class ViewLuaInterface : public HeapObject
{
public:
    typedef OwningPtr<ViewLuaInterface> Ptr;
    
    static Ptr create(RawPtr<TextEditorWidget> e) {
        return Ptr(new ViewLuaInterface(e));
    }

    LuaCFunctionResult getFileName             (const LuaCFunctionArguments& args);

    LuaCFunctionResult getCursorPosition       (const LuaCFunctionArguments& args);

    LuaCFunctionResult setCursorPosition       (const LuaCFunctionArguments& args);

    LuaCFunctionResult getCursorLine           (const LuaCFunctionArguments& args);    

    LuaCFunctionResult getCursorColumn         (const LuaCFunctionArguments& args);
    
    LuaCFunctionResult getColumn               (const LuaCFunctionArguments& args);
    
    LuaCFunctionResult getCharAtCursor         (const LuaCFunctionArguments& args);
    LuaCFunctionResult getByteAtCursor         (const LuaCFunctionArguments& args);
    
    LuaCFunctionResult getBytesAtCursor        (const LuaCFunctionArguments& args);

    LuaCFunctionResult getBytes                (const LuaCFunctionArguments& args);
    
    LuaCFunctionResult insertAtCursor          (const LuaCFunctionArguments& args);
    
    LuaCFunctionResult insert                  (const LuaCFunctionArguments& args);

    LuaCFunctionResult find                    (const LuaCFunctionArguments& args);
    
    LuaCFunctionResult findMatch               (const LuaCFunctionArguments& args);

    LuaCFunctionResult executeAction           (const LuaCFunctionArguments& args);

    LuaCFunctionResult hasPrimarySelection     (const LuaCFunctionArguments& args);
    LuaCFunctionResult hasPseudoSelection      (const LuaCFunctionArguments& args);
    LuaCFunctionResult getSelection            (const LuaCFunctionArguments& args);
    LuaCFunctionResult replaceSelection        (const LuaCFunctionArguments& args);

    LuaCFunctionResult releaseSelection        (const LuaCFunctionArguments& args);
    LuaCFunctionResult removeSelection         (const LuaCFunctionArguments& args);

    LuaCFunctionResult assureCursorVisible     (const LuaCFunctionArguments& args);
    LuaCFunctionResult setCurrentActionCategory(const LuaCFunctionArguments& args);

private:
    ViewLuaInterface(RawPtr<TextEditorWidget> e)
        : e(e),
          textData(e->getTextData()),
          findUtil(e->getTextData()),
          m(e->getTextData()->createNewMark())
    {}
    
    void parseAndSetFindUtilOptions(const LuaCFunctionArguments& args);

    RawPtr<TextEditorWidget> e;
    RawPtr<TextData>         textData;
    FindUtil                 findUtil;
    TextData::TextMark       m;
};

} // namespace LucED

#endif // VIEW_LUA_INTERFACE_HPP
