/** \file beep-event-loop.c
 * \brief main event loop for beep
 * \author Copyright (C) 2022 Hans Ulrich Niedermann
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
 * \defgroup beep_event_loop The main event loop
 *
 * @{
 *
 */


#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "auto-config.h"

#include "beep-drivers.h"
#include "beep-event-loop.h"
#include "beep-library.h"
#include "beep-log.h"


#define LOG_MODULE "evloop"


/** state machine states */
typedef enum {
    BEEPING,
    SILENCE,
    PAUSED_BEEPING,
    PAUSED_SILENCE,
    THE_END,
} state_T;


/** state machine event types */
typedef enum {
    EV_TIMEOUT,
    EV_TERMINATE,
    EV_PAUSE,
    EV_CONTINUE,
} event_T;


/** main event loop context */
typedef struct {
    beep_driver  *driver;
    beep_parms_T *parms;
    int          timerfd;
} context_T;


/** main event loop: helper function to set the timer */
static
void mel_set_timeout(const context_T *const context,
                     const unsigned int duration_ms)
    __attribute__(( nonnull(1) ));

static
void mel_set_timeout(const context_T *const context,
                     const unsigned int duration_ms)
{
    LOG_VERBOSE("mel_set_timeout(%u ms)", duration_ms);
    const unsigned int part_sec  = (duration_ms / 1000UL);
    const unsigned int part_nsec = (duration_ms % 1000UL) * 1000000UL;
    struct itimerspec new_times = {
        .it_interval = { .tv_sec=0,
                         .tv_nsec=0},
        .it_value    = { .tv_sec=part_sec,
                         .tv_nsec=part_nsec},
    };
    timerfd_settime(context->timerfd, 0, &new_times, NULL);
}


/** main event loop: FSM startup code */
static
state_T mel_fsm_startup(context_T *const context)
    __attribute__(( nonnull(1) ))
    __attribute__(( warn_unused_result ))
    ;

static
state_T mel_fsm_startup(context_T *const context)
{
    beep_drivers_begin_tone(context->driver,
                            context->parms->freq & 0xffff);
    mel_set_timeout(context, context->parms->length);
    return BEEPING;
}


/** main event loop: FSM process one event
 *
 * @dot
 * digraph main_event_loop {
 *    ranksep = 1.5;
 *
 *    node [shape=box];
 *
 *    //{   rank=same;
 *        begin [label="begin", shape=Mdiamond];
 *        end   [label="end",   shape=Msquare];
 *        // begin [shape=circle,label="",width=0.15,peripheries=1,color=black,style=filled];
 *        // end   [shape=circle,label="",width=0.15,peripheries=2,color=black,style=filled];
 *    //}
 *
 *        {   rank=same;
 *            beeping [label="beeping"];
 *            silence [label="silence"];
 *        }
 *        {   rank=same;
 *            paused_beeping [label="paused_beeping"];
 *            paused_silence [label="paused_silence"];
 *        }
 *
 *    begin -> beeping [label="start beep\nset timeout"];
 *    beeping -> silence [label="EV_TIMEOUT\nelse\nset timeout (repeat or to next beep)"];
 *    beeping -> paused_beeping [label="EV_PAUSE\nstop beep"];
 *    silence -> paused_silence [label="EV_PAUSE\n"];
 *    paused_beeping -> beeping [label="EV_CONT\nstart beep\ncontinue wait"];
 *    paused_silence -> silence [label="EV_CONT\ncontinue wait"];
 *    silence -> beeping [label="EV_TIMEOUT\nif next tone parms\nstart beep\nset timeout"];
 *    silence -> beeping [label="EV_DATA\nrepeat this tone parms\nstart beep\nset timeout"];
 *    beeping -> end [label="EV_TIMEOUT\nif tone seq done\nstop beep"];
 *
 *    silence        -> end [label="EV_TIMEOUT\nelse"];
 *
 *    beeping        -> end [label="EV_TERM\nstop beep"];
 *    silence        -> end [label="EV_TERM"];
 *    paused_beeping -> end [label="EV_TERM"];
 *    paused_silence -> end [label="EV_TERM"];
 * }
 * @enddot
 */
static
state_T mel_fsm_evhandler(context_T *const context,
                          const state_T state, const event_T event)
    __attribute__(( nonnull(1) ))
    __attribute__(( warn_unused_result ))
    ;

