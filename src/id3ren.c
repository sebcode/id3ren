/* id3 Renamer
 * id3ren.c - Main functions
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
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#ifdef __WIN32__
 #include <windows.h>
#endif

#include "id3file.h"
#include "id3misc.h"
#include "id3tag.h"
#include "id3ren.h"

#define EXIT_PAUSE FALSE


extern ID3_tag *ptrtag;
extern const int genre_count;
extern char *genre_table[];
extern const char logfile[];

char *def_artist = NULL;
char *def_song = NULL;
char *def_album = NULL;
char *def_year = NULL;
char *def_comment = NULL;
int  def_genre = -1;
char def_track = -1;
char *def_field = "unknown";

ID3_tag copytag;
FILE *copyfp=NULL;

FLAGS_struct flags = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
                       FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, 
                       FALSE, 0 };


/*
 * Default template
 * %a - artist
 * %c - comment
 * %g - genre
 * %s - song name
 * %t - album title
 * %y - year
 * %n - track
 *
 */
#ifdef __WIN32__
  char filename_template[256] = "[$a]-[$s].mp3";
#else
  char filename_template[256] = "[%a]-[%s].mp3";
#endif

#ifdef __WIN32__
  char tag_template[256] = "[$a]-[$s].mp3";
#else
  char tag_template[256] = "[%a]-[%s].mp3";
#endif

char *program_name = NULL;
char *program_path = NULL;

/* character to replace spaces with (defaults to " ") */
char replace_spacechar[32] = " ";

char *replace_char = "";
char *remove_char = "";

/* template-applied filename */
char applied_filename[512];


void exit_function (void);
void apply_template (char *);
void show_usage (char *);
void check_num_args (int, int);
int read_config (char *, char *);
void toggle_flag (short *);
int check_option (int *, int , char *, char *, char *, char **, int );
void check_arg (int *, int, char *, char *);
int check_args (int, char *[]);
int main (int, char *[]);


void
exit_function (void)
{
  if (EXIT_PAUSE)
    while(getc(stdin)!='\n') ;
}


void
apply_template (char *origfile)
{
  char *ptrNewfile;
  char tmpfile[512];
  char strack[10];
  int i, tcount = 0;

  applied_filename[0] = '\0';
  ptrNewfile = applied_filename;

  while (filename_template[tcount] != '\0')
  {
    if (filename_template[tcount] == IDENT_CHAR)
    {
      tcount++;

      switch (filename_template[tcount])
      {
        case 'a': 
          if(strcmp(ptrtag->artist,"")!=0)
           strcat(applied_filename, ptrtag->artist); 
          else if(def_field!=NULL)
           strcat(applied_filename, def_field); 
          break;
        case 'c': 
          if(strcmp(ptrtag->u.v10.comment,"")!=0)
           strcat(applied_filename, ptrtag->u.v10.comment);
          else if(def_field!=NULL)
           strcat(applied_filename, def_field); 
          break;
        case 'g':
          if (ptrtag->genre < genre_count && ptrtag->genre >= 0)
           strcat(applied_filename, genre_table[ptrtag->genre]);
          else if(def_field!=NULL)
           strcat(applied_filename, def_field); 
          break;
        case 's': 
          if(strcmp(ptrtag->songname,"")!=0)
           strcat(applied_filename, ptrtag->songname); 
          else if(def_field!=NULL)
           strcat(applied_filename, def_field); 
          break;
        case 't': 
          if(strcmp(ptrtag->album,"")!=0)
           strcat(applied_filename, ptrtag->album); 
          else if(def_field!=NULL)
           strcat(applied_filename, def_field); 
          break;
        case 'y': 
          if(strcmp(ptrtag->year,"")!=0)
           strcat(applied_filename, ptrtag->year); 
          else if(def_field!=NULL)
           strcat(applied_filename, def_field); 
          break;
        case 'n':   
          if (ptrtag->version >=1)
           {sprintf(strack,"%02d",ptrtag->u.v11.track);
            strcat(applied_filename, strack); 
           }
          else if(def_field!=NULL)
           strcat(applied_filename, def_field); 
          break;
        default:
          user_message(FALSE, "Unknown identifier in template: %c%c\n",
            IDENT_CHAR, filename_template[tcount]);
          break;
      }

      tcount++;
    }
    else
    {
      ptrNewfile = applied_filename;
      ptrNewfile += strlen(applied_filename);
      *ptrNewfile = filename_template[tcount];
      ptrNewfile++;
      *ptrNewfile = '\0';
      tcount++;
    }
  }

  strcpy(tmpfile, applied_filename);
  ptrNewfile = applied_filename;
  tcount = 0;

  while (tmpfile[tcount] != '\0')
  {
    for (i = 0; i < strlen(replace_char); i += 2)
    {
      if ( replace_char[i] == tmpfile[tcount] &&
           ((i+1) < strlen(replace_char)) )
      {
        *ptrNewfile = replace_char[i+1];
        ptrNewfile++;
        i = 31335;
      }
    }

    if (i != 31337 && strchr(remove_char, tmpfile[tcount]) == NULL)
    {
      if (isalpha(tmpfile[tcount]) && flags.ulcase == 1)
      {
        *ptrNewfile = toupper(tmpfile[tcount]);
        ptrNewfile++;
      }
      else if (isalpha(tmpfile[tcount]) && flags.ulcase == 2)
      {
        *ptrNewfile = tolower(tmpfile[tcount]);
        ptrNewfile++;
      }
      else if (tmpfile[tcount] == ' ')
      {
        *ptrNewfile = '\0';
        strcat(applied_filename, replace_spacechar);
        ptrNewfile = applied_filename;
        ptrNewfile += strlen(applied_filename);
      }
      else
      {
        *ptrNewfile = tmpfile[tcount];
        ptrNewfile++;
      }
    }

    tcount++;
  }

  *ptrNewfile = '\0';

  /* Convert illegal or bad filename characters to good characters */
  for (i=0; i < strlen(applied_filename); i++)
  {
    switch (applied_filename[i])
    {
      case '<': applied_filename[i] = '['; break;
      case '>': applied_filename[i] = ']'; break;
      case '|': applied_filename[i] = '_'; break;
      // this is now used to create dirs
      //case '/': applied_filename[i] = '-'; break;
      case '\\': applied_filename[i]= '-'; break;
      case '*': applied_filename[i] = '_'; break;
      case '?': applied_filename[i] = '_'; break;
      case ':': applied_filename[i] = ';'; break;
      case '"': applied_filename[i] = '-'; break;
      default: break;
    }
  }

}


