#ifndef DATE_TIME_C_TIME_HPP___
#define DATE_TIME_C_TIME_HPP___

/* Copyright (c) 2002,2003,2005 CrystalClear Software, Inc.
 * Use, modification and distribution is subject to the
 * Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)
 * Author: Jeff Garland, Bart Garst
 * $Date: 2012-09-22 09:04:10 -0700 (Sat, 22 Sep 2012) $
 */


/*! @file c_time.hpp
  Provide workarounds related to the ctime header
*/

#include <ctime>

//////////////////////////////////////////////////////////////////////////
// {{{ VIDEOJET patch: missing several time functions on Windows CE
#if defined(UNDER_CE)
	#include <windows.h>
#endif
// VIDEOJET patch }}}
//////////////////////////////////////////////////////////////////////////

#include <string> // to be able to convert from string literals to exceptions
#include <stdexcept>
#include <boost/throw_exception.hpp>
#include <boost/date_time/compiler_config.hpp>

//Work around libraries that don't put time_t and time in namespace std
#ifdef BOOST_NO_STDC_NAMESPACE
namespace std { using ::time_t; using ::time; using ::localtime;
                using ::tm;  using ::gmtime; }
#endif // BOOST_NO_STDC_NAMESPACE

//The following is used to support high precision time clocks
#ifdef BOOST_HAS_GETTIMEOFDAY
#include <sys/time.h>
#endif

#ifdef BOOST_HAS_FTIME
#include <time.h>
#endif

namespace boost {
namespace date_time {
  //! Provides a uniform interface to some 'ctime' functions
  /*! Provides a uniform interface to some ctime functions and
   * their '_r' counterparts. The '_r' functions require a pointer to a
   * user created std::tm struct whereas the regular functions use a
   * staticly created struct and return a pointer to that. These wrapper
   * functions require the user to create a std::tm struct and send in a
   * pointer to it. This struct may be used to store the resulting time.
   * The returned pointer may or may not point to this struct, however,
   * it will point to the result of the corresponding function.
   * All functions do proper checking of the C function results and throw
   * exceptions on error. Therefore the functions will never return NULL.
   */
  struct c_time {
    public:

//////////////////////////////////////////////////////////////////////////
// {{{ VIDEOJET patch: missing several time functions on Windows CE
#ifdef UNDER_CE
	//! requires a pointer to a user created std::tm struct 
	inline static std::tm* localtime(const std::time_t* t, std::tm* result)
	{ 
		/* convert... 
		0. time_t to int64_t to avoid overflows 
		1. epoch beginning 1970 to one beginning 1601 
		2. seconds to 100ns 
		3. int64_t to FILETIME */ 
		int64_t t64 = static_cast<int64_t>(*t); 
		t64 += 11644473600; 
		t64 *= 10000000; 
		FILETIME utc_time; 
		utc_time.dwLowDateTime = static_cast<DWORD>(t64 & 0xFFFFFFFF); 
		utc_time.dwHighDateTime= static_cast<DWORD>(t64 >> 32); 
		// convert to local time 
		FILETIME  local_time; 
		if(!FileTimeToLocalFileTime(&utc_time, &local_time)) 
			boost::throw_exception(std::runtime_error("could not convert calendar time to local time")); 
		// split into year, month, day etc 
		SYSTEMTIME st; 
		if(!FileTimeToSystemTime(&local_time, &st)) 
			boost::throw_exception(std::runtime_error("could not convert calendar time to local time")); 
		systime_to_tm(result, st); 
		/* I'm not sure if MS Windows CE actually supports things like DST, at 
		least I haven't found a way to retrieve the info. For that reason, use 
		-1 to mark the info as not available. */ 
		result->tm_isdst = -1; 
		return result; 
	} 
	//! requires a pointer to a user created std::tm struct 
	inline static std::tm* gmtime(const std::time_t* t, std::tm* result)
	{ 
		/* convert... 
		0. time_t to int64_t to avoid overflows 
		1. epoch beginning 1970 to one beginning 1601 
		2. seconds to 100ns 
		3. int64_t to FILETIME */ 
		int64_t t64 = static_cast<int64_t>(*t); 
		t64 += 11644473600; 
		t64 *= 10000000; 
		FILETIME utc_time; 
		utc_time.dwLowDateTime = static_cast<DWORD>(t64 & 0xFFFFFFFF); 
		utc_time.dwHighDateTime= static_cast<DWORD>(t64 >> 32); 
		// split into year, month, day etc 
		SYSTEMTIME st; 
		if(!FileTimeToSystemTime(&utc_time, &st)) 
			boost::throw_exception(std::runtime_error("could not convert calendar time to local time")); 
		systime_to_tm(result, st); 
		/* MSVC8 implementation always sets tm_isdst=0 in gmtime(), using a 
		German MS Windows XP, not sure if that is correct... */ 
		result->tm_isdst = 0; 
		return result; 
	}

