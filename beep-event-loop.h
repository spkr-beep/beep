/** \file beep-event-loop.h
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
 * \addtogroup beep_event_loop
 *
 * @{
 *
 */


#ifndef BEEP_EVENT_LOOP_H
#define BEEP_EVENT_LOOP_H


#include "beep-driver.h"
#include "beep-types.h"


/**
 * main event loop
 *
 * Condense the raw events into events for the main event loop state
 * machine.
 */

int main_event_loop(beep_driver *driver, beep_parms_T *parms)
    __attribute__(( nonnull(1,2) ))
    __attribute__(( warn_unused_result ))
    ;


#endif /* !defined(BEEP_EVENT_LOOP_H) */


/** @} */


/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
