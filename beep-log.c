/** \file beep-log.c
 * \brief implement logging and message output
 * \author Copyright (C) 2018-2019 Hans Ulrich Niedermann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *
 * \defgroup beep_log Logging and message output
 *
 * @{
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "beep-log.h"


#define LOG_MODULE "log"


/* documented in header file */
int log_level = 0;


/* documented in header file */
const char *progname = "beep-log";


/**
 * beep-log internal function to actually do the printing of a message
 */

static
void log_internal_vf(const char *const module, const char *levelstr,
                     const char *const format, va_list args)
    __attribute__(( nonnull(2) ));


static
void log_internal_vf(const char *const module, const char *levelstr,
                     const char *const format, va_list args)
{
    va_list copied_args;
    va_copy(copied_args, args);

    if (module) {
        fprintf(stdout, "%s: %s: %s: ", progname, levelstr, module);
    } else {
        fprintf(stdout, "%s: %s: ", progname, levelstr);
    }
    vfprintf(stdout, format, copied_args);
    fputc('\n', stdout);

    va_end(copied_args);
}


/* documented in header file */
void log_output(const char *const format, ...)
{
    va_list args;

    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}


/* documented in header file */
void log_error(const char *const format, ...)
{
    va_list args;

    va_start(args, format);
    log_internal_vf(NULL, "Error", format, args);
    va_end(args);
}


/* documented in header file */
void log_warning(const char *const module,
                 const char *const format, ...)
{
    va_list args;

    va_start(args, format);
    log_internal_vf(module, "Warning", format, args);
    va_end(args);
}


/* documented in header file */
void log_verbose(const char *const module,
                 const char *const format, ...)
{
    va_list args;

    if (log_level > 0) {
        va_start(args, format);
        log_internal_vf(module, "Verbose", format, args);
        va_end(args);
    }
}


/* documented in header file */
void log_init(const int argc, char *const argv[]) {
    /* If argv[0] is "/foo/bar/beep", set progname to "beep".
     * Otherwise, set progname to argv[0] (if it does not contain a
     * slash) or keep the default progname value which points to a
     * constant string.
     */
    if (argc >= 1) {
        if ('\0' != *argv[0]) {
            const char *last_slash = strrchr(argv[0], '/');
            if (NULL != last_slash) {
                const char *post_last_slash = last_slash+1;
                if ('\0' != *post_last_slash) {
                    progname = post_last_slash;
                }
            } else {
                progname = argv[0];
            }
        }
    }
}


/**
 * beep-log internal function to initialize the log subsystem.
 *
 */

static
void log_constructor(void)
    __attribute__(( constructor ));

static
void log_constructor(void)
{
    const char *beep_log_level = secure_getenv("BEEP_LOG_LEVEL");
    /* silently ignore all errors, keeping the default log_level */
    if (beep_log_level) {
        if (*beep_log_level) {
            char *endptr = NULL;
            const long int i = strtol(beep_log_level, &endptr, 10);
            if (beep_log_level != endptr) {
                if (*endptr == '\0') {
                    if ((-999<=i) && (i<=999)) {
                        log_level = (int) i;
                    }
                }
            }
        }
    }
    LOG_VERBOSE("log_constructor");
}


/** @} */


/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
