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

#ifndef MESSAGE_BOX_PARAMETER_HPP
#define MESSAGE_BOX_PARAMETER_HPP

#include "String.hpp"
#include "Callback.hpp"

namespace LucED
{

class MessageBoxParameter
{
public:
    MessageBoxParameter& setTitle(String title) {
        this->title = title;
        return *this;
    }
    MessageBoxParameter& setMessage(String message) {
        this->message = message;
        return *this;
    }
    MessageBoxParameter& setDefaultButton(String buttonLabel, Callback<>::Ptr callback) {
        defaultButtonLabel = buttonLabel;
        defaultButtonCallback = callback;
        return *this;
    }
    MessageBoxParameter& setAlternativeButton(String buttonLabel, Callback<>::Ptr callback) {
        alternativeButtonLabel = buttonLabel;
        alternativeButtonCallback = callback;
        return *this;
    }
    MessageBoxParameter& setCancelButton(String buttonLabel) {
        cancelButtonLabel = buttonLabel;
        return *this;
    }
    
    MessageBoxParameter& setCancelButton(String buttonLabel, Callback<>::Ptr callback) {
        cancelButtonLabel = buttonLabel;
        cancelButtonCallback = callback;
        return *this;
    }
    
    MessageBoxParameter& setCloseCallback(Callback<>::Ptr callback) {
        closeCallback= callback;
        return *this;
    }
    
private:
    friend class MessageBox;

    String title;
    String message;

    String defaultButtonLabel;
    Callback<>::Ptr   defaultButtonCallback;

    String alternativeButtonLabel;
    Callback<>::Ptr   alternativeButtonCallback;

    String cancelButtonLabel;
    Callback<>::Ptr   cancelButtonCallback;
    
    Callback<>::Ptr   closeCallback;
};

} // namespace LucED

#endif // MESSAGE_BOX_PARAMETER_HPP
