/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2007 Oliver Schmidt, oliver at luced dot de
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

#ifndef CONFIG_ERROR_HANDLER_HPP
#define CONFIG_ERROR_HANDLER_HPP

#include "HeapObject.hpp"
#include "HeapObjectArray.hpp"
#include "RunningComponent.hpp"
#include "EventDispatcher.hpp"
#include "OwningPtr.hpp"
#include "ConfigException.hpp"
#include "FileOpener.hpp"
#include "ActionId.hpp"
#include "Nullable.hpp"

namespace LucED
{

class ConfigErrorHandler : public RunningComponent
{
public:
    typedef LucED::OwningPtr<ConfigErrorHandler> OwningPtr;
    typedef LucED::WeakPtr  <ConfigErrorHandler> WeakPtr;

    static WeakPtr start(ConfigException::ErrorList::Ptr errorList,
                         FileOpener::ParameterList::Ptr  fileOpenerParameters = Null)
    {
        OwningPtr ptr(new ConfigErrorHandler(errorList, fileOpenerParameters, Null));
        EventDispatcher::getInstance()->registerRunningComponent(ptr);
        ptr->startMessageBox();
        return ptr;
    }
    static WeakPtr start(ConfigException::ErrorList::Ptr errorList,
                         Nullable<ActionId>              optionalActionId = Null)
    {
        OwningPtr ptr(new ConfigErrorHandler(errorList, Null, optionalActionId));
        EventDispatcher::getInstance()->registerRunningComponent(ptr);
        ptr->startMessageBox();
        return ptr;
    }
    static WeakPtr startWithCatchedException(Nullable<ActionId> optionalActionId = Null)
    {
        ConfigException::ErrorList::Ptr errorList = ConfigException::ErrorList::create();
        errorList->appendCatchedException();
        return ConfigErrorHandler::start(errorList, optionalActionId);
    }

private:
    ConfigErrorHandler(ConfigException::ErrorList::Ptr errorList,
                       FileOpener::ParameterList::Ptr  fileOpenerParameters,
                       Nullable<ActionId>              optionalActionId)
        : errorList(errorList),
          fileOpenerParameters(fileOpenerParameters),
          optionalActionId(optionalActionId)
    {}
    
    void startMessageBox();
    void handleOpenFilesButton();
    void handleAbortButton();
    
    ConfigException::ErrorList::Ptr errorList;
    FileOpener::ParameterList::Ptr  fileOpenerParameters;
    Nullable<ActionId>              optionalActionId;
};

} // namespace LucED

#endif // CONFIG_ERROR_HANDLER_HPP
