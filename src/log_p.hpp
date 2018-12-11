#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _SRC_LOG_P_HPP_
#define _SRC_LOG_P_HPP_

#include <qi/os.hpp>
#include <qi/log.hpp>

#define CATSIZEMAX 16

namespace qi
{
  namespace detail
  {
    // export so we can test it
    std::string logline(LogContext                         context,
                        const qi::Clock::time_point        date,
                        const qi::SystemClock::time_point  systemDate,
                        const char                        *category,
                        const char                        *msg,
                        const char                        *file,
                        const char                        *fct,
                        const int                          line,
                        const qi::LogLevel                 verb
                        );

    std::string logline(LogContext             context,
                        const qi::os::timeval  systemDate,
                        const char            *category,
                        const char            *msg,
                        const char            *file,
                        const char            *fct,
                        const int              line,
                        const qi::LogLevel     verb
                        );

    std::string csvline(const qi::Clock::time_point date,
                        const qi::SystemClock::time_point systemDate,
                        const char            *category,
                        const char            *msg,
                        const char            *file,
                        const char            *fct,
                        const int              line,
                        const qi::LogLevel     verb);
    std::string csvheader();


    const std::string dateToString(const qi::os::timeval date);
    inline const std::string dateToString(const qi::Clock::time_point date)
    {
      return dateToString(qi::os::timeval(date.time_since_epoch()));
    }
    inline const std::string dateToString(const qi::SystemClock::time_point date)
    {
      return dateToString(qi::os::timeval(date.time_since_epoch()));
    }
    const std::string tidToString();

    std::size_t rtrim(const char *msg);
  }
} // namespace qi::detail

#endif  // _SRC_LOG_P_HPP_