	/* This wrapper for time() is to work around systems (i.e. MS Windows CE) 
	that lack a working time() function. */
	inline static std::time_t time(time_t* p)
	{ 
		SYSTEMTIME systime; 
		GetSystemTime(&systime); 
		FILETIME ft; 
		if(!SystemTimeToFileTime(&systime, &ft)) 
			boost::throw_exception(std::runtime_error("could not convert local time to file time")); 
		/* convert 
		1. FILETIME to int64_t 
		2. 100ns to seconds 
		3. epoch beginning 1601 to one beginning 1970 
		4. int64_t to time_t */ 
		int64_t t64 = (static_cast<int64_t>(ft.dwHighDateTime) << 32) + ft.dwLowDateTime; 
		t64 = (t64 + 5000000) / 10000000; 
		t64 -= 11644473600; 
		std::time_t res = static_cast<std::time_t>(t64); 
		// make sure the static cast didn't truncate the result 
		if(res != t64) 
			boost::throw_exception(std::runtime_error("could not convert t64 to time_t")); 
		if(p) 
			*p = res; 
		return res; 
	} 

  private:
	/* utility function to convert a SYSTIME to a std::tm 
	Most fields are equal or similar, but the day of the year requires some 
	computations. */ 
	inline static void systime_to_tm(std::tm* result, SYSTEMTIME const& st) 
	{
		// assign the common values 
		result->tm_sec = st.wSecond; 
		result->tm_min = st.wMinute; 
		result->tm_hour = st.wHour; 
		result->tm_mday = st.wDay; 
		result->tm_mon = st.wMonth - 1; 
		result->tm_year = st.wYear - 1900; 
		result->tm_wday = st.wDayOfWeek; 
		// compute day of year 
		bool leapyear; 
		if ((st.wYear % 1000) == 0) 
			leapyear = true; 
		else if((st.wYear % 100) == 0) 
			leapyear = false; 
		else if((st.wYear % 4) == 0) 
			leapyear = true; 
		else
			leapyear = false; 
		result->tm_yday = result->tm_mday - 1; 
		if(leapyear) 
		{
			int const dpm[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; 
			for(int i=0; i!=result->tm_mon; ++i) 
				result->tm_yday += dpm[i]; 
		}
		else
		{
			int const dpm[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; 
			for(int i=0; i!=result->tm_mon; ++i) 
				result->tm_yday += dpm[i]; 
		}
	}
#else
// VIDEOJET patch }}}
//////////////////////////////////////////////////////////////////////////

#if defined(BOOST_DATE_TIME_HAS_REENTRANT_STD_FUNCTIONS)
      //! requires a pointer to a user created std::tm struct
      inline
      static std::tm* localtime(const std::time_t* t, std::tm* result)
      {
        // localtime_r() not in namespace std???
 	#if defined(__VMS) && __INITIAL_POINTER_SIZE == 64
 	std::tm tmp;
 	if(!localtime_r(t,&tmp))
            result = 0;
	else
            *result = tmp;	
 	#else
        result = localtime_r(t, result);
	#endif
        if (!result)
          boost::throw_exception(std::runtime_error("could not convert calendar time to local time"));
        return result;
      }
      //! requires a pointer to a user created std::tm struct
      inline
      static std::tm* gmtime(const std::time_t* t, std::tm* result)
      {
        // gmtime_r() not in namespace std???
 	#if defined(__VMS) && __INITIAL_POINTER_SIZE == 64
 	std::tm tmp;
 	if(!gmtime_r(t,&tmp))
          result = 0;
        else
          *result = tmp;	
	#else
        result = gmtime_r(t, result);
	#endif
        if (!result)
          boost::throw_exception(std::runtime_error("could not convert calendar time to UTC time"));
        return result;
      }
#else // BOOST_HAS_THREADS

#if (defined(_MSC_VER) && (_MSC_VER >= 1400))
#pragma warning(push) // preserve warning settings
#pragma warning(disable : 4996) // disable depricated localtime/gmtime warning on vc8
#endif // _MSC_VER >= 1400
      //! requires a pointer to a user created std::tm struct
      inline
      static std::tm* localtime(const std::time_t* t, std::tm* result)
      {
        result = std::localtime(t);
        if (!result)
          boost::throw_exception(std::runtime_error("could not convert calendar time to local time"));
        return result;
      }
      //! requires a pointer to a user created std::tm struct
      inline
      static std::tm* gmtime(const std::time_t* t, std::tm* result)
      {
        result = std::gmtime(t);
        if (!result)
          boost::throw_exception(std::runtime_error("could not convert calendar time to UTC time"));
        return result;
      }
#if (defined(_MSC_VER) && (_MSC_VER >= 1400))
#pragma warning(pop) // restore warnings to previous state
#endif // _MSC_VER >= 1400

//////////////////////////////////////////////////////////////////////////
// {{{ VIDEOJET patch: missing several time functions on Windows CE

#endif // UNDER_CE

// VIDEOJET patch }}}
//////////////////////////////////////////////////////////////////////////

#endif // BOOST_HAS_THREADS
  };
}} // namespaces

#endif // DATE_TIME_C_TIME_HPP___