void
show_usage (char *myname)
{
  user_message(TRUE, "GPL'd id3 Renamer & Tagger version %s\n", APP_VERSION);
  user_message(TRUE, "(C) Copyright 1998 by Robert Alto (badcrc@tscnet.com)\n");
  user_message(TRUE, "(C) Copyright 2001 by Christophe Bothamy (cbothamy@free.fr)\n");
  user_message(TRUE, "Usage: %s [-help]\n",myname);
  user_message(TRUE, "       [-song=SONG_NAME] [-artist=ARTIST_NAME] [-album=ALBUM_NAME]\n");
  user_message(TRUE, "       [-year=YEAR] [-genre={# | GENRE}] [-comment=COMMENT] [-track=TRACK]\n");
  user_message(TRUE, "       [-showgen] [-searchgen={# | GENRE}] [-default=DEFAULT]\n");
  user_message(TRUE, "       [-copytagfrom=FILE [-copysong] [-copyartist] [-copyalbum]\n");
  user_message(TRUE, "       [-copyyear] [-copygenre] [-copycomment] [-copytrack] [-copyall] ]\n");
  user_message(TRUE, "       [-quick] [-noalbum] [-nocomment] [-noyear] [-nogenre] [-notrack]\n");
  user_message(TRUE, "       [-tag] [-edit] [-notagprompt | -showtag | -striptag | -tagonly]\n");
  user_message(TRUE, "       [-nocfg] [-log] [-quiet] [-verbose] [-defcase | -lower | -upper]\n");
  user_message(TRUE, "       [-remchar=CHARS] [-repchar=CHARS] [-space=STRING]\n");
  user_message(TRUE, "       [-tagfromfilename | -tagffn] [-tagtemplate=TAGTEMPLATE]\n");   
  user_message(TRUE, "       [-template=TEMPLATE] [FILE1 FILE2.. | WILDCARDS]\n\n");    
  user_message(TRUE, "When logging is enabled, most normal output is also sent to %s.\n", logfile);   
  user_message(TRUE, "To disable all output except for the usage screen on errors, use -quiet.\n\n");   
  user_message(TRUE, "The templates can contain the following identifiers from the id3 tag:\n");   
  user_message(TRUE, " %ca - Artist       %cc - Comment  %cg - Genre  %cs - Song Name\n",IDENT_CHAR, IDENT_CHAR, IDENT_CHAR, IDENT_CHAR);   
  user_message(TRUE, " %ct - Album title  %cy - Year     %cn - Track ##\n",IDENT_CHAR,IDENT_CHAR,IDENT_CHAR); 
  user_message(TRUE, "The tagtemplate can also contain the dummy identifier %cd.\n",IDENT_CHAR);   
 }


