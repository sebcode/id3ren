/* id3 Renamer
 * id3tag.h - Header for id3 tag manipulation functions
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


#ifndef __ID3TAG_H__
#define __ID3TAG_H__

void show_genres (int);
int search_genre (int, int *, char *);
void resize_tag_field (char *);
int get_tag_genre (int *, int);
int get_tag_string (int, char *, char *, char *);
int ask_tag (char *);
int tag_file (char *);


#endif /* __ID3TAG_H__ */
