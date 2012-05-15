/* id3 Renamer
 * id3ren.h - Header for main functions
 * Copyright (C) 1998  Robert Alto (badcrc@tscnet.com)
 * Copyright (C) 2001  Christophe Bothamy (cbothamy@free.fr)
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

#ifndef __ID3REN_H__
#define __ID3REN_H__

#define SHORTVERSION "1.1b0"

#ifdef __WIN32__
  #define APP_VERSION SHORTVERSION" for Win32"
  #define IDENT_CHAR '$'
  #define PATH_CHAR '\\'
  #define CONFIG_HOME "id3ren.rc"
  #define CONFIG_GLOBAL "id3ren.rc"
#else
  #define APP_VERSION SHORTVERSION" for *nix"
  #define IDENT_CHAR '%'
  #define PATH_CHAR '/'
  #define CONFIG_HOME ".id3renrc"
  #define CONFIG_GLOBAL "id3renrc"
#endif

#undef WIN32_FIND

#ifndef TRUE
 #define TRUE 1
#endif
#ifndef FALSE
 #define FALSE 0
#endif

#define ERRLEV_SUCCESS (0)
#define ERRLEV_MALLOC  (1)
#define ERRLEV_ARGS    (2)
#define ERRLEV_OTHER   (3)


typedef struct
{
  /* Log normal output to a file */
  short logging;

  /* Don't prompt for tags on files without tags */
  short no_tag_prompt;

  /* Don't read config files */
  short no_config;

  /* Run silently */
  short quiet;

  /* Show lots of stuff */
  short verbose;

  /* Only display tags */
  short show_tag;

  /* Tag from file name */
  short tag_ffn;

 /* Strip tag */
  short strip_tag;

  /* Always prompt for tag info, even when renaming files with tags */
  short force_tag;

  /* Don't do any renaming, only prompt for tags */
  short tag_only;

  /* If a tag field already has info, don't prompt for that */
  short edit_tag;

  /* Skip asking for these tag items? */
  short no_year;
  short no_album;
  short no_comment;
  short no_genre;
  short no_track;

  /* 0-default 1-upper 2-lower */
  short ulcase;
} FLAGS_struct;


#endif /* __ID3REN_H__ */

