/* id3 Renamer
 * id3file.c - File i/o functions
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


#include <stdio.h>
/* open */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "id3file.h"
#include "id3genre.h"
#include "id3ren.h"
#include "id3misc.h"


/* todo: add option to change log filename */
const char logfile[] = "id3ren.log";


int
add_to_log (char *data)
{
  FILE *fp;

  if (id3_open_file(&fp, (char*)logfile, "at") == FALSE)
    return FALSE;

  fprintf(fp, "%s", data);
  fclose(fp);

  return TRUE;
}


int
id3_read_file (char *dest, unsigned long size, FILE *fp, char *fn)
{
  if (fread(dest, size, 1, fp) != 1)
  {
    print_error("id3_read_file: Error reading %s", fn);
    fclose(fp);
    return FALSE;
  }

  if (ferror(fp) != 0)
  {
    print_error("id3_read_file: Error reading %s", fn);
    clearerr(fp);
    fclose(fp);
    return FALSE;
  }

  if (feof(fp))
  {
    print_error("id3_read_file: Premature end of file in %s", fn);
    fclose(fp);
    return FALSE;
  }

  return TRUE;
}


int
id3_write_file(char *src, unsigned long size, FILE *fp, char *fn)
{
  if (fwrite(src, size, 1, fp) != 1)
  {
    print_error("id3_write_file: Error writing to %s", fn);
    fclose(fp);
    return FALSE;
  }

  if (ferror(fp) != 0)
  {
    print_error("id3_write_file: Error writing to %s", fn);
    clearerr(fp);
    fclose(fp);
    return FALSE;
  }

  return TRUE;
}


int
id3_open_file (FILE **fp, char *fn, char *mode)
{
  *fp = fopen(fn, mode);

  if (*fp == NULL)
  {
    print_error("id3_open_file: Error opening file %s", fn);
    return FALSE;
  }

  return TRUE;
}

int
id3_close_file (FILE *fp)
{
  return fclose(fp);
}

int
id3_seek_header (FILE *fp, char *fn)
{

  if (fseek(fp, -128, SEEK_END) < 0)
  {
    fclose(fp);
    print_error("id3_seek_header: Error reading file %s", fn);
    return FALSE;
  }

  return TRUE;
}


int
id3_strip_tag (long sizelesstag, char *fn)
{
  int fd;

  fd = open(fn, O_RDWR);

  if (fd == -1)
  {
    print_error("strip_tag: Error opening %s", fn);
    return FALSE;
  }

#ifdef __WIN32__
  chsize(fd, sizelesstag);
#else
  ftruncate(fd, sizelesstag);
#endif

  close(fd);
  return TRUE;
}

int
id3_write_tag (ID3_tag *tag, int append_flag, char *fn)
{
  FILE *fp;

  if (append_flag == TRUE)
  {
    if (id3_open_file(&fp, fn, "ab") == FALSE)
      return FALSE;
  }
  else
  {
    if (id3_open_file(&fp, fn, "r+b") == FALSE)
      return FALSE;

    if (id3_seek_header(fp, fn) == FALSE)
      return FALSE;
  }

  strcpy(tag->tag, "TAG");
  if (!id3_write_file(tag->tag, (sizeof(tag->tag)-1), fp, fn))
    return FALSE;
  if (!id3_write_file(tag->songname, (sizeof(tag->songname)-1), fp, fn))
    return FALSE;
  if (!id3_write_file(tag->artist, (sizeof(tag->artist)-1), fp, fn))
    return FALSE;
  if (!id3_write_file(tag->album, (sizeof(tag->album)-1), fp, fn))
    return FALSE;
  if (!id3_write_file(tag->year, (sizeof(tag->year)-1), fp, fn))
    return FALSE;
  if (!id3_write_file(tag->u.v10.comment, (sizeof(tag->u.v10.comment)-1), fp, fn))
    return FALSE;
/*    fwrite(tag->genre, 1, 1, fp);*/

  if (fputc(tag->genre, fp) == EOF)
  {
    fclose(fp);
    print_error("write_tag: Error writing to %s", fn);
    return FALSE;
  }

  fclose(fp);
  return TRUE;
}


int
id3_read_tag (ID3_tag *tag, FILE *fp, char *fn)
{
  if (!id3_read_file(tag->songname, (sizeof(tag->songname)-1), fp, fn))
    return FALSE;
  if (!id3_read_file(tag->artist, (sizeof(tag->artist)-1), fp, fn))
    return FALSE;
  if (!id3_read_file(tag->album, (sizeof(tag->album)-1), fp, fn))
    return FALSE;
  if (!id3_read_file(tag->year, (sizeof(tag->year)-1), fp, fn))
    return FALSE;
  if (!id3_read_file(tag->u.v10.comment, (sizeof(tag->u.v10.comment)-1), fp, fn))
    return FALSE;

  /* Detect v1.1 tag */
  if((tag->u.v10.comment[28]==0)
   &&(tag->u.v10.comment[29]!=0))
   tag->version=1;

/*  fread(&tag->genre, 1, 1, fp); */
  tag->genre = fgetc(fp);

  if (tag->genre == EOF)
  {
    fclose(fp);
    print_error("tag_file: Error reading %s", fn);
    return FALSE;
  }

  return TRUE;
}

void
id3_show_tag (ID3_tag *tag, char *fn)
{
  user_message(FALSE, "===> %s:\n", fn);

  if (strlen(tag->songname) > 0)
    user_message(FALSE, "Song Name: %s\n", tag->songname);

  if (strlen(tag->artist) > 0)
    user_message(FALSE, "   Artist: %s\n", tag->artist);

  if (strlen(tag->album) > 0)
    user_message(FALSE, "    Album: %s\n", tag->album);

  if (strlen(tag->year) > 0)
    user_message(FALSE, "     Year: %s\n", tag->year);

  if (strlen(tag->u.v10.comment) > 0)
    user_message(FALSE, "  Comment: %s\n", tag->u.v10.comment);

  if (tag->version == 1 && tag->u.v11.track !=0)
    user_message(FALSE, "    Track: %02d\n", tag->u.v11.track);

  if (tag->genre >= 0 && tag->genre < genre_count)
    user_message(FALSE, "    Genre: %s\n", genre_table[tag->genre]);

  user_message(FALSE, "\n");
}
