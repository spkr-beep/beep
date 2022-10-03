/** \file beep-types.h
 * \brief Some types used internally for beeps.
 *
 * \author Copyright (C) 2000-2010 Johnathan Nightingale
 * \author Copyright (C) 2010-2013 Gerfried Fuchs
 * \author Copyright (C) 2013-2018 Hans Ulrich Niedermann
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
 */

#ifndef BEEP_TYPES_H
#define BEEP_TYPES_H


/**
 * Whether to to delay after the current repetition set of tones.
 */
typedef enum {
    END_DELAY_NO = 0,
    END_DELAY_YES = 1,
} end_delay_E;


/**
 * Whether to and how to react to data from stdin.
 */
typedef enum {
    STDIN_BEEP_NONE = 0,
    STDIN_BEEP_LINE = 1,
    STDIN_BEEP_CHAR = 2,
} stdin_beep_E;


/**
 * Per note parameter set.
 */
typedef struct beep_parms_T beep_parms_T;


/* The default values are defined in beep-config.h */


/**
 * Per note parameter set (including heritage information and linked list pointer).
 */
struct beep_parms_T
{
    unsigned int freq;       /**< tone frequency (Hz)         */
    unsigned int length;     /**< tone length    (ms)         */
    unsigned int reps;       /**< number of repetitions       */
    unsigned int delay;      /**< delay between reps  (ms)    */
    end_delay_E  end_delay;  /**< do we delay after last rep? */

    /** Are we using stdin triggers?
     *
     * We have three options:
     *   - just beep and terminate (default)
     *   - beep after a line of input
     *   - beep after a character of input
     * In the latter two cases, pass the text back out again,
     * so that beep can be tucked appropriately into a text-
     * processing pipe.
     */
    stdin_beep_E stdin_beep;

    beep_parms_T *next;      /**< in case -n/--new is used. */
};


#endif /* !defined(BEEP_TYPES_H) */


/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