void
check_num_args (int current, int total)
{
  if (current >= total)
  {
    user_message(TRUE, "%s: Not enough arguments\n", program_name);
    exit(ERRLEV_ARGS);
  }
}


int
read_config (char *path, char *filename)
{
  FILE *fp;
  char buffer[1024];
  char *cfile;
  char *p1, *p2;
  int i;
  char first[1024];
  char second[1024];
  char insingle,indouble;

  if (path == NULL || strlen(path) < 1)
    return FALSE;

  if (filename == NULL || strlen(filename) < 1)
  {
    alloc_string(&cfile, (strlen(path) + 1));
    strcpy(cfile, path);
  }
  else
  {
    alloc_string(&cfile, (strlen(path) + strlen(filename) + 2));
    sprintf(cfile, "%s%c%s", path, PATH_CHAR, filename);
  }

  if (flags.verbose)
    user_message(FALSE, "%s: Checking for config file %s...", program_name, cfile);

  if (access(cfile, F_OK) != 0)
  {
    if (flags.verbose)
      user_message(FALSE, "not found\n");

    free(cfile);
    return FALSE;
  } else if (flags.verbose)
    user_message(FALSE, "found\n");

  fp = fopen(cfile, "rt");

  if (fp == NULL)
  {
    print_error("Couldn't open config file %s", cfile);
    free(cfile);
    return FALSE;
  }

  if (flags.verbose)
    user_message(FALSE, "%s: Reading config file %s\n", program_name, cfile);

  fgets(buffer, sizeof(buffer), fp);

  while (!feof(fp))
  {
    if (buffer[strlen(buffer)-1] == '\n')
      buffer[strlen(buffer)-1] = '\0';

    p1 = buffer;
    while (*p1 == ' ') p1++;

    if (*p1 != '\0' && *p1 != '#')
    {
      p2 = p1;

      while (*p2 != '=' && *p2 != ' ' && *p2 != '\0') p2++;

      if (*p2 == ' ' || *p2 == '=')
      {
        *p2 = '\0';
        p2++;
      }
      
      // Go to next non space
      while (*p2 == ' ' && *p2 != '\0') p2++;

      // Handle double and simple quotes
      strcpy(first,p1);
      strcpy(second,"");
      insingle=0;indouble=0;
      while(*p2!='\0')
       { 
        if(insingle)
         {if(*p2=='\'')insingle=0;
          else strncat(second,p2,1);
         }
        else if(indouble)
         {if(*p2=='"')indouble=0;
          else strncat(second,p2,1);
         }
        else if(*p2=='"')indouble=1;
        else if(*p2=='\'')insingle=1;
        else if(*p2!=' ')strncat(second,p2,1);
        else break;
        
        p2++;
       }
      
      i = 0;
      check_arg (&i, ((second[0] == '\0') ? 1 : 2), first, ((second[0] == '\0') ? "" : second));
    }

    fgets(buffer, sizeof(buffer), fp);
  }

  fclose(fp);
  free(cfile);
  return TRUE;
}

void
toggle_flag (short *flag)
{
  if (*flag == TRUE)
  {
    *flag = FALSE;
  }
  else
  {
    *flag = TRUE;
  }
}

int 
check_option (int *count, int argc, char *arg1, char *arg2, char *name, char **option, int nb)
{char *stmp;
 char found=0;

 *option=NULL;

 // Single option
 if(nb==0)
  {
   if(strcmp(name,arg1)==0)
    found=1;
  }
 else // Option with param
  {
   // Option with next arg
   if(strcmp(name,arg1)==0)
    {
     (*count)++;
     check_num_args(*count, argc);

     *option=arg2;
     found=1;
    }
   else
    {// option with =
     alloc_string(&stmp, (strlen(name)+2));
     sprintf(stmp,"%s=",name);
     if(strncmp(stmp,arg1,strlen(stmp))==0)
      {// Option begins next char
       *option=strchr(arg1,'=')+1;
       found=1;
      }
     free(stmp);
    }
  }

 if(found)return 1;
 else return 0;
}

