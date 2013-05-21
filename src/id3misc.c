/* id3 Renamer
 * id3misc.c - Miscellaneous functions for id3ren
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>

#include "id3file.h"
#include "id3misc.h"
#include "id3ren.h"


extern FLAGS_struct flags;
extern char *program_name;


void
alloc_string (char **dest, unsigned long size)
{
  *dest = (char *)malloc(size);

  if (*dest == NULL)
  {
    print_error("alloc_string: Out of memory for malloc");
    exit(ERRLEV_MALLOC);
  }
}

int
strcase_search (char *in_s1, char *in_s2)
{
  char *s1, *s2;
  int retflag = FALSE;

  s1 = (char *)malloc( (strlen(in_s1)+1) );
  if (s1 == NULL)
  {
    print_error("strcase_search: Out of memory for malloc");
    exit(ERRLEV_MALLOC);
  }
  s2 = (char *)malloc( (strlen(in_s2)+1) );
  if (s2 == NULL)
  {
    free(s1);
    print_error("strcase_search: Out of memory for malloc");
    exit(ERRLEV_MALLOC);
  }

  strcpy(s1, in_s1);
  strcpy(s2, in_s2);
  string_lower(s1);
  string_lower(s2);

  if (strstr(s1, s2) == NULL)
    retflag = FALSE;
  else
    retflag = TRUE;

  free(s1);
  free(s2);
  return retflag;
}

void
string_lower (char *lowstr)
{
  int i;

  for (i = 0; i < strlen(lowstr); i++)
    lowstr[i] = tolower(lowstr[i]);
}


int
get_term_lines (void)
{
  char *lines;

  lines = getenv("LINES");

  if (lines == NULL || atoi(lines) < 1)
    return 25;

  return (int)atoi(lines);
}


void
print_error (char *format, ...)
{
  char buf[1024];
  va_list msg;

  va_start(msg, format);
  vsprintf(buf, format, msg);
  va_end(msg);

  fprintf(stderr, "%s: %s: %s\n", program_name, buf, strerror(errno));

  if (flags.logging == TRUE)
  {
    sprintf(buf, "%s: %s: %s\n", program_name, buf, strerror(errno));
    add_to_log(buf);
  }
}


void
user_message (int errflag, char *format, ...)
{
  char buf[1024];
  va_list msg;

  va_start(msg, format);
  vsprintf(buf, format, msg);
  va_end(msg);

  if (errflag)
    fprintf(stderr, "%s", buf);
  else if (flags.quiet == FALSE)
    printf("%s", buf);

  if (flags.logging == TRUE)
    add_to_log(buf);

}
