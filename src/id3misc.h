/* id3 Renamer
 * misc.h - Header for miscellaneous functions for id3ren
 * Copyright (C) 1998  Robert Alto (badcrc@tscnet.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#ifndef __MISC_H__
#define __MISC_H__

void alloc_string (char **, unsigned long);
int strcase_search (char *, char *);
void string_lower (char *);
int get_term_lines (void);
void print_error (char *, ...);
void user_message (int, char *, ...);


#endif /* __MISC_H__ */