void
check_arg (int *count, int argc, char *arg1, char *arg2)
{
  char *option;
  int i;

  if(check_option(count,argc,arg1,arg2,"-help",&option,0)!=0)
  {
    show_usage(program_name);
    exit(ERRLEV_SUCCESS);
  }
  else if(check_option(count,argc,arg1,arg2,"-song",&option,1)!=0)
  {
    alloc_string(&def_song, (strlen(option)+1));
    strcpy(def_song, option);
  }
  else if(check_option(count,argc,arg1,arg2,"-artist",&option,1)!=0)
  {
    alloc_string(&def_artist, (strlen(option)+1));
    strcpy(def_artist, option);
  }
  else if(check_option(count,argc,arg1,arg2,"-album",&option,1)!=0)
  {
    alloc_string(&def_album, (strlen(option)+1));
    strcpy(def_album, option);
  }
  else if(check_option(count,argc,arg1,arg2,"-year",&option,1)!=0)
  {
    alloc_string(&def_year, (strlen(option)+1));
    strcpy(def_year, option);
  }
  else if(check_option(count,argc,arg1,arg2,"-track",&option,1)!=0)
  {
  
    def_track=0;
    for(i=0;i<strlen(option);i++)
     def_track=def_track*10+option[i]-'0';
    if(def_track>99)
     {
      user_message(TRUE, "%s: track %s is > 99\n", program_name,option);
      exit(ERRLEV_ARGS);
     }
  }
  else if(check_option(count,argc,arg1,arg2,"-comment",&option,1)!=0)
  {
    alloc_string(&def_comment, (strlen(option)+1));
    strcpy(def_comment, option);
  }
  else if(check_option(count,argc,arg1,arg2,"-genre",&option,1)!=0)
  {
    if (search_genre(1, &def_genre, option) == FALSE)
    {
      user_message(TRUE, "%s: No genre selected\n", program_name);
      exit(ERRLEV_ARGS);
    }
  }
  else if(check_option(count,argc,arg1,arg2,"-default",&option,1)!=0)
  {
    alloc_string(&def_field, (strlen(option)+1));
    strcpy(def_field, option);
  }
  else if(check_option(count,argc,arg1,arg2,"-log",&option,0)!=0)
    toggle_flag((short*)&flags.logging);
  else if(check_option(count,argc,arg1,arg2,"-notagprompt",&option,0)!=0)
    toggle_flag((short*)&flags.no_tag_prompt);
  else if(check_option(count,argc,arg1,arg2,"-noalbum",&option,0)!=0)
    toggle_flag((short*)&flags.no_album);
  else if(check_option(count,argc,arg1,arg2,"-nocomment",&option,0)!=0)
    toggle_flag((short*)&flags.no_comment);
  else if(check_option(count,argc,arg1,arg2,"-noyear",&option,0)!=0)
    toggle_flag((short*)&flags.no_year);
  else if(check_option(count,argc,arg1,arg2,"-notrack",&option,0)!=0)
    toggle_flag((short*)&flags.no_track);
  else if(check_option(count,argc,arg1,arg2,"-nogenre",&option,0)!=0)
    toggle_flag((short*)&flags.no_genre);
  else if(check_option(count,argc,arg1,arg2,"-nocfg",&option,0)!=0)
    toggle_flag((short*)&flags.no_config);
  else if(check_option(count,argc,arg1,arg2,"-quick",&option,0)!=0)
  {
    flags.no_album = TRUE;
    flags.no_comment = TRUE;
    flags.no_year = TRUE;
  }
  else if(check_option(count,argc,arg1,arg2,"-quiet",&option,0)!=0)
    toggle_flag((short*)&flags.quiet);
  else if(check_option(count,argc,arg1,arg2,"-verbose",&option,0)!=0)
    toggle_flag((short*)&flags.verbose);
  else if(check_option(count,argc,arg1,arg2,"-searchgen",&option,1)!=0)
  {
    search_genre(2, &def_genre, option);
    exit(ERRLEV_SUCCESS);
  }
  else if(check_option(count,argc,arg1,arg2,"-showgen",&option,0)!=0)
  {
    show_genres(FALSE);
    exit(ERRLEV_SUCCESS);
  }
  else if(check_option(count,argc,arg1,arg2,"-showtag",&option,0)!=0)
  {
    flags.show_tag = TRUE;
    flags.no_tag_prompt = TRUE;
  }
  else if(check_option(count,argc,arg1,arg2,"-striptag",&option,0)!=0)
    toggle_flag((short*)&flags.strip_tag);
  else if(check_option(count,argc,arg1,arg2,"-tag",&option,0)!=0)
    toggle_flag((short*)&flags.force_tag);
  else if(check_option(count,argc,arg1,arg2,"-tagonly",&option,0)!=0)
    toggle_flag((short*)&flags.tag_only);
  else if(check_option(count,argc,arg1,arg2,"-tagfromfilename",&option,0)!=0)
    toggle_flag((short*)&flags.tag_ffn);
  else if(check_option(count,argc,arg1,arg2,"-tagffn",&option,0)!=0)
    toggle_flag((short*)&flags.tag_ffn);
  else if(check_option(count,argc,arg1,arg2,"-edit",&option,0)!=0)
    toggle_flag((short*)&flags.edit_tag);
  else if(check_option(count,argc,arg1,arg2,"-defcase",&option,0)!=0)
    flags.ulcase = 0;
  else if(check_option(count,argc,arg1,arg2,"-upper",&option,0)!=0)
    flags.ulcase = 1;
  else if(check_option(count,argc,arg1,arg2,"-lower",&option,0)!=0)
    flags.ulcase = 2;
  else if(check_option(count,argc,arg1,arg2,"-space",&option,1)!=0)
  {
    if (option == NULL)
      strcpy(replace_spacechar, "");
    else
      strncpy(replace_spacechar, option, (sizeof(replace_spacechar)-1));
  }
  else if(check_option(count,argc,arg1,arg2,"-remchar",&option,1)!=0)
  {
    alloc_string(&remove_char, (strlen(option)+1));
    strcpy(remove_char, option);
  }
  else if(check_option(count,argc,arg1,arg2,"-repchar",&option,1)!=0)
  {
    if ( (strlen(option) % 2) != 0 )
    {
      user_message(TRUE, "%s: Replace characters must be in pairs\n", program_name);
      exit(ERRLEV_ARGS);
    }

    alloc_string(&replace_char, (strlen(option)+1));
    strcpy(replace_char, option);
  }
  else if(check_option(count,argc,arg1,arg2,"-template",&option,1)!=0)
  {
    if (option == NULL)
    {
      user_message(TRUE, "%s: Empty template specified (%s)\n", program_name, arg1);
      exit(ERRLEV_ARGS);
    }
    else
      strncpy(filename_template, option, (sizeof(filename_template)-1));
  }  
  else if(check_option(count,argc,arg1,arg2,"-tagtemplate",&option,1)!=0)
  {
    if (option == NULL)
    {
      user_message(TRUE, "%s: Empty tagtemplate specified (%s)\n", program_name, arg1);
      exit(ERRLEV_ARGS);
    }
    else
      strncpy(tag_template, option, (sizeof(tag_template)-1));
  }
  else if(check_option(count,argc,arg1,arg2,"-copytagfrom",&option,1)!=0)
  {
    if (option == NULL)
    {
      user_message(TRUE, "%s: Empty file name to copy from specified (%s)\n", program_name, arg1);
      exit(ERRLEV_ARGS);
    }
    else
     {
      if(id3_open_file(&copyfp,option,"rb")==FALSE)
       {
        user_message(TRUE, "%s: Can not copy from file (%s)\n", program_name, arg1);
        exit(ERRLEV_ARGS);
       }

      memset(&copytag,0,sizeof(ID3_tag));
      if(id3_seek_header(copyfp, option) != FALSE) 
       id3_read_file(copytag.tag, (sizeof(copytag.tag)-1), copyfp, option);
      if (strcmp(copytag.tag, "TAG") != 0)
       {
        user_message(TRUE, "%s: Can not find any tag in file (%s)\n", program_name, arg1);
        id3_close_file(copyfp);
        exit(ERRLEV_ARGS);
       }
      id3_read_tag(&copytag,copyfp,option);
      id3_close_file(copyfp);
     }
  }
  else if(check_option(count,argc,arg1,arg2,"-copysong",&option,0)!=0)
  {
    if(copyfp==NULL)
     {user_message(TRUE, "%s: No copytagfrom file specified\n", program_name);
      exit(ERRLEV_ARGS);
     }
    
    alloc_string(&def_song, strlen(copytag.songname)+1);
    strcpy(def_song, copytag.songname);
  }
  else if(check_option(count,argc,arg1,arg2,"-copyartist",&option,0)!=0)
  {
    if(copyfp==NULL)
     {user_message(TRUE, "%s: No copytagfrom file specified\n", program_name);
      exit(ERRLEV_ARGS);
     }
    
    alloc_string(&def_artist, (strlen(copytag.artist)+1));
    strcpy(def_artist, copytag.artist);
  }
  else if(check_option(count,argc,arg1,arg2,"-copyalbum",&option,0)!=0)
  {
    if(copyfp==NULL)
     {user_message(TRUE, "%s: No copytagfrom file specified\n", program_name);
      exit(ERRLEV_ARGS);
     }
    
    alloc_string(&def_album, (strlen(copytag.album)+1));
    strcpy(def_album, copytag.album);
  }
  else if(check_option(count,argc,arg1,arg2,"-copyyear",&option,0)!=0)
  {
    if(copyfp==NULL)
     {user_message(TRUE, "%s: No copytagfrom file specified\n", program_name);
      exit(ERRLEV_ARGS);
     }
    
    alloc_string(&def_year, (strlen(copytag.year)+1));
    strcpy(def_year, copytag.year);
  }
  else if(check_option(count,argc,arg1,arg2,"-copytrack",&option,0)!=0)
  {
    if(copyfp==NULL)
     {user_message(TRUE, "%s: No copytagfrom file specified\n", program_name);
      exit(ERRLEV_ARGS);
     }
    if(copytag.version==0)
     {user_message(TRUE, "%s: Copied tag is not id3v1.1\n", program_name);
      exit(ERRLEV_ARGS);
     }
    
    def_track=copytag.u.v11.track;
  }
  else if(check_option(count,argc,arg1,arg2,"-copycomment",&option,0)!=0)
  {
    if(copyfp==NULL)
     {user_message(TRUE, "%s: No copytagfrom file specified\n", program_name);
      exit(ERRLEV_ARGS);
     }
    
    alloc_string(&def_comment, (strlen(copytag.u.v10.comment)+1));
    strcpy(def_comment, copytag.u.v10.comment);
  }
  else if(check_option(count,argc,arg1,arg2,"-copygenre",&option,0)!=0)
  {
    if(copyfp==NULL)
     {user_message(TRUE, "%s: No copytagfrom file specified\n", program_name);
      exit(ERRLEV_ARGS);
     }
    
    def_genre=copytag.genre;
  }
  else if(check_option(count,argc,arg1,arg2,"-copyall",&option,0)!=0)
  {
    if(copyfp==NULL)
     {user_message(TRUE, "%s: No copytagfrom file specified\n", program_name);
      exit(ERRLEV_ARGS);
     }
    
    alloc_string(&def_song, strlen(copytag.songname)+1);
    strcpy(def_song, copytag.songname);
    alloc_string(&def_artist, (strlen(copytag.artist)+1));
    strcpy(def_artist, copytag.artist);
    alloc_string(&def_album, (strlen(copytag.album)+1));
    strcpy(def_album, copytag.album);
    alloc_string(&def_year, (strlen(copytag.year)+1));
    strcpy(def_year, copytag.year);
    if(copytag.version>0)def_track=copytag.u.v11.track;
    alloc_string(&def_comment, (strlen(copytag.u.v10.comment)+1));
    strcpy(def_comment, copytag.u.v10.comment);
    def_genre=copytag.genre;
  }
  else
  {
    user_message(TRUE, "%s: Unknown option: %s\n", program_name, arg1);
    exit(ERRLEV_ARGS);
  }
}