static
state_T mel_fsm_evhandler(context_T *const context,
                          const state_T state, const event_T event)
{
    switch (state) {
    case THE_END:
        /* This should never happen, but humour the compiler warnings */
        return THE_END;
    case BEEPING:
        switch (event) {
        case EV_TERMINATE:
            beep_drivers_end_tone(context->driver);
            return THE_END;
        case EV_PAUSE:
            beep_drivers_end_tone(context->driver);
            /* FIXME: Make sure we later continue at the same place */
            return PAUSED_BEEPING;
        case EV_CONTINUE:
            return state;
        case EV_TIMEOUT:
            beep_drivers_end_tone(context->driver);
            context->parms->reps--;
            if (context->parms->reps == 0) {
                switch (context->parms->end_delay) {
                case END_DELAY_NO:
                    /* AHEM. REALLY? */
                    mel_set_timeout(context, 100);
                    break;
                case END_DELAY_YES:
                    mel_set_timeout(context, context->parms->delay);
                    break;
                }
                context->parms = context->parms->next;
                return SILENCE;
            } else {
                if (context->parms->next) {
                    mel_set_timeout(context, context->parms->delay);
                    return SILENCE;
                } else {
                    return THE_END;
                }
            }
        }
        break;
    case PAUSED_BEEPING:
        switch (event) {
        case EV_TERMINATE:
            return THE_END;
        case EV_TIMEOUT:
            /* FIXME */
            return state;
        case EV_PAUSE:
            /* FIXME */
            return state;
        case EV_CONTINUE:
            beep_drivers_begin_tone(context->driver,
                                    context->parms->freq & 0xffff);
            /* FIXME: Make sure we continue the wait at the same place */
            return BEEPING;
        }
        break;
    case SILENCE:
        switch (event) {
        case EV_TERMINATE:
            return THE_END;
        case EV_CONTINUE:
            return state;
        case EV_PAUSE:
            /* FIXME: Make sure we later continue the waite the same place */
            return PAUSED_SILENCE;
        case EV_TIMEOUT:
            if (context->parms) {
                beep_drivers_begin_tone(context->driver,
                                        context->parms->freq & 0xffff);
                mel_set_timeout(context, context->parms->length);
                return BEEPING;
            } else {
                beep_drivers_end_tone(context->driver);
                return THE_END;
            }
        }
        break;
    case PAUSED_SILENCE:
        switch (event) {
        case EV_TERMINATE:
            return THE_END;
        case EV_CONTINUE:
            /* FIXME: Make sure we continue the wait */
            return SILENCE;
        case EV_TIMEOUT:
            /* FIXME */
            return PAUSED_SILENCE;
        case EV_PAUSE:
            /* FIXME */
            return PAUSED_SILENCE;
        }
    }
    beep_drivers_end_tone(context->driver);
    LOG_ERROR("internal logic error");
    _exit(2);
}


/** Max number of events the main event loop will process at once */
#define MAX_EVENTS 16


