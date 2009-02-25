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

#ifndef RAW_PTR_GUARD_HOLDER_HPP
#define RAW_PTR_GUARD_HOLDER_HPP

#include <stddef.h>
#include <new>

#include "debug.hpp"

namespace LucED
{

class RawPtrGuard;
template<class T> class WeakPtr;

class RawPtrGuardHolder
{
public:
    class RawPtrGuardAccess
    {
        template<class T
                >
        friend class RawPtr;

        template<class T
                >
        friend class ObjectArray;
        
    #ifdef DEBUG
        static WeakPtr<RawPtrGuard> getRawPtrGuard(const RawPtrGuardHolder* holder);

        static void invalidateRawPtrGuard(const RawPtrGuardHolder* holder);

        static void invalidateRawPtrGuard(const void* holder)
        {}
    #endif
    };

protected:
    RawPtrGuardHolder();
    
#ifdef DEBUG
    RawPtrGuardHolder(const RawPtrGuardHolder& rhs);
    
    RawPtrGuardHolder& operator=(const RawPtrGuardHolder& rhs);
    
    ~RawPtrGuardHolder();
#endif

private:
#ifdef DEBUG
    static RawPtrGuard* createRawPtrGuard();

    void invalidateRawPtrGuard() const;
    
    mutable RawPtrGuard* rawPtrGuard;
#endif
};

} // namespace LucED


#include "HeapObject.hpp"
#include "WeakPtr.hpp"
#include "OwningPtr.hpp"
#include "RawPtrGuard.hpp"
#include "ValueObject.hpp"

namespace LucED
{

#ifdef DEBUG
inline WeakPtr<RawPtrGuard> RawPtrGuardHolder::RawPtrGuardAccess::getRawPtrGuard
(
    const RawPtrGuardHolder* holder
) {
    if (holder != NULL) {
        return WeakPtr<RawPtrGuard>(holder->rawPtrGuard);
    } else {
        return Null;
    }
}
#endif

#ifdef DEBUG
inline RawPtrGuard* RawPtrGuardHolder::createRawPtrGuard()
{
    RawPtrGuard*  rslt = new RawPtrGuard();
    HeapObjectRefManipulator::obtainInitialOwnership(rslt);
    return rslt;
}
#endif


#ifdef DEBUG
inline void RawPtrGuardHolder::RawPtrGuardAccess::invalidateRawPtrGuard
(
    const RawPtrGuardHolder* holder
) {
    holder->invalidateRawPtrGuard();
    holder->rawPtrGuard = createRawPtrGuard();
}
#endif

#ifdef DEBUG
inline void RawPtrGuardHolder::invalidateRawPtrGuard() const
{
    HeapObjectRefManipulator::decRefCounter(rawPtrGuard);
    rawPtrGuard = NULL;
}
#endif

inline RawPtrGuardHolder::RawPtrGuardHolder()
#ifdef DEBUG
    : rawPtrGuard(createRawPtrGuard())
#endif
{}

#ifdef DEBUG
inline RawPtrGuardHolder::~RawPtrGuardHolder()
{
    invalidateRawPtrGuard();
}
#endif

#ifdef DEBUG
inline RawPtrGuardHolder::RawPtrGuardHolder(const RawPtrGuardHolder& rhs)
    : rawPtrGuard(createRawPtrGuard())
{}
#endif

#ifdef DEBUG
inline RawPtrGuardHolder& RawPtrGuardHolder::operator=(const RawPtrGuardHolder& rhs)
{
    return *this;    
}
#endif

} // namespace LucED

#endif // RAW_PTR_GUARD_HOLDER_HPP