int
check_args (int argc, char *argv[])
{
  int i;

  for (i = 1; i < argc && argv[i][0] == '-'; i++)
  {
    check_arg (&i, argc, argv[i], (((i+1) < argc) ? argv[i+1] : ""));
  }

  if (flags.no_config == FALSE)
  {
    if (!read_config(getenv("ID3REN"), ""))
      if (!read_config(getenv("HOME"), CONFIG_HOME))
  #ifdef __WIN32__
        read_config(program_path, CONFIG_GLOBAL);
  #else
        read_config("/etc", CONFIG_GLOBAL);
  #endif
  }

  return i;
}


int
main (int argc, char *argv[])
{
  char arg[256];
  char *p;
  int i;
  int count = 0;
  int errcount = 0;
  char tempname[512],*d,*s;
  int okflag;
#ifdef WIN32_FIND
  WIN32_FIND_DATA *lpwfd;
  HANDLE hSearch;
#endif

  atexit(exit_function);
  p = strrchr(argv[0], PATH_CHAR);

  if (p == NULL)
  {
    p = argv[0];
    alloc_string(&program_path, 2);
    strcpy(program_path, ".");
  }
  else
  {
    p++;
    alloc_string(&program_path, (strlen(argv[0]) - strlen(p)));
    strncpy(program_path, argv[0], ((strlen(argv[0]) - strlen(p)) - 1));
  }

  alloc_string(&program_name, (strlen(p)+1));
  strcpy(program_name, p);

  if (argc <= 1)
  {
    show_usage(argv[0]);
    return ERRLEV_ARGS;
  }

  if ( (ptrtag = (ID3_tag*)malloc(sizeof(*ptrtag)) ) == NULL)
  {
    print_error("Out of memory for malloc");
    exit(ERRLEV_MALLOC);
  }

#ifdef WIN32_FIND
  if ( (lpwfd = (WIN32_FIND_DATA*)malloc(sizeof(*lpwfd)) ) == NULL)
  {
    print_error("Out of memory for malloc");
    exit(ERRLEV_MALLOC);
  }
#endif  

  i = check_args (argc, argv);

  if (i >= argc)
  {
    user_message(TRUE, "%s: Not enough arguments specified\n", program_name);
    exit(ERRLEV_ARGS);
  }


  for (; i < argc; i++)
  {
    count++;

#ifdef WIN32_FIND
    hSearch = FindFirstFile(argv[i], lpwfd);

    if (hSearch != INVALID_HANDLE_VALUE)
    {
      do
      {
        strncpy(arg, lpwfd->cFileName, (sizeof(arg)-1));

        if (tag_file(arg))
        {
#else
        strncpy(arg, argv[i], (sizeof(arg)-1));

        if (access(arg, F_OK) != 0)
        {
          user_message(TRUE, "%s: File not found: %s\n",
            program_name, arg);
        }
        else if (tag_file(arg))
        {
#endif
          if (flags.show_tag)
          {
            id3_show_tag(ptrtag, arg);
          }
          else if (!flags.tag_only && !flags.strip_tag)
          {
            apply_template(arg);

            if (access(applied_filename, F_OK) == 0)
            {
              user_message(TRUE, "%s: File already exists: %s\n",
                program_name, applied_filename);
            }
            else  
            {
              // We create the dirs if needed.
              okflag=1;
              strcpy(tempname,applied_filename);
              s=tempname;
              while(okflag==1)
              {
                d=strchr(s,PATH_CHAR);
                if(d==NULL)break;
                // Set to 0 
                *d=0;
                if(strcmp(tempname,"")!=0)
                 if(mkdir(tempname,0777)!=0)
                  {
                   if(errno!=EEXIST)
                    {
                     print_error("Create dir failed on %s", tempname);
                     okflag=0;
                    }
                  }
                // Reset to what it was
                *d=PATH_CHAR;
                s=d+1;
              }
             
             if(okflag==1)
              {
               if (rename(arg, applied_filename) == 0)
                 user_message(FALSE, "%-38s => %-37s\n", arg, applied_filename);
               else
                 print_error("Rename failed on %s", arg);
              }
             else
              {
               count--;
               errcount++;
              }
            }
          }
        }
        else
        {
          count--;
          errcount++;
        }

#ifdef WIN32_FIND
      } while (FindNextFile(hSearch, lpwfd)); /* End do */
    } /* end of FindFirstFile */
    else
    {
      user_message(TRUE, "%s: File not found: %s\n", program_name, argv[i]);
    }
#endif
  } /* end for loop */


  user_message(FALSE, "Processed: %d  Failed: %d  Total: %d\n", count, errcount, count+errcount);
  return ERRLEV_SUCCESS;
} /* end main */

