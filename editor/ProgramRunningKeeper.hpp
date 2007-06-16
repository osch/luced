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

#ifndef PROGRAM_RUNNING_KEEPER_HPP
#define PROGRAM_RUNNING_KEEPER_HPP

#include "NonCopyable.hpp"
#include "RunningComponent.hpp"
#include "EventDispatcher.hpp"

namespace LucED
{

class ProgramRunningKeeper : public NonCopyable
{
public:

    ProgramRunningKeeper()
    {}
    
    ~ProgramRunningKeeper() {
        doNotKeepProgramRunning();
    }
    
    void keepProgramRunning() {
        if (pseudoRunningComponent.isInvalid()) {
            pseudoRunningComponent = PseudoRunningComponent::create();
            EventDispatcher::getInstance()->registerRunningComponent(pseudoRunningComponent);
        }
    }
    
    void doNotKeepProgramRunning() {
        if (pseudoRunningComponent.isValid()) {
            EventDispatcher::getInstance()->deregisterRunningComponent(pseudoRunningComponent);
            pseudoRunningComponent.invalidate();
        }
    }

private:
    class PseudoRunningComponent : public RunningComponent
    {
    public:
        typedef LucED::OwningPtr<PseudoRunningComponent> Ptr;
        static Ptr create() {
            return Ptr(new PseudoRunningComponent());
        }
    private:
        PseudoRunningComponent() {}
    };
    
    PseudoRunningComponent::Ptr pseudoRunningComponent;
};

} // namespace LucED

#endif // PROGRAM_RUNNING_KEEPER_HPP
