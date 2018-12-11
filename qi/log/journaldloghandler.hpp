#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_LOG_JOURNALDLOGHANDLER_HPP_
#define _QI_LOG_JOURNALDLOGHANDLER_HPP_

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <qi/log.hpp>

namespace qi
{
namespace log
{
    /**
     * \brief Return a log handler, which forwards logs to systemd's journal
     *
     * The following journal fields are added:
     * - MESSAGE, the log message
     * - QI_CATEGORY, the qiLogCategory
     * - PRIORITY, with the log level (mapped to syslog priority)
     * - QI=1
     * - CODE_FILE, CODE_LINE, CODE_FUNC
     *   (only when NO_QI_LOG_DETAILED_CONTEXT is not defined)
     * - the content of the QI_LOG_EXTRA_JOURNALD_FIELD environment variable.
     *
     * The returned handler is not thread-safe, which is fine: libqi does not
     * call its log handlers concurrently.
     */
    Handler makeJournaldLogHandler();
}; // !log
}; // !qi

#endif

