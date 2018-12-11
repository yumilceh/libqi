/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/os.hpp>

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#ifdef __APPLE__
# define MY_PTRACE_ATTACH PT_ATTACHEXC
# define MY_PTRACE_CONT   PT_CONTINUE
# define MY_PTRACE_DETACH PT_DETACH
#else
# define MY_PTRACE_ATTACH PTRACE_ATTACH
# define MY_PTRACE_CONT   PTRACE_CONT
# define MY_PTRACE_DETACH PTRACE_DETACH
#endif

namespace qi {
  namespace os {

    int checkdbg()
    {
      int pid = fork();
      int status;
      int res;

      if (pid == -1) {
        perror("fork");
        return -1;
      }

      if (pid == 0) {
        int ppid = getppid();

        /* Child */
        if (ptrace(MY_PTRACE_ATTACH, ppid, NULL, 0) == 0)
        {
          /* Wait for the parent to stop and continue it */
          ::waitpid(ppid, NULL, 0);
          ptrace(MY_PTRACE_CONT, 0, NULL, 0);

          /* Detach */
          ptrace(MY_PTRACE_DETACH, getppid(), NULL, 0);

          /* We were the tracers, so gdb is not present */
          res = 0;
        }
        else
        {
          /* Trace failed so gdb is present */
          res = 1;
        }
        _exit(res);
      }
      else
      {
        ::waitpid(pid, &status, 0);
        res = WEXITSTATUS(status);
      }
      return res;
    }

  }
}
