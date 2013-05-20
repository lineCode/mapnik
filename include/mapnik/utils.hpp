/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#ifndef MAPNIK_UTILS_HPP
#define MAPNIK_UTILS_HPP

// boost
#ifdef MAPNIK_THREADSAFE
#include <boost/thread/mutex.hpp>
#endif

// stl
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <limits>
#include <ctime>
#include <algorithm>
#include <cmath>

namespace mapnik
{

#ifdef MAPNIK_THREADSAFE
using boost::mutex;
#endif

template <typename T>
class CreateUsingNew
{
public:
    static T* create()
    {
        return new T;
    }
    static void destroy(T* obj)
    {
        delete obj;
    }
};

template <typename T>
class CreateStatic
{
private:
    union MaxAlign
    {
        char t_[sizeof(T)];
        short int shortInt_;
        int int_;
        long int longInt_;
        float float_;
        double double_;
        long double longDouble_;
        struct Test;
        int Test::* pMember_;
        int (Test::*pMemberFn_)(int);
    };

public:

    static T* create()
    {
        static MaxAlign staticMemory;
        return new(&staticMemory) T;
    }

    static void destroy(volatile T* obj)
    {
        obj->~T();
    }
};

#ifdef __GNUC__
    template <typename T,
              template <typename U> class CreatePolicy=CreateStatic> class MAPNIK_DECL singleton
    {
#else
    template <typename T,
              template <typename U> class CreatePolicy=CreateStatic> class singleton
    {
#endif
        friend class CreatePolicy<T>;
        static T* pInstance_;
        static bool destroyed_;
        singleton(const singleton &rhs);
        singleton& operator=(const singleton&);

        static void onDeadReference()
        {
            throw std::runtime_error("dead reference!");
        }

        static void DestroySingleton()
        {
            CreatePolicy<T>::destroy(pInstance_);
            pInstance_ = 0;
            destroyed_ = true;
        }

    protected:
#ifdef MAPNIK_THREADSAFE
        static mutex mutex_;
#endif
        singleton() {}
    public:
        static T& instance()
        {
            if (! pInstance_)
            {
#ifdef MAPNIK_THREADSAFE
                mutex::scoped_lock lock(mutex_);
#endif
                if (! pInstance_)
                {
                    if (destroyed_)
                    {
                        destroyed_ = false;
                        onDeadReference();
                    }
                    else
                    {
                        pInstance_ = CreatePolicy<T>::create();

                        // register destruction
                        std::atexit(&DestroySingleton);
                    }
                }
            }
            return *pInstance_;
        }
    };
#ifdef MAPNIK_THREADSAFE
    template <typename T,
              template <typename U> class CreatePolicy> mutex singleton<T,CreatePolicy>::mutex_;
#endif

    template <typename T,
              template <typename U> class CreatePolicy> T* singleton<T,CreatePolicy>::pInstance_=0;
    template <typename T,
              template <typename U> class CreatePolicy> bool singleton<T,CreatePolicy>::destroyed_=false;


#ifdef _WINDOWS

#include <windows.h>

    // UTF8 <--> UTF16 conversion routines

    std::string utf16_to_utf8(std::wstring const& wstr)
    {
        std::string str;
        int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, 0, 0, 0, 0);
        if(size > 0)
        {
            std::vector<char> buffer(size);
            WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &buffer[0], size, 0, 0);
            str.assign(buffer.begin(), buffer.end() - 1);
        }
        return str;
    }

    std::wstring utf8_to_utf16 (std::string const& str)
    {
        std::wstring wstr;
        int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, 0, 0);
        if (size > 0)
        {
            std::vector<wchar_t> buffer(size);
            MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &buffer[0], size);
            wstr.assign(buffer.begin(), buffer.end() - 1);
        }
        return wstr;
    }

#endif  // _WINDOWS

}

#endif // MAPNIK_UTILS_HPP
