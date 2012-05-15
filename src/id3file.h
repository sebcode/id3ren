/* id3 Renamer
 * file.h - Header for file i/o functions
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

#ifndef __FILE_H__
#define __FILE_H__

#define TAGLEN_TAG 3
#define TAGLEN_SONG 30
#define TAGLEN_ARTIST 30
#define TAGLEN_ALBUM 30
#define TAGLEN_YEAR 4
#define TAGLEN_COMMENT_V10 30
#define TAGLEN_COMMENT_V11 28
#define TAGLEN_GENRE 1

typedef struct ID3_struct
{ // version: 0 is v1.0 - 1 is v1.1
  char version;  
  char tag[TAGLEN_TAG+1];
  char songname[TAGLEN_SONG+1];
  char artist[TAGLEN_ARTIST+1];
  char album[TAGLEN_ALBUM+1];
  char year[TAGLEN_YEAR+1];
  union 
  {
    struct 
    {
      char comment[TAGLEN_COMMENT_V10+1];
    }v10;
    struct 
    {
      char comment[TAGLEN_COMMENT_V11+1];
      char track;
    }v11;
  }u;
  int genre;
} ID3_tag;

int add_to_log (char *);
int id3_read_file (char *, unsigned long, FILE *, char *);
int id3_write_file (char *, unsigned long, FILE *, char *);
int id3_open_file (FILE **, char *, char *);
int id3_close_file (FILE *);
int id3_seek_header (FILE *, char *);
int id3_strip_tag (long, char *);

int id3_write_tag (ID3_tag *,int, char *);
int id3_read_tag (ID3_tag *,FILE *, char *);
void id3_show_tag (ID3_tag *, char *);

#endif /* __FILE_H__ */