/* documented in header file */
int main_event_loop(beep_driver *driver, beep_parms_T *parms)
{
    /* As we use signalfd() to have the main loop react to received
     * signals in sequence, we do not need to deal with signal
     * handlers being asynchronous, volatile flags of atomic types, or
     * memory barriers.
     */

    /* main event loop
     *
     * We have 0 or 1 file descriptors to watch for either characters
     * or whatever units data arrives in, and some signals (SIGHUP,
     * SIGINT, SIGTERM terminate beep, SIGTSTP pauses beep, SIGCONT
     * continues beep).
     *
     * This can be handled by either poll() or epoll().
     *
     * states: BEEPING SILENCE
     */

    /* listen to a set of signals via signalfd()... */
    sigset_t fd_signal_mask;
    sigemptyset(&fd_signal_mask);
    sigaddset(&fd_signal_mask, SIGHUP);
    sigaddset(&fd_signal_mask, SIGINT);
    sigaddset(&fd_signal_mask, SIGTERM);
    sigaddset(&fd_signal_mask, SIGTSTP);
    sigaddset(&fd_signal_mask, SIGSTOP);
    sigaddset(&fd_signal_mask, SIGCONT);
    const int sigfd = signalfd(-1, &fd_signal_mask, SFD_CLOEXEC);
    if (sigfd == -1) {
        safe_errno_exit("signalfd()");
    }

    /* ...and block these signals' handlers */
    if (sigprocmask(SIG_BLOCK, &fd_signal_mask, NULL) == -1) {
        safe_errno_exit("sigprocmask(SIG_BLOCK, ...)");
    }

    /* To be emulated by a state machine: */
    /* */
    /* for (unsigned int i = 0; (!global_flag_abort) && (i < parms.reps); i++) { */
    /*     beep_drivers_begin_tone(driver, parms.freq & 0xffff); */
    /*     sleep_ms(driver, parms.length); */
    /*     beep_drivers_end_tone(driver); */
    /*     if ((parms.end_delay == END_DELAY_YES) || ((i+1) < parms.reps)) { */
    /*         sleep_ms(driver, parms.delay); */
    /*     } */
    /* } */

    const int epollfd = epoll_create1(0);
    if (epollfd == -1) {
        safe_errno_exit("epoll_create1()");
    }

    if (true) {
        struct epoll_event ev;
        ev.data.fd = sigfd;
        ev.events  = EPOLLIN;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1) {
            safe_errno_exit("epoll_ctl() EPOLL_CTL_ADD sigfd");
        }
    }

    const int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timerfd == -1) {
        safe_errno_exit("timerfd_create()");
    }

    if (true) {
        struct epoll_event ev;
        ev.data.fd = timerfd;
        ev.events  = EPOLLIN;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1) {
            safe_errno_exit("epoll_ctl() EPOLL_CTL_ADD timerfd");
        }
    }

    context_T context = {
        .driver   = driver,
        .parms    = parms,
        .timerfd  = timerfd,
    };
    state_T state = mel_fsm_startup(&context);

    while (true) {
        LOG_VERBOSE("event loop body begin");

        struct epoll_event events[MAX_EVENTS];

        LOG_VERBOSE("doing epoll_wait(...)");
        const int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            if (errno == EINTR) {
                LOG_VERBOSE("epoll_wait() EINTR");
            } else {
                safe_errno_exit("epoll_wait() error");
            }
        } else if (nfds == 0) {
            LOG_VERBOSE("epoll_wait() timeout");
        } else if (nfds > 0) {
            LOG_VERBOSE("epoll_wait() ready to read (nfds==%d)", nfds);
            for (int n = 0; n < nfds; ++n) {
                if (events[n].data.fd == sigfd) {
                    LOG_VERBOSE("reading event from sigfd");
                    struct signalfd_siginfo siginfo[MAX_EVENTS];
                    const ssize_t ssz = read(sigfd, siginfo, sizeof(siginfo));
                    if (ssz < 0) {
                        safe_errno_exit("reading signalfd event");
                    } else {
                        const size_t usz = ssz;
                        LOG_VERBOSE("read %zu bytes from sigfd (%zu)",
                                    usz, sizeof(struct signalfd_siginfo));
                        if (usz < sizeof(struct signalfd_siginfo)) {
                            LOG_ERROR("only read %zu bytes from sigfd", usz);
                            _exit(EXIT_FAILURE);
                        }
                    }
                    const size_t max_m = ((size_t) ssz) / sizeof(struct signalfd_siginfo);
                    for (size_t m=0; m < max_m; m++) {
                        switch (siginfo[m].ssi_signo) {
                        case SIGHUP:
                            LOG_VERBOSE("SIGHUP");
                            state = mel_fsm_evhandler(&context, state, EV_TERMINATE);
                            break;
                        case SIGINT:
                            LOG_VERBOSE("SIGINT");
                            state = mel_fsm_evhandler(&context, state, EV_TERMINATE);
                            break;
                        case SIGTERM:
                            LOG_VERBOSE("SIGTERM");
                            state = mel_fsm_evhandler(&context, state, EV_TERMINATE);
                            break;
                        case SIGCONT:
                            LOG_VERBOSE("SIGCONT");
                            state = mel_fsm_evhandler(&context, state, EV_CONTINUE);
                            break;
                        case SIGSTOP:
                            // TODO: We cannot catch this, right?
                            LOG_VERBOSE("SIGSTOP");
                            state = mel_fsm_evhandler(&context, state, EV_PAUSE);
                            break;
                        case SIGTSTP:
                            LOG_VERBOSE("SIGTSTP");
                            state = mel_fsm_evhandler(&context, state, EV_PAUSE);
                            break;
                        default:
                            LOG_ERROR("unknown signal: %d", siginfo[m].ssi_signo);
                            state = mel_fsm_evhandler(&context, state, EV_TERMINATE);
                            break;
                        }
                    }
                } else if (events[n].data.fd == timerfd) {
                    LOG_VERBOSE("reading event from timerfd");
                    uint64_t value;
                    const ssize_t ssz = read(timerfd, &value, sizeof(value));
                    if (ssz < 0) {
                        safe_errno_exit("reading timerfd event");
                    } else if (ssz == sizeof(value)) {
                        /* ok */
                    } else {
                        LOG_ERROR("error reading from timerfd (%zd bytes)", ssz);
                        _exit(EXIT_FAILURE);
                    }
                    state = mel_fsm_evhandler(&context, state, EV_TIMEOUT);
                } else { /* input file */
                    /* FIXME: Handle input data */
                    safe_errno_exit("to be implemented: read from input file");
                }
            }
        }
        LOG_VERBOSE("event loop body end");
        if (state == THE_END) {
            break;
        }
    }

    beep_drivers_fini(driver);
    return EXIT_SUCCESS;
}


/** @} */


/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
