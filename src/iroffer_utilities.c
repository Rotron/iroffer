/*
iroffer - An IRC file server using the DCC protocol
Copyright (C) see CONTRIBUTORS

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* include the headers */
#include "iroffer_config.h"
#include "iroffer_defines.h"
#include "iroffer_headers.h"
#include "iroffer_globals.h"


const char* strstrnocase (const char *str1, const char *match1)
{
  char *str, *match;
  const char *retval;
  
  str   = mymalloc(strlen(str1)+1);
  match = mymalloc(strlen(match1)+1);
  
  strcpy(str,str1);
  caps(str);
  strcpy(match,match1);
  caps(match);
  
  retval = strstr(str, match);
  
  if (retval)
    {
      retval = str1 + (retval - str);
    }
  
  mydelete(str);
  mydelete(match);
  
  return retval;
}

char* getpart2(const char *line, int howmany,
               const char *src_function, const char *src_file, int src_line)
{
  char *part;
  int li;
  int plen;
  int hi;
  
  li=0;
  
  for (hi = 1; hi < howmany; hi++)
    {
      while (line[li] != ' ')
        {
          if (line[li] == '\0')
            {
              return NULL;
            }
          else
            {
              li++;
            }
        }
      li++;
    }
  
  if (line[li] == '\0')
    {
      return NULL;
    }
  
  for (plen=0; (line[li] != ' ') && (line[li] != '\0'); plen++, li++) ;
  
  li -= plen;
  
  part = mymalloc2(plen+1, 0, src_function, src_file, src_line);
  
  memcpy(part, line+li, plen);
  part[plen] = '\0';
  
  return part;
}

char* caps(char *text) {
   int i;
   if (text)
      for (i=0; i<sstrlen(text); i++)
          if ( text[i] >= 'a' && text[i] <= 'z' )
             text[i] = text[i]-32;
   return text;
   }

char* nocaps(char *text) {
   int i;
   if (text)
      for (i=0; i<sstrlen(text); i++)
          if ( text[i] >= 'A' && text[i] <= 'Z' )
             text[i] = text[i]+32;
   return text;
   }


char* sizestr(int spaces, off_t num)
{
#define SIZESTR_SIZE 5
  char *str = mycalloc(SIZESTR_SIZE);
  
  if (num >= 1024*1024*1024*1000ULL)
    {
      /* >1000GB */
      snprintf(str, SIZESTR_SIZE,
               "%2.1fT",
               (((float)num)/(1024.0*1024.0*1024.0*1024.0)));
    }
  else if (num >= 1024*1024*1024*10ULL)
    {
      /* >10GB */
      snprintf(str, SIZESTR_SIZE,
               "%3.0fG",
               (((float)num)/(1024.0*1024.0*1024.0)));
    }
  else if (num >= 1024*1024*1000)
    {
      /* >1000MB */
      snprintf(str, SIZESTR_SIZE,
               "%2.1fG",
               (((float)num)/(1024.0*1024.0*1024.0)));
    }
  else if (num >= 1024*1024*10)
    {
      /* >10MB */
      snprintf(str, SIZESTR_SIZE,
               spaces ? "%3.0fM" : "%.0fM",
               (((float)num)/(1024.0*1024.0)));
    }
  else if (num >= 1024*1000)
    {
      /* >1000KB */
      snprintf(str, SIZESTR_SIZE,
               "%2.1fM",
               (((float)num)/(1024.0*1024.0)));
    }
  else if (num >= 1024)
    {
      /* >1KB */
      snprintf(str, SIZESTR_SIZE,
               spaces ? "%3.0fK" : "%.0fK",
               (((float)num)/1024.0));
    }
  else if (num >= 1)
    {
      /* <1KB */
      snprintf(str, SIZESTR_SIZE,
               spaces ? " <1K" : "<1K");
    }
  else
    {
      snprintf(str,SIZESTR_SIZE,
               spaces ? "   0" : "0");
    }
  
  return str;
}

void getos (void) {

   struct utsname u1;

   updatecontext();

   if ( uname(&u1) < 0)
      outerror(OUTERROR_TYPE_CRASH,"Couldn't Get Your OS Type");
   
   printf("** You Are Running %s %s on a %s",u1.sysname,u1.release,u1.machine);
   mylog(CALLTYPE_NORMAL,"You Are Running %s %s on a %s",u1.sysname,u1.release,u1.machine);
   
   gdata.osstring = mymalloc(strlen(u1.sysname) + strlen(u1.release) + 2);
   
   sprintf(gdata.osstring, "%s %s",
           u1.sysname, u1.release);
   
   /* verify we are who we were configured for, and set config */
#if defined(_OS_Linux)
   if (strcmp(u1.sysname,"Linux"))
      outerror(OUTERROR_TYPE_WARN_LOUD,"Configured for Linux but not running Linux?!?");
   printf(", Good\n");
   
#elif defined(_OS_FreeBSD)   || \
    defined(_OS_OpenBSD)     || \
    defined(_OS_NetBSD)      || \
    defined(_OS_BSDI)        || \
    defined(_OS_BSD_OS)
   if (strcmp(u1.sysname,"FreeBSD") && strcmp(u1.sysname,"OpenBSD") && strcmp(u1.sysname,"BSD/OS") && strcmp(u1.sysname,"NetBSD"))
      outerror(OUTERROR_TYPE_WARN_LOUD,"Configured for *BSD but not running *BSD?!?");
   printf(", Good\n");

#elif defined(_OS_SunOS)
   if (strcmp(u1.sysname,"SunOS"))
      outerror(OUTERROR_TYPE_WARN_LOUD,"Configured for Solaris but not running Solaris?!?");
   printf(", Good\n");

#elif defined(_OS_HPUX)
   if (strcmp(u1.sysname,"HP-UX"))
      outerror(OUTERROR_TYPE_WARN_LOUD,"Configured for HP-UX but not running HP-UX?!?");
   printf(", Good\n");
   
#elif defined(_OS_IRIX)
   if (strcmp(u1.sysname,"IRIX"))
      outerror(OUTERROR_TYPE_WARN_LOUD,"Configured for IRIX but not running IRIX?!?");
   printf(", Good\n");

#elif defined(_OS_IRIX64)
   if (strcmp(u1.sysname,"IRIX64"))
      outerror(OUTERROR_TYPE_WARN_LOUD,"Configured for IRIX64 but not running IRIX64?!?");
   printf(", Good\n");

#elif defined(_OS_OSF1)
   if (strcmp(u1.sysname,"OSF1"))
      outerror(OUTERROR_TYPE_WARN_LOUD,"Configured for OSF1 but not running OSF1?!?");
   printf(", Good\n");

#elif defined(_OS_Rhapsody)
   if (strcmp(u1.sysname,"Rhapsody"))
      outerror(OUTERROR_TYPE_WARN_LOUD,"Configured for Rhapsody but not running Rhapsody?!?");
   printf(", Good\n");

#elif defined(_OS_Darwin)
   if (strcmp(u1.sysname,"Darwin"))
      outerror(OUTERROR_TYPE_WARN_LOUD,"Configured for Darwin but not running Darwin?!?");
   printf(", Good\n");

#elif defined(_OS_AIX)
   if (strcmp(u1.sysname,"AIX"))
      outerror(OUTERROR_TYPE_WARN_LOUD,"Configured for AIX but not running AIX?!?");
   printf(", Good\n");

#elif defined(_OS_CYGWIN)
   if (strncmp(u1.sysname,"CYGWIN",6))
     {
       outerror(OUTERROR_TYPE_WARN_LOUD,"Configured for CYGWIN but not running CYGWIN?!?");
     }
   {
     int count,v1=0,v2=0,v3=0;
     count = sscanf(u1.release,"%d.%d.%d",&v1,&v2,&v3);
     if (
	 (v1 < 1) ||
	 ((v1 == 1) && (v2 < 5)) ||
	 ((v1 == 1) && (v2 == 5) && (v3 < 3))
	 )
       {
	 printf(", Too Old\n");
	 outerror(OUTERROR_TYPE_CRASH,"CYGWIN too old (1.5.3 or greater required)");
       }
   }
   printf(", Good\n");

#else
   printf(", I don't know of that!\n");

#endif

   
   }

void outerror (outerror_type_e type, const char *format, ...) {
   char tempstr[maxtextlength];
   va_list args;
   int ioutput_options = OUT_S|OUT_L|OUT_D;
   int len;

   /* can't log an error if the error was due to logging */
   if (type & OUTERROR_TYPE_NOLOG)
     {
       ioutput_options &= ~OUTERROR_TYPE_NOLOG;
     }
   type &= ~OUTERROR_TYPE_NOLOG;
   
   updatecontext();

   va_start(args, format);
   len = vsnprintf(tempstr, maxtextlength, format, args);
   va_end(args);
   
  if ((len < 0) || (len >= maxtextlength))
    {
      snprintf(tempstr, maxtextlength-1, "OUTERROR-INT: Output too large, ignoring!");
    }

   if (!gdata.background && !gdata.attop) gototop();
   
   if ( type == OUTERROR_TYPE_CRASH ) {
      
      ioutput(CALLTYPE_NORMAL,ioutput_options,COLOR_RED|COLOR_BOLD,"ERROR: %s",tempstr);
      
      tostdout_disable_buffering(1);
      
      uninitscreen();
      
      if (gdata.background)
        {
          printf("** ERROR: %s\n\n",tempstr);
        }
         
      mylog(CALLTYPE_NORMAL,"iroffer exited (Error)\n\n");
      if (gdata.pidfile) unlink(gdata.pidfile);

      exit(1);
      }
   else if (type == OUTERROR_TYPE_WARN_LOUD ) {
      ioutput(CALLTYPE_NORMAL,ioutput_options,COLOR_RED|COLOR_BOLD,"WARNING: %s",tempstr);
      }
   else if (type == OUTERROR_TYPE_WARN ) {
      ioutput(CALLTYPE_NORMAL,ioutput_options,COLOR_RED,"WARNING: %s",tempstr);
      }
   
   }

char* getdatestr(char* str, time_t Tp, int len)
{
  struct tm *localt = NULL;
  size_t llen;
    
  if (Tp == 0)
    {
      Tp = gdata.curtime;
    }
  localt = localtime(&Tp);
  
  llen = strftime (str, len, "%Y-%m-%d-%H:%M:%S", localt);
  if ((llen == 0) || (llen == len))
    {
      str[0] = '\0';
    }
  
  return str;
}

char* getuptime(char *str, int type, time_t fromwhen, int len)
{
  int days, hours, mins;
  long temp;
  int llen;
  
  updatecontext();
  
  temp  = (gdata.curtime-fromwhen)/60;
  days  = temp/60/24;
  hours = temp/60 - (24*days);
  mins  = temp - (60*hours) - (24*60*days);
  
  if (type)
    {
      llen = snprintf(str, len, "%dD %dH %dM",
                      days, hours, mins);
    }
  else
    {
      llen = snprintf(str, len, "%d Days %d Hrs and %d Min",
                      days, hours, mins);
    }
  
  if ((llen < 0) || (llen >= len))
    {
      str[0] = '\0';
    }
  return str;
}


void mylog(calltype_e type, const char *format, ...)
{
  char tempstr[maxtextlength];
  va_list args;
  int len;
  
  if (gdata.logfile == NULL)
    {
      return;
    }
  
  if (gdata.logfd == FD_UNUSED)
    {
      gdata.logfd = open(gdata.logfile,
                         O_WRONLY | O_CREAT | O_APPEND | ADDED_OPEN_FLAGS,
                         CREAT_PERMISSIONS);
      if (gdata.logfd < 0)
        {
          outerror(OUTERROR_TYPE_WARN_LOUD | OUTERROR_TYPE_NOLOG,
                   "Cant Access Log File '%s': %s",
                   gdata.logfile,strerror(errno));
          gdata.logfd = FD_UNUSED;
          return;
        }
    }
  
  if ((type == CALLTYPE_NORMAL) || (type == CALLTYPE_MULTI_FIRST))
    {
      write(gdata.logfd,"** ",3);
      getdatestr(tempstr,0,maxtextlength);
      write(gdata.logfd,tempstr,strlen(tempstr));
      write(gdata.logfd,": ",2);
    }
  
  va_start(args, format);
  len = vsnprintf(tempstr, maxtextlength, format, args);
  va_end(args);
  
  if ((len < 0) || (len >= maxtextlength))
    {
      outerror(OUTERROR_TYPE_WARN_LOUD | OUTERROR_TYPE_NOLOG,
               "MYLOG-INT: Output too large, ignoring!");
      return;
    }
  
  write(gdata.logfd,tempstr,strlen(tempstr));
  
  if ((type == CALLTYPE_NORMAL) || (type == CALLTYPE_MULTI_END))
    {
      write(gdata.logfd,"\n",1);
    }
  
  return;
}


unsigned long atoul (const char *str) {
   unsigned long num,temp;
   int i,j;
   if (str == NULL) return 0;
   
   num = 0;
   
   for (i=strlen(str)-1; i>=0; i--) {
      temp = (str[i]-'0');
      for (j=strlen(str)-1; j>i; j--)
         temp *= 10;
      num += temp;
      }
   return num;
   }

unsigned long long atoull (const char *str) {
   unsigned long long num,temp;
   int i,j;
   if (str == NULL) return 0;
   
   num = 0;
   
   for (i=strlen(str)-1; i>=0; i--) {
      temp = (str[i]-'0');
      for (j=strlen(str)-1; j>i; j--)
         temp *= 10;
      num += temp;
      }
   return num;
   }

void ioutput(calltype_e type, int dest, unsigned int color_flags, const char *format, ...) {
   va_list args;
   va_start(args, format);
   vioutput(type, dest, color_flags, format, args);
   va_end(args);
}

void vioutput(calltype_e type, int dest, unsigned int color_flags, const char *format, va_list ap) {
   char tempstr[maxtextlength];
   int len;
   dccchat_t *chat;
   
   len = vsnprintf(tempstr,maxtextlength,format,ap);
   
   if ((len < 0) || (len >= maxtextlength))
     {
       snprintf(tempstr, maxtextlength-1, "IOUTPUT-INT: Output too large, ignoring!");
     }

   /* screen */
   if (!gdata.background && (dest & OUT_S)) {
      
      if (!gdata.attop) gototop();
      
      if ((type == CALLTYPE_NORMAL) || (type == CALLTYPE_MULTI_FIRST))
        {
          if (!gdata.nocolor && (color_flags != COLOR_NO_COLOR))
            {
              tostdout("\x1b[%d;%dm",
                       (color_flags & COLOR_BOLD) ? 1 : 0,
                       (color_flags & ~COLOR_BOLD));
            }
          if (gdata.timestampconsole)
            {
              char tempstr2[maxtextlength];
              getdatestr(tempstr2,0,maxtextlength);
              tostdout("** %s: ",tempstr2);
            }
          else
            {
              tostdout("** ");
            }
        }
      
      tostdout("%s",tempstr);
      
      if ((type == CALLTYPE_NORMAL) || (type == CALLTYPE_MULTI_END))
        {
          if (!gdata.nocolor && (color_flags != COLOR_NO_COLOR))
            {
              tostdout("\x1b[0m\n");
            }
          else
            {
              tostdout("\n");
            }
        }
      }
   
   /* log */
   if (dest & OUT_L)
      mylog(type,"%s",tempstr);
   
   /* dcc chat */
   if (dest & OUT_D)
     {
       for (chat = irlist_get_head(&gdata.dccchats);
            chat;
            chat = irlist_get_next(chat))
         {
           if (chat->status == DCCCHAT_CONNECTED)
             {
               if ((type == CALLTYPE_NORMAL) || (type == CALLTYPE_MULTI_FIRST))
                 {
                   writedccchat(chat, 0, "--> ");
                 }
               
               writedccchat(chat, 0, "%s", tempstr);
               
               if ((type == CALLTYPE_NORMAL) || (type == CALLTYPE_MULTI_END))
                 {
                   writedccchat(chat, 0, "\n");
                 }
             }
         }
      }
   
   }

void privmsg_slow(const char *nick, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vprivmsg_slow(nick, format, args);
  va_end(args);
}

void vprivmsg_slow(const char *nick, const char *format, va_list ap)
{
  char tempstr[maxtextlength];
  int len;
  
  if (!nick) return;
  
  len = vsnprintf(tempstr,maxtextlength,format,ap);
  
  if ((len < 0) || (len >= maxtextlength))
    {
      outerror(OUTERROR_TYPE_WARN,"PRVMSG-SLOW: Output too large, ignoring!");
      return;
    }
  
  writeserver(WRITESERVER_SLOW, "PRIVMSG %s :%s", nick, tempstr);
}

void privmsg_fast(const char *nick, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vprivmsg_fast(nick, format, args);
  va_end(args);
}

void vprivmsg_fast(const char *nick, const char *format, va_list ap)
{
  char tempstr[maxtextlength];
  int len;
  
  if (!nick) return;
  
  len = vsnprintf(tempstr,maxtextlength,format,ap);
  
  if ((len < 0) || (len >= maxtextlength))
    {
      outerror(OUTERROR_TYPE_WARN,"PRVMSG-FAST: Output too large, ignoring!");
      return;
    }
  
  writeserver(WRITESERVER_FAST, "PRIVMSG %s :%s", nick, tempstr);
}

void privmsg(const char *nick, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vprivmsg(nick, format, args);
  va_end(args);
}

void vprivmsg(const char *nick, const char *format, va_list ap)
{
  char tempstr[maxtextlength];
  int len;
  
  if (!nick) return;
  
  len = vsnprintf(tempstr,maxtextlength,format,ap);
  
  if ((len < 0) || (len >= maxtextlength))
    {
      outerror(OUTERROR_TYPE_WARN,"PRVMSG: Output too large, ignoring!");
      return;
    }
  
  writeserver(WRITESERVER_NORMAL, "PRIVMSG %s :%s", nick, tempstr);
}

void notice_slow(const char *nick, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vnotice_slow(nick, format, args);
  va_end(args);
}

void vnotice_slow(const char *nick, const char *format, va_list ap)
{
  char tempstr[maxtextlength];
  int len;
  
  if (!nick) return;
  
  len = vsnprintf(tempstr,maxtextlength,format,ap);
  
  if ((len < 0) || (len >= maxtextlength))
    {
      outerror(OUTERROR_TYPE_WARN,"NOTICE-SLOW: Output too large, ignoring!");
      return;
    }
  
  writeserver(WRITESERVER_SLOW, "NOTICE %s :%s", nick, tempstr);
}

void notice_fast(const char *nick, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vnotice_fast(nick, format, args);
  va_end(args);
}

void vnotice_fast(const char *nick, const char *format, va_list ap)
{
  char tempstr[maxtextlength];
  int len;
  
  if (!nick) return;
  
  len = vsnprintf(tempstr,maxtextlength,format,ap);
  
  if ((len < 0) || (len >= maxtextlength))
    {
      outerror(OUTERROR_TYPE_WARN,"NOTICE-FAST: Output too large, ignoring!");
      return;
    }
  
  writeserver(WRITESERVER_FAST, "NOTICE %s :%s", nick, tempstr);
}

void notice(const char *nick, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vnotice(nick, format, args);
  va_end(args);
}

void vnotice(const char *nick, const char *format, va_list ap)
{
  char tempstr[maxtextlength];
  int len;
  
  if (!nick) return;
  
  len = vsnprintf(tempstr,maxtextlength,format,ap);
  
  if ((len < 0) || (len >= maxtextlength))
    {
      outerror(OUTERROR_TYPE_WARN,"NOTICE: Output too large, ignoring!");
      return;
    }
  
  writeserver(WRITESERVER_NORMAL, "NOTICE %s :%s", nick, tempstr);
}

char* hostmasktoregex(const char *str) {
   char *tempstr;
   int i,j;
   int maxlen;
   
   updatecontext();

   if (!str) return NULL;
   
   maxlen = 2 + (strlen(str) * 11);
   
   tempstr = mycalloc(maxlen+1);
   
   tempstr[0] = '^';
   
   for (i=0,j=1; i<sstrlen(str); i++,j++) {
      if ( (str[i] >= 'a' && str[i] <= 'z')
        || (str[i] >= 'A' && str[i] <= 'Z')
        || (str[i] >= '0' && str[i] <= '9') )
         tempstr[j] = str[i];
      else if (str[i] == '?')
         tempstr[j] = '.';
      else if (str[i] == '*') {
         tempstr[j] = '.'; j++;
         tempstr[j] = '*';
         }
      else if (str[i] == '#') {
         tempstr[j] = '['; j++;
         tempstr[j] = '0'; j++;
         tempstr[j] = '-'; j++;
         tempstr[j] = '9'; j++;
         tempstr[j] = ']'; j++;
         tempstr[j] = '['; j++;
         tempstr[j] = '0'; j++;
         tempstr[j] = '-'; j++;
         tempstr[j] = '9'; j++;
         tempstr[j] = ']'; j++;
         tempstr[j] = '*';
         }
      else {
         tempstr[j] = '\\'; j++;
         tempstr[j] = str[i];
         }
      }
   
   tempstr[j] = '$';
   tempstr[j+1] = '\0';
   
   return tempstr;
   
   }

int verifyhost(irlist_t *list, const char *hmask)
{
  regex_t *ah;
  
  updatecontext();
  
  ah = irlist_get_head(list);
  while (ah)
    {
    if (!regexec(ah,hmask,0,NULL,0))
      {
        return 1;
      }
    ah = irlist_get_next(ah);
    }
  
  return 0;
}

int verifypass(const char *testpass) {
   
#ifndef NO_CRYPT
   char *pwout;

   updatecontext();

   if ( !gdata.adminpass || !testpass )
     return 0;
   
   if (
     strlen(gdata.adminpass) != 13
     || strlen(testpass) < 5
     || strlen(testpass) > 8
     )
     return 0;
   
   pwout = crypt(testpass,gdata.adminpass);
   
   if (strcmp(pwout,gdata.adminpass)) return 0;
#else
   if ( !gdata.adminpass || !testpass )
     return 0;
   
   if (strcmp(testpass,gdata.adminpass)) return 0;
#endif
   
   return 1;
   
   }

char* getfline(char* str, int slen, int descr, int ret)
{
  char *tempbuf;
  int i, j, len;
  
  updatecontext();
  
  tempbuf = mycalloc(slen);
  
  j = 0;
  str[0] = '\0';
  
  while((len = read(descr,tempbuf,slen)) > 0)
    {
      for (i=0; i<len; i++)
        {
          if (tempbuf[i] == '\n' || tempbuf[i] == 13) /* 13 is ^M */
            {
              lseek(descr, (off_t)(1-(len-i)), SEEK_CUR);
              str[j] = '\0';
              j = 0;
              if (str[0] != '\0' )
                {
                  mydelete(tempbuf);
                  return str;
                }
              else if ( ret )
                {
                  mydelete(tempbuf);
                  return str;
                }
              else
                {
                  mydelete(tempbuf);
                  return NULL;
                }
            }
          else
            {
              if (j >= slen)
                {
                  outerror(OUTERROR_TYPE_WARN,"Line too long, truncating");
                }
              else
                {
                  str[j] = tempbuf[i];
                  j++;
                }
            }
        }
    }
  mydelete(tempbuf);
  return NULL;
}

int packnumtonum(const char *a) {
   
   if (!a) return 0;
   
   if (a[0] == '#') a++;
   
   return atoi(a);
   
   }


int sstrlen (const char *p) {
   if (!p) return -1;
   return ((int)(strlen(p)));
   }

char dayofweektomask(const char a) {
   switch (a) {
      case 'U':
         return 0x01;
      case 'M':
         return 0x02;
      case 'T':
         return 0x04;
      case 'W':
         return 0x08;
      case 'R':
         return 0x10;
      case 'F':
         return 0x20;
      case 'S':
         return 0x40;
      default:
         return 0x00;
      }
   return 0;
   }


/* reverse a string */
char *strrev(char *str) {
   int i,len;
   char c;
   
   len = sstrlen(str);
   for (i=0; i<len/2; i++) {
      c = str[i];
      str[i] = str[len-i];
      str[len-i] = c;
      }
   
   return str;
   }

int isprintable(char a) {
   if ( a >= 0x20 && a <= 0x7E )
      return 1;
   else
      return 0; 
   }

char onlyprintable(char a) {
   if ( a >= 0x20 && a <= 0x7E )
      return a;
   else
      return '.'; 
   }

static unsigned long alloc_hash(void *ptr)
{
  unsigned long retval;
  
  retval = 0xAA;
  retval ^= ((unsigned long)ptr >>  0) & 0xFF;
  retval ^= ((unsigned long)ptr >>  8) & 0xFF;
  retval ^= ((unsigned long)ptr >> 16) & 0xFF;
  retval ^= ((unsigned long)ptr >> 24) & 0xFF;
  
  return retval & (MEMINFOHASHSIZE-1);
}

static void meminfo_grow(int grow)
{
  meminfo_t *newmeminfo;
  int cc;
  int dd;
  int len;
  int i;
  int start;

  len = MEMINFOHASHSIZE * sizeof(meminfo_t) * (gdata.meminfo_depth+grow);
  newmeminfo = calloc(len,1);
  
  /* replace zero entry */
  if (gdata.meminfo)
    {
      gdata.meminfo[0].ptr       = NULL;
      gdata.meminfo[0].alloctime = 0;
      gdata.meminfo[0].size      = 0;
      gdata.meminfo[0].src_func  = NULL;
      gdata.meminfo[0].src_file  = NULL;
      gdata.meminfo[0].src_line  = 0;
    }
  else
    {
      /* first time, count item #0 */
      gdata.meminfo_count++;
    }
  
  newmeminfo[0].ptr          = newmeminfo;
  newmeminfo[0].alloctime    = gdata.curtime;
  newmeminfo[0].size         = len;
  newmeminfo[0].src_func     = __FUNCTION__;
  newmeminfo[0].src_file     = __FILE__;
  newmeminfo[0].src_line     = __LINE__;
  
  for (cc=0; cc<MEMINFOHASHSIZE; cc++)
    {
      for (dd=0; dd<gdata.meminfo_depth; dd++)
        {
          if (gdata.meminfo[(cc*(gdata.meminfo_depth)) + dd].ptr)
            {
              /* find new location */
              start = alloc_hash(gdata.meminfo[(cc*(gdata.meminfo_depth)) + dd].ptr) * (gdata.meminfo_depth+grow);
              
              for (i=0; newmeminfo[(i+start)%(MEMINFOHASHSIZE * (gdata.meminfo_depth+grow))].ptr; i++) ;
              
              i = (i+start)%(MEMINFOHASHSIZE * (gdata.meminfo_depth+grow));
              
              newmeminfo[i] = gdata.meminfo[(cc*(gdata.meminfo_depth)) + dd];
            }
        }
    }
  
  if (gdata.meminfo)
    {
      /* second or later time */
      free(gdata.meminfo);
    }
  
  gdata.meminfo = newmeminfo;
  gdata.meminfo_depth += grow;
  
  if (gdata.debug > 0)
    {
      ioutput(CALLTYPE_NORMAL,OUT_S,COLOR_NO_COLOR,"growing meminfo from %d to %d",
              gdata.meminfo_depth-grow, gdata.meminfo_depth);
    }

  return;
}

void* mymalloc2(int len, int zero,
                const char *src_function, const char *src_file, int src_line) {
   void *t = NULL;
   int i;
   unsigned long start;
   
   updatecontext();

   t = zero ? calloc(len,1) : malloc(len);
   
   if (t == NULL)
     {
       outerror(OUTERROR_TYPE_CRASH,"Couldn't Allocate Memory!!");
     }
   
   if (gdata.meminfo_count >= ((MEMINFOHASHSIZE * gdata.meminfo_depth) / 2))
     {
       meminfo_grow(gdata.meminfo_depth/3 + 1);
     }
   
   start = alloc_hash(t) * gdata.meminfo_depth;
   
   for (i=0; gdata.meminfo[(i+start)%(MEMINFOHASHSIZE * gdata.meminfo_depth)].ptr; i++) ;
   
   i = (i+start)%(MEMINFOHASHSIZE * gdata.meminfo_depth);
   
   gdata.meminfo[i].ptr       = t;
   gdata.meminfo[i].alloctime = gdata.curtime;
   gdata.meminfo[i].size      = len;
   gdata.meminfo[i].src_func  = src_function;
   gdata.meminfo[i].src_file  = src_file;
   gdata.meminfo[i].src_line  = src_line;
   
   gdata.meminfo_count++;
   
   return t;
   }

void mydelete2(void *t) {
   unsigned char *ut = (unsigned char *)t;
   int i;
   unsigned long start;
   
   updatecontext();

   if (t == NULL) return;
   
   start = alloc_hash(t) * gdata.meminfo_depth;
   
   for (i=0; (i<(MEMINFOHASHSIZE * gdata.meminfo_depth) && (gdata.meminfo[(i+start)%(MEMINFOHASHSIZE * gdata.meminfo_depth)].ptr != t)); i++) ;
   
   if (i == (MEMINFOHASHSIZE * gdata.meminfo_depth)) {
      outerror(OUTERROR_TYPE_WARN_LOUD,"Pointer 0x%8.8lX not found in meminfo database while trying to free!!",(long)t);
      outerror(OUTERROR_TYPE_WARN_LOUD,"Please report this error to PMG");
      for(i=0; i<(12*12); i+=12) {
         outerror(OUTERROR_TYPE_WARN_LOUD," : %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X = \"%c%c%c%c%c%c%c%c%c%c%c%c\"",
               ut[i+0], ut[i+1], ut[i+2], ut[i+3], ut[i+4], ut[i+5], ut[i+6], ut[i+7], ut[i+8], ut[i+9], ut[i+10], ut[i+11],
               onlyprintable(ut[i+0]), onlyprintable(ut[i+1]),
               onlyprintable(ut[i+2]), onlyprintable(ut[i+3]),
               onlyprintable(ut[i+4]), onlyprintable(ut[i+5]),
               onlyprintable(ut[i+6]), onlyprintable(ut[i+7]),
               onlyprintable(ut[i+8]), onlyprintable(ut[i+9]),
               onlyprintable(ut[i+10]), onlyprintable(ut[i+11]));
         }
      outerror(OUTERROR_TYPE_WARN_LOUD,"Aborting Program! (core file should be generated)");
      abort(); /* getting a core file will help greatly */
      }
   else {
      free(t);
      
      i = (i+start)%(MEMINFOHASHSIZE * gdata.meminfo_depth);
      
      gdata.meminfo[i].ptr       = NULL;
      gdata.meminfo[i].alloctime = 0;
      gdata.meminfo[i].size      = 0;
      gdata.meminfo[i].src_func  = NULL;
      gdata.meminfo[i].src_file  = NULL;
      gdata.meminfo[i].src_line  = 0;
      
      gdata.meminfo_count--;
      }
   
   if ((gdata.meminfo_depth > 1) &&
       (gdata.meminfo_count < ((MEMINFOHASHSIZE * gdata.meminfo_depth) / 8)))
     {
       meminfo_grow(-1);
     }
   
   return;
   }

char* removenonprintable(char *str1) {
   int i;
   unsigned char *str = (unsigned char*)str1;
   
   if (!str) return NULL;
   
   for (i=0; i<strlen(str); i++) {
      if (!((str[i] >= 0x20 && str[i] <= 0x7E) ||
            (str[i] >= 0xA1) ||
            str[i] == 0x01 ||  /* ctcp */
            str[i] == 0x02 ||  /* bold */
            str[i] == 0x03 ||  /* color */
            str[i] == 0x09 ||  /* tab */
            str[i] == 0x0A ||  /* return */
            str[i] == 0x0D ||  /* return */
            str[i] == 0x0F ||  /* end formatting */
            str[i] == 0x16 ||  /* inverse */
            str[i] == 0x1F ))   /* underline */
      str[i] = '.';
      
      }
   return str;
   }

char* removenonprintablectrl(char *str1) {
   int i;
   unsigned char *str = (unsigned char*)str1;
   if (!str) return NULL;
   
   for (i=0; i<strlen(str); i++) {
      if (!((str[i] >= 0x20 && str[i] <= 0x7E) ||
            (str[i] >= 0xA1)))
      str[i] = ' ';
      
      }
   return str;
   }

char* removenonprintablefile(char *str) {
   int i;
   char last='.';
   
   if (!str) return NULL;
   
   for (i=0; i<strlen(str); i++) {
      if (str[i] >= 0x7E) str[i] = '_';
      if (str[i] <  0x28) str[i] = '_';
      switch (str[i]) {
         case 0x2F:
         case 0x3A:
         case 0x3D:
         case 0x3F:
         case 0x40:
         case 0x5C:
         case 0x60:
         case 0x7C:
            str[i] = '_';
         }
      if (last == '.' && str[i] == '.') str[i] = '_';
      
      last = str[i];
      }
      
   return str;
   }


int doesfileexist(const char *f) {
   int fd;
   
   updatecontext();

   if (!f) return 0;
   
   if ((fd = open(f,O_RDONLY | O_CREAT | O_EXCL | ADDED_OPEN_FLAGS, CREAT_PERMISSIONS)) < 0 && errno == EEXIST)
      return 1;
   else if (fd < 0)
      return 0;
   
   close(fd);
   unlink(f);
   return 0;
   
   
   }


void checkadminpass(void) {
#ifndef NO_CRYPT
   int err=0,i;
   
   updatecontext();

   if (!gdata.adminpass || strlen(gdata.adminpass) != 13) err++;
   
   for (i=0; !err && i<13; i++) {
      if (!((gdata.adminpass[i] >= 'a' && gdata.adminpass[i] <= 'z') ||
            (gdata.adminpass[i] >= 'A' && gdata.adminpass[i] <= 'Z') ||
            (gdata.adminpass[i] >= '0' && gdata.adminpass[i] <= '9') ||
            (gdata.adminpass[i] == '.') ||
            (gdata.adminpass[i] == '/')))
         err++;
      }
   
   if (err) outerror(OUTERROR_TYPE_CRASH,"adminpass is not encrypted!");

#endif   
   }

void updatecontext_f(const char *file, const char *func, int line)
{
  context_t *c;
  
  if (gdata.crashing)
    {
      return;
    }
  
  gdata.context_cur_ptr++;
  if (gdata.context_cur_ptr > (2*MAXCONTEXTS))
    gdata.context_cur_ptr = gdata.context_cur_ptr % MAXCONTEXTS;
  
  c = &gdata.context_log[gdata.context_cur_ptr % MAXCONTEXTS];
  
  c->file = file;
  c->func = func;
  c->line = line;
  
  if (gdata.debug > 0)
    {
      gettimeofday(&c->tv, NULL);
    }
  else
    {
      c->tv.tv_sec  = 0;
      c->tv.tv_usec = 0;
    }
  
}

void dumpcontext(void)
{
  int i;
  context_t *c;
  
  for (i=0; i<MAXCONTEXTS; i++)
    {
      c = &gdata.context_log[(gdata.context_cur_ptr + 1 + i) % MAXCONTEXTS];
      
      ioutput(CALLTYPE_NORMAL,OUT_S|OUT_L|OUT_D,COLOR_NO_COLOR,
              "Trace %3i  %-20s %-16s:%5i  %lu.%06lu",
              i-MAXCONTEXTS+1,
              c->func ? c->func : "UNKNOWN",
              c->file ? c->file : "UNKNOWN",
              c->line,
              (unsigned long)c->tv.tv_sec,
              (unsigned long)c->tv.tv_usec);
    }
  return;
}


#define gdata_common CALLTYPE_NORMAL,OUT_S|OUT_L|OUT_D,COLOR_NO_COLOR
#define gdata_string(x) ((x) ? (x) : "<undef>")

#define gdata_print_number(format,name) \
    ioutput(gdata_common, "GDATA * " #name ": " format, gdata. name);

#define gdata_print_number_cast(format,name,type) \
    ioutput(gdata_common, "GDATA * " #name ": " format, (type) gdata. name);

#define gdata_print_string(name) \
    ioutput(gdata_common, "GDATA * " #name ": %s", gdata_string(gdata. name));

#define gdata_print_int(name)   gdata_print_number("%d", name)
#define gdata_print_uint(name)  gdata_print_number("%u", name)
#define gdata_print_long(name)  gdata_print_number("%ld", name)
#define gdata_print_ulong(name) gdata_print_number("%lu", name)
#define gdata_print_float(name) gdata_print_number("%.5f", name)


#define gdata_print_number_array(format,name) \
    { if (gdata. name [ii]) { ioutput(gdata_common, "GDATA * " #name "[%d]: " format, ii, gdata. name [ii]); } }

#define gdata_print_string_array(name) \
    { if (gdata. name [ii]) { ioutput(gdata_common, "GDATA * " #name "[%d]: %s", ii, gdata_string(gdata. name [ii])); } }

#define gdata_print_number_array_item(format,name,item) \
    { if (gdata. name [ii] . item) { ioutput(gdata_common, "GDATA * " #name "[%d]: " #item "=" format, ii, gdata. name [ii] . item); } }

#define gdata_print_number_array_item_cast(format,name,item,type) \
    { if (gdata. name [ii] . item) { ioutput(gdata_common, "GDATA * " #name "[%d]: " #item "=" format, ii, (type) gdata. name [ii] . item); } }

#define gdata_print_string_array_item(name,item) \
    { if (gdata. name [ii] . item) { ioutput(gdata_common, "GDATA * " #name "[%d]: " #item "=%s", ii, gdata_string(gdata. name [ii] . item)); } }

#define gdata_print_number_array_itemptr(format,name,item) \
    { if (gdata. name [ii] -> item) { ioutput(gdata_common, "GDATA * " #name "[%d]: " #item "=" format, ii, gdata. name [ii] -> item); } }

#define gdata_print_string_array_itemptr(name,item) \
    { if (gdata. name [ii] -> item) { ioutput(gdata_common, "GDATA * " #name "[%d]: " #item "=%s", ii, gdata_string(gdata. name [ii] -> item)); } }

#define gdata_print_int_array(name)   gdata_print_number_array("%d", name)
#define gdata_print_uint_array(name)  gdata_print_number_array("%u", name)
#define gdata_print_long_array(name)  gdata_print_number_array("%ld", name)
#define gdata_print_ulong_array(name) gdata_print_number_array("%lu", name)
#define gdata_print_float_array(name) gdata_print_number_array("%.5f", name)


#define gdata_irlist_iter_start(name, type) \
    { type *iter; ioutput(gdata_common, "GDATA * " #name ":"); for(iter=irlist_get_head(&gdata. name); iter; iter=irlist_get_next(iter)) { 

#define gdata_irlist_iter_end } }

#define gdata_iter_as_print_number(format) \
    ioutput(gdata_common, "  : " format, iter);

#define gdata_iter_as_print_number_cast(format,type) \
    ioutput(gdata_common, "  : " format, (type) iter);

#define gdata_iter_as_print_string \
    ioutput(gdata_common, "  : %s", gdata_string(iter));


#define gdata_iter_as_print_int(name)   gdata_iter_as_print_number("%d", name)
#define gdata_iter_as_print_uint(name)  gdata_iter_as_print_number("%u", name)
#define gdata_iter_as_print_long(name)  gdata_iter_as_print_number("%ld", name)
#define gdata_iter_as_print_ulong(name) gdata_iter_as_print_number("%lu", name)
#define gdata_iter_as_print_float(name) gdata_iter_as_print_number("%.5f", name)


#define gdata_iter_print_number(format,name) \
    ioutput(gdata_common, "  " #name ": " format, iter-> name);

#define gdata_iter_print_number_cast(format,name,type) \
    ioutput(gdata_common, "  " #name ": " format, (type) iter-> name);

#define gdata_iter_print_string(name) \
    ioutput(gdata_common, "  " #name ": %s", gdata_string(iter-> name));

#define gdata_iter_print_int(name)   gdata_iter_print_number("%d", name)
#define gdata_iter_print_uint(name)  gdata_iter_print_number("%u", name)
#define gdata_iter_print_long(name)  gdata_iter_print_number("%ld", name)
#define gdata_iter_print_ulong(name) gdata_iter_print_number("%lu", name)
#define gdata_iter_print_float(name) gdata_iter_print_number("%.5f", name)



void dumpgdata(void)
{
  int ii;
  
  ioutput(gdata_common,"GDATA DUMP BEGIN");
  
  gdata_print_int(transfermethod);
  ioutput(gdata_common,"GDATA * connectionmethod: how=%d host=%s port=%d passwd=%s vhost=%s",
          (int)gdata.connectionmethod.how,
          gdata_string(gdata.connectionmethod.host),
          (int)gdata.connectionmethod.port,
          gdata_string(gdata.connectionmethod.password),
          gdata_string(gdata.connectionmethod.vhost));
  
  for (ii=0; ii<MAXCONFIG; ii++)
    {
      gdata_print_string_array(configfile);
    }
  gdata_print_string(osstring);
  gdata_print_int(hideos);
  gdata_print_int(lognotices);
  gdata_print_int(logmessages);
  gdata_print_int(timestampconsole);
  gdata_print_long(startuptime);
  gdata_print_int(lowbdwth);
  gdata_print_int(background);
  gdata_print_number("0x%.8lX",ourip);
  gdata_print_int(usenatip);
  gdata_print_int(autosend.pack);
  gdata_print_string(autosend.word);
  gdata_print_string(autosend.message);
  gdata_print_number("0x%.8lX",local_vhost);
  gdata_print_int(logstats);
  gdata_print_string(logfile);
  gdata_print_number_cast("%d",logrotate,int);
  gdata_print_number_cast("%d",last_logrotate,int);
  gdata_print_string(headline);
  gdata_print_string(creditline);
  gdata_print_string(pidfile);

  gdata_irlist_iter_start(proxyinfo, char);
  gdata_iter_as_print_string;
  gdata_irlist_iter_end;

  gdata_print_int(tcprangestart);
  gdata_print_float(transferminspeed);
  gdata_print_float(transfermaxspeed);
  gdata_print_int(overallmaxspeed);
  gdata_print_int(overallmaxspeeddayspeed);
  gdata_print_int(maxb);
  gdata_print_int(overallmaxspeeddaytimestart);
  gdata_print_int(overallmaxspeeddaytimeend);
  gdata_print_int(overallmaxspeeddaydays);
  
  for (ii=0; ii<NUMBER_TRANSFERLIMITS; ii++)
    {
      gdata_print_number_array_item("%" LLPRINTFMT "u",transferlimits,limit);
      gdata_print_number_array_item("%" LLPRINTFMT "u",transferlimits,used);
      gdata_print_number_array_item_cast("%ld",transferlimits,ends,long int);
    }
  gdata_print_int(transferlimits_over);
  
  gdata_print_int(maxtransfersperperson);
  gdata_print_int(maxqueueditemsperperson);
  gdata_print_int(punishslowusers);
  gdata_print_int(nomd5sum);
  
  /* downloadhost */
  
  gdata_print_string(adminpass);
  
  /* adminhost */
  
  gdata_print_string(filedir);
  gdata_print_string(statefile);
  gdata_print_string(xdcclistfile);
  gdata_print_int(xdcclistfileraw);
  gdata_print_string(periodicmsg_nick);
  gdata_print_string(periodicmsg_msg);
  gdata_print_int(periodicmsg_time);
  /* autoignore_exclude */
  gdata_print_int(autoignore_threshold);
  /* uploadhost */
  gdata_print_string(uploaddir);
  gdata_print_number_cast("%" LLPRINTFMT "d",uploadmaxsize,long long);
  gdata_print_string(config_nick);
  gdata_print_string(user_nick);
  gdata_print_string(caps_nick);
  gdata_print_string(user_realname);
  gdata_print_string(user_modes);
  gdata_print_int(quietmode);
  gdata_print_string(loginname);
  gdata_print_int(restrictlist);
  gdata_print_int(restrictsend);
  gdata_print_int(restrictprivlist);
  gdata_print_string(restrictprivlistmsg);
  gdata_print_string(nickserv_pass);
  gdata_print_int(notifytime);
  gdata_print_int(respondtochannelxdcc);
  gdata_print_int(respondtochannellist);
  gdata_print_int(smallfilebypass);

  gdata_irlist_iter_start(server_join_raw, char);
  gdata_iter_as_print_string;
  gdata_irlist_iter_end;

  gdata_irlist_iter_start(server_connected_raw, char);
  gdata_iter_as_print_string;
  gdata_irlist_iter_end;

  gdata_irlist_iter_start(channel_join_raw, char);
  gdata_iter_as_print_string;
  gdata_irlist_iter_end;

  /* r_channel t_channel r_local_vhost r_pidfile r_config_nick */
  /* r_transferminspeed r_transfermaxspeed */
  
  gdata_irlist_iter_start(servers, server_t);
  gdata_iter_print_string(hostname);
  gdata_iter_print_uint(port);
  gdata_iter_print_string(password);
  gdata_irlist_iter_end;
  
  gdata_print_string(curserver.hostname);
  gdata_print_uint(curserver.port);
  gdata_print_string(curserver.password);
  gdata_print_string(curserveractualname);
  gdata_print_int(nocon);
  gdata_print_int(servertime);
  gdata_print_number_cast("%d",serverstatus,int);
  gdata_print_long(lastservercontact);
  
  gdata_irlist_iter_start(serverq_fast, char);
  gdata_iter_as_print_string;
  gdata_irlist_iter_end;
  
  gdata_irlist_iter_start(serverq_normal, char);
  gdata_iter_as_print_string;
  gdata_irlist_iter_end;
  
  gdata_irlist_iter_start(serverq_slow, char);
  gdata_iter_as_print_string;
  gdata_irlist_iter_end;
  
  gdata_print_int(adjustcore);
  
  gdata_print_int(serverbucket);
  gdata_print_int(ircserver);
  gdata_print_int(serverconnectbackoff);

  for (ii=0; ii<MAX_PREFIX && gdata.prefixes[ii].p_mode; ii++)
    {
      gdata_print_number_array_item("%c",prefixes,p_mode);
      gdata_print_number_array_item("%c",prefixes,p_symbol);
    }
  
  for (ii=0; ii<MAX_CHANMODES && gdata.chanmodes[ii]; ii++)
    {
      gdata_print_number_array("%c", chanmodes);
    }

  gdata_print_int(attop);
  gdata_print_int(needsclear);
  gdata_print_int(termcols);
  gdata_print_int(termlines);
  gdata_print_int(nocolor);
  gdata_print_int(noscreen);
  gdata_print_int(curcol);
  gdata_print_int(console_history_offset);
  gdata_print_string(console_input_line);

  gdata_irlist_iter_start(console_history, char);
  gdata_iter_as_print_string;
  gdata_irlist_iter_end;
  
  /* startup_tio */
     
  /* stdout_buffer_init stdout_buffer */
  
  gdata_irlist_iter_start(ignorelist, channel_t);
  ioutput(gdata_common,"  : name=%s key=%s",
          iter->name,
          gdata_string(iter->key));
  ioutput(gdata_common,"  : flags=%d plsittime=%d plistoffset=%d",
          iter->flags,
          iter->plisttime,
          iter->plistoffset);
  /* members */
  gdata_irlist_iter_end;
  

  gdata_irlist_iter_start(msglog, msglog_t);
  ioutput(gdata_common,"  : when=%ld hostmask=%s message=%s",
          (long)iter->when,
          gdata_string(iter->hostmask),
          gdata_string(iter->message));
  gdata_irlist_iter_end;
  

  gdata_irlist_iter_start(dccchats, dccchat_t);
  ioutput(gdata_common,
          "  : status=%d fd=%d",
          iter->status,
          iter->fd);
  ioutput(gdata_common,
          "  : lastcontact=%ld connecttime=%ld",
          (long)iter->lastcontact,
          (long)iter->connecttime);
  ioutput(gdata_common,
          "  : localport=%d remoteport=%d localip=0x%.8lX remoteip=0x%.8lX",
          iter->localport,
          iter->remoteport,
          iter->localip,
          iter->remoteip);
  gdata_iter_print_string(nick);
  gdata_irlist_iter_end;
  gdata_print_int(num_dccchats);

  gdata_print_number_cast("%d",curtime,int);
  
  /* readset writeset */
  
  gdata_print_float(record);
  gdata_print_float(sentrecord);
  gdata_print_number("%" LLPRINTFMT "u", totalsent);
  gdata_print_long(totaluptime);
  gdata_print_int(debug);
  gdata_print_int(exiting);
  gdata_print_int(crashing);
  
  for (ii=0; ii<XDCC_SENT_SIZE; ii++)
    {
      gdata_print_ulong_array(xdccsent)
    }
  for (ii=0; ii<INAMNT_SIZE; ii++)
    {
      gdata_print_int_array(inamnt)
    }
  
  gdata_print_int(ignore);
  gdata_print_int(slotsmax);
  gdata_print_int(recentsent);
  gdata_print_int(queuesize);
  gdata_print_int(noautosave);
  gdata_print_long(nonewcons);
  gdata_print_long(nolisting);
  gdata_print_int(needsrehash);
  gdata_print_int(needsshutdown);
  gdata_print_int(needsswitch);
  gdata_print_int(needsreap);
  gdata_print_int(delayedshutdown);
  gdata_print_int(cursendptr);
  gdata_print_int(next_tr_id);
  gdata_print_number_cast("%" LLPRINTFMT "d",max_file_size,long long);
  
  gdata_print_uint(max_fds_from_rlimit);
  
  gdata_print_int(nick_number);
  gdata_print_int(logfd);
  
  /* sendbuff context_log context_cur_ptr */
  
  gdata_irlist_iter_start(xlistqueue, char);
  gdata_iter_as_print_string;
  gdata_irlist_iter_end;
  
  gdata_irlist_iter_start(ignorelist, igninfo);
  ioutput(gdata_common,
          "  : hostmask=%s flags=%d bucket=%ld lastcontact=%ld",
          iter->hostmask,
          iter->flags,
          iter->bucket,
          (long)iter->lastcontact);
  gdata_irlist_iter_end;
  
  gdata_irlist_iter_start(xdccs, xdcc);
  gdata_iter_print_string(file);
  gdata_iter_print_string(desc);
  gdata_iter_print_string(note);
  ioutput(gdata_common,
          "  : ptr=0x%.8lX gets=%d minspeed=%.1f maxspeed=%.1f st_size=%" LLPRINTFMT "d",
          (unsigned long)iter,
          iter->gets,
          iter->minspeed,
          iter->maxspeed,
          (long long)iter->st_size);
  /* st_dev st_ino */
  ioutput(gdata_common,
          "  : fd=%d fd_count=%d fd_loc=%" LLPRINTFMT "d",
          iter->file_fd,
          iter->file_fd_count,
          (long long)iter->file_fd_location);
  ioutput(gdata_common,
          "  : has_md5=%d md5sum=" MD5_PRINT_FMT,
          iter->has_md5sum, MD5_PRINT_DATA(iter->md5sum));
#ifdef HAVE_MMAP
  {
    mmap_info_t *iter2;
    ioutput(gdata_common, "  : mmaps:");
    for(iter2 = irlist_get_head(&iter->mmaps);
        iter2;
        iter2 = irlist_get_next(iter2))
      {
        ioutput(gdata_common,
                "  : ptr=%p ref_count=%d mmap_ptr=%p mmap_offset=0x%.8" LLPRINTFMT "X mmap_size=0x%.8" LLPRINTFMT "X",
                iter2,
                iter2->ref_count,
                iter2->mmap_ptr,
                (unsigned long long)iter2->mmap_offset,
                (unsigned long long)iter2->mmap_size);
      }
  }
#endif
  gdata_irlist_iter_end;
  
  gdata_irlist_iter_start(mainqueue, pqueue);
  gdata_iter_print_string(nick);
  gdata_iter_print_string(hostname);
  ioutput(gdata_common,
          "  : xpack=0x%.8lX queuedtime=%ld",
          (unsigned long)iter->xpack,
          (long)iter->queuedtime);
  gdata_irlist_iter_end;
  
  gdata_irlist_iter_start(trans, transfer);
  ioutput(gdata_common,
          "  : listen=%d client=%d id=%d",
          iter->listensocket,
          iter->clientsocket,
          iter->id);
  ioutput(gdata_common,
          "  : sent=%" LLPRINTFMT "d got=%" LLPRINTFMT "d lastack=%" LLPRINTFMT "d curack=%" LLPRINTFMT "d resume=%" LLPRINTFMT "d speedamt=%" LLPRINTFMT "d tx_bucket=%li",
          (long long)iter->bytessent,
          (long long)iter->bytesgot,
          (long long)iter->lastack,
          (long long)iter->curack,
          (long long)iter->startresume,
          (long long)iter->lastspeedamt,
          (long)iter->tx_bucket);
  ioutput(gdata_common,
          "  : lastcontact=%ld connecttime=%ld lastspeed=%.1f pack=0x%.8lX",
          (long)iter->lastcontact,
          (long)iter->connecttime,
          iter->lastspeed,
          (unsigned long)iter->xpack);
  ioutput(gdata_common,
          "  : listenport=%d remoteport=%d localip=0x%.8lX remoteip=0x%.8lX",
          iter->listenport,
          iter->remoteport,
          iter->localip,
          iter->remoteip);
#ifdef HAVE_MMAP
  ioutput(gdata_common, "  : mmap_info=%p", iter->mmap_info);
#endif
  /* severaddress */
  gdata_iter_print_string(nick);
  gdata_iter_print_string(caps_nick);
  gdata_iter_print_string(hostname);
  ioutput(gdata_common,
          "  : nomin=%d nomax=%d reminded=%d overlimit=%d tr_status=%d",
          iter->nomin,
          iter->nomax,
          iter->reminded,
          iter->overlimit,
          iter->tr_status);
  gdata_irlist_iter_end;
  
  gdata_irlist_iter_start(uploads, upload);
  ioutput(gdata_common,
          "  : client=%d file=%d ul_status=%d",
          iter->clientsocket,
          iter->filedescriptor,
          iter->ul_status);
  ioutput(gdata_common,
          "  : got=%" LLPRINTFMT "d totalsize=%" LLPRINTFMT "d resume=%" LLPRINTFMT "d speedamt=%" LLPRINTFMT "d",
          (long long)iter->bytesgot,
          (long long)iter->totalsize,
          (long long)iter->resumesize,
          (long long)iter->lastspeedamt);
  ioutput(gdata_common,
          "  : lastcontact=%ld connecttime=%ld lastspeed=%.1f",
          (long)iter->lastcontact,
          (long)iter->connecttime,
          iter->lastspeed);
  ioutput(gdata_common,
          "  : localport=%d remoteport=%d localip=0x%.8lX remoteip=0x%.8lX",
          iter->localport,
          iter->remoteport,
          iter->localip,
          iter->remoteip);
  gdata_iter_print_string(nick);
  gdata_iter_print_string(hostname);
  gdata_iter_print_string(file);
  gdata_irlist_iter_end;

  gdata_irlist_iter_start(listen_ports, ir_listen_port_item_t);
  gdata_iter_print_number_cast("%hu",port,unsigned short int);
  gdata_iter_print_number_cast("%u",listen_time,unsigned int);
  gdata_irlist_iter_end;
  
  gdata_print_number("%p", md5build.xpack);
  gdata_print_int(md5build.file_fd);
  
  /* meminfo */
  
#if !defined(NO_CHROOT)
  gdata_print_string(chrootdir);
#endif
  
#if !defined(NO_SETUID)
  gdata_print_string(runasuser);
#endif

  ioutput(gdata_common,"GDATA DUMP END");
  
}


void clearmemberlist(channel_t *c)
{
  /* clear members list */
  if (gdata.debug > 2)
    {
      ioutput(CALLTYPE_NORMAL,OUT_S,COLOR_NO_COLOR,
              "clearing %s",c->name);
    }
  
  irlist_delete_all(&c->members);
  return;
}


int isinmemberlist(const char *nick)
{
  member_t *member;
  channel_t *ch;
  
  updatecontext();

  if (gdata.debug > 2)
    {
      ioutput(CALLTYPE_NORMAL,OUT_S,COLOR_NO_COLOR,
              "checking for %s",nick);
    }
  
  ch = irlist_get_head(&gdata.channels);
  while(ch)
    {
      member = irlist_get_head(&ch->members);
      while(member)
        {
          if (!strcasecmp(caps(member->nick),nick))
            {
              return 1;
            }
          member = irlist_get_next(member);
        }
      ch = irlist_get_next(ch);
    }
  
  return 0;
}


void addtomemberlist(channel_t *c, const char *nick)
{
  member_t *member;
  char prefixes[MAX_PREFIX] = {};
  
  updatecontext();

  if (gdata.debug > 2)
    {
      ioutput(CALLTYPE_NORMAL,OUT_S,COLOR_NO_COLOR,
              "adding %s to %s",nick,c->name);
    }
  
  /* find any prefixes */
 more:
  if (*nick)
    {
      int pi;
      for (pi = 0; (pi < MAX_PREFIX && gdata.prefixes[pi].p_symbol); pi++)
        {
          if (*nick == gdata.prefixes[pi].p_symbol)
            {
              for (pi = 0;
                   (pi < MAX_PREFIX && prefixes[pi] && prefixes[pi] != *nick);
                   pi++) ;
              if (pi < MAX_PREFIX)
                {
                  prefixes[pi] = *nick;
                }
              nick++;
              goto more;
            }
        }
    }
  
  /* is in list for this channel already? */
  member = irlist_get_head(&c->members);
  while(member)
    {
      if (!strcasecmp(member->nick,nick))
        {
          break;
        }
      member = irlist_get_next(member);
    }
  
  if (!member)
    {
      /* add it */
      member = irlist_add(&c->members, sizeof(member_t) + strlen(nick));
      strcpy(member->nick, nick);
      memcpy(member->prefixes, prefixes, sizeof(prefixes));
    }
  
  return;
}

void removefrommemberlist(channel_t *c, const char *nick)
{
  member_t *member;
  
  updatecontext();
  
  if (gdata.debug > 2)
    {
      ioutput(CALLTYPE_NORMAL,OUT_S,COLOR_NO_COLOR,
              "removing %s from %s",nick,c->name);
    }
  
  /* is in list for this channel? */
  member = irlist_get_head(&c->members);
  while(member)
    {
      if (!strcasecmp(member->nick,nick))
        {
          irlist_delete(&c->members, member);
          return;
        }
      member = irlist_get_next(member);
    }
  
  /* not found */
  return;
}


void changeinmemberlist_mode(channel_t *c, const char *nick, char mode, int add)
{
  member_t *member;
  int pi;
  
  updatecontext();
  
  if (gdata.debug > 2)
    {
      ioutput(CALLTYPE_NORMAL, OUT_S, COLOR_NO_COLOR,
              "%s prefix %c on %s in %s",
              add ? "adding" : "removing",
              mode, nick, c->name);
    }
  
  /* is in list for this channel? */
  member = irlist_get_head(&c->members);
  while(member)
    {
      if (!strcasecmp(member->nick,nick))
        {
          break;
        }
      member = irlist_get_next(member);
    }
  
  if (member)
    {
      if (add)
        {
          for (pi = 0;
               (pi < MAX_PREFIX && member->prefixes[pi] && member->prefixes[pi] != mode);
               pi++) ;
          if (pi < MAX_PREFIX)
            {
              member->prefixes[pi] = mode;
            }
        }
      else
        {
          for (pi = 0; (pi < MAX_PREFIX && member->prefixes[pi]); pi++)
            {
              if (member->prefixes[pi] == mode)
                {
                  for (pi++; pi < MAX_PREFIX; pi++)
                    {
                      member->prefixes[pi-1] = member->prefixes[pi];
                    }
                  member->prefixes[MAX_PREFIX-1] = '\0';
                  break;
                }
            }
        }
      
    }
  
  return;
}


void changeinmemberlist_nick(channel_t *c, const char *oldnick, const char *newnick)
{
  member_t *oldmember;
  member_t *newmember;
  
  updatecontext();
  
  if (gdata.debug > 2)
    {
      ioutput(CALLTYPE_NORMAL,OUT_S,COLOR_NO_COLOR,
              "changing %s to %s in %s",oldnick,newnick,c->name);
    }
  
  /* find old in list for this channel  */
  oldmember = irlist_get_head(&c->members);
  while(oldmember)
    {
      if (!strcasecmp(oldmember->nick,oldnick))
        {
          break;
        }
      oldmember = irlist_get_next(oldmember);
    }
  
  /* add it */
  newmember = irlist_add(&c->members, sizeof(member_t) + strlen(newnick));
  strcpy(newmember->nick, newnick);
  
  if (oldmember)
    {
      memcpy(newmember->prefixes, oldmember->prefixes, sizeof(oldmember->prefixes));
      irlist_delete(&c->members, oldmember);
    }
  else
    {
      /* this shouldn't happen, set no prefixes */
      memset(newmember->prefixes, 0, sizeof(newmember->prefixes));
    }
  
  return;
}

int set_socket_nonblocking (int s, int nonblock)
{
  long current;
  
  current = fcntl(s, F_GETFL, 0);
  
  if (current == -1)
    {
      return -1;
    }
  
  if (nonblock)
    return fcntl(s, F_SETFL, current | O_NONBLOCK);
  else
    return fcntl(s, F_SETFL, current & ~O_NONBLOCK);
}

void set_loginname(void)
{
  struct passwd *p;
  
  p = getpwuid(geteuid());
  if (p == NULL || p->pw_name == NULL)
    {
#if !defined(NO_SETUID)
      if (gdata.runasuser)
        {
          gdata.loginname = mymalloc(strlen(gdata.runasuser)+1);
          strcpy(gdata.loginname,gdata.runasuser);
        }
      else
#endif
        {
          outerror(OUTERROR_TYPE_WARN_LOUD,"Couldn't Get username, specify loginname in config file");
          gdata.loginname = mymalloc(strlen("UNKNOWN")+1);
          strcpy(gdata.loginname,"UNKNOWN");
        }
    }
  else
    {
      gdata.loginname = mymalloc(strlen(p->pw_name)+1);
      strcpy(gdata.loginname,p->pw_name);
    }
  
}


int is_fd_readable(int fd)
{
  int ret;
  fd_set readfds;
  struct timeval timeout;
  
  FD_ZERO(&readfds);
  FD_SET(fd, &readfds);
  
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  
  ret = select(fd+1, &readfds, NULL, NULL, &timeout);
  
  if (ret < 0)
    {
      return 0;
    }
  
  if (FD_ISSET(fd, &readfds))
    {
      return 1;
    }
  
  return 0;
}

int is_fd_writeable(int fd)
{
  int ret;
  fd_set writefds;
  struct timeval timeout;
  
  FD_ZERO(&writefds);
  FD_SET(fd, &writefds);
  
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  
  ret = select(fd+1, NULL, &writefds, NULL, &timeout);
  
  if (ret < 0)
    {
      return 0;
    }
  
  if (FD_ISSET(fd, &writefds))
    {
      return 1;
    }
  
  return 0;
}



#ifdef NO_SNPRINTF

int snprintf(char *str, size_t n, const char *format, ... )
{
  va_list args;
  int actlen;
  char mysnprintf_buff[1024*10];   
  
  va_start(args, format);
  actlen = vsprintf(mysnprintf_buff,format,args);
  va_end(args);
  
  strncpy(str,mysnprintf_buff,min2(n-1,actlen));
  str[min2(n-1,actlen)] = '\0';
  
  return min2(n-1,actlen);
}

int vsnprintf(char *str, size_t n, const char *format, va_list ap )
{
  int actlen;
  char mysnprintf_buff[1024*10];   
  
  actlen = vsprintf(mysnprintf_buff,format,ap);
  
  strncpy(str,mysnprintf_buff,min2(n-1,actlen));
  str[min2(n-1,actlen)] = '\0';
  
  return min2(n-1,actlen);
}

#endif

#ifdef NO_STRCASECMP
int strcasecmp(const char *s1, const char *s2)
{
  while (*s1 && (tolower(*s1) == tolower(*s2)))
    {
      s1++;
      s2++;
    }
  
  return tolower(*s1) - tolower(*s2);
}
#endif

#ifdef NO_STRSIGNAL
const char *strsignal(int sig)
{
  switch (sig)
    {
    case SIGBUS:
      return "sigbus";
    case SIGABRT:
      return "sigabrt";
    case SIGILL:
      return "sigill";
    case SIGFPE:
      return "sigfpe";
    case SIGSEGV:
      return "sigsegv";
    case SIGTERM:
      return "sigterm";
    case SIGINT:
      return "sigint";
    case SIGUSR1:
      return "sigusr1";
    case SIGUSR2:
      return "sigusr2";
    default:
      return "unknown";
    }
}
#endif


char* convert_to_unix_slash(char *ss)
{
  int ii;
  
  for (ii=0; ; ii++)
    {
      if (ss[ii] == '\\')
        {
          ss[ii] = '/';
        }
      else if (ss[ii] == '\0')
        {
          return ss;
        }
    }
}

#define IRLIST_INT_TO_EXT(p) ((irlist_item_t *)p + 1)
#define IRLIST_INT_TO_EXT_CONST(p) ((const irlist_item_t *)p + 1)
#define IRLIST_EXT_TO_INT(p) ((irlist_item_t *)p - 1)
#define IRLIST_EXT_TO_INT_CONST(p) ((const irlist_item_t *)p - 1)

void* irlist_add2(irlist_t *list, unsigned int size,
                  const char *src_function, const char *src_file, int src_line)
{
  irlist_item_t *iitem;
  
  updatecontext();
  
  iitem = mymalloc2(sizeof(irlist_item_t) + size, 1,
                    src_function, src_file, src_line);
  
  irlist_insert_tail(list, IRLIST_INT_TO_EXT(iitem));
  
  return IRLIST_INT_TO_EXT(iitem);
}

void irlist_insert_head(irlist_t *list, void *item)
{
  irlist_item_t *iitem = IRLIST_EXT_TO_INT(item);
  
  updatecontext();
  
  assert(!iitem->next);
  assert(!iitem->prev);
  
  if (!list->size)
    {
      assert(!list->head);
      assert(!list->tail);
      
      list->tail = iitem;
    }
  else
    {
      assert(list->head);
      assert(list->tail);
      assert(!list->head->prev);
      assert(!list->tail->next);
      
      list->head->prev = iitem;
    }
  
  iitem->next = list->head;
  iitem->prev = NULL;
  list->head = iitem;
  
  list->size++;
  
  return;
}

void irlist_insert_tail(irlist_t *list, void *item)
{
  irlist_item_t *iitem = IRLIST_EXT_TO_INT(item);
  
  updatecontext();
  
  assert(!iitem->next);
  assert(!iitem->prev);
  
  if (!list->size)
    {
      assert(!list->head);
      assert(!list->tail);
      
      list->head = iitem;
    }
  else
    {
      assert(list->head);
      assert(list->tail);
      assert(!list->head->prev);
      assert(!list->tail->next);
      
      list->tail->next = iitem;
    }
  
  iitem->next = NULL;
  iitem->prev = list->tail;
  list->tail = iitem;
  
  list->size++;
  
  return;
}

void irlist_insert_before(irlist_t *list, void *item, void *before_this)
{
  irlist_item_t *iitem = IRLIST_EXT_TO_INT(item);
  irlist_item_t *ibefore = IRLIST_EXT_TO_INT(before_this);
  
  updatecontext();
  
  assert(list->size > 0);
  assert(!iitem->next);
  assert(!iitem->prev);
  
  if (ibefore->prev)
    {
      iitem->prev = ibefore->prev;
      iitem->prev->next = iitem;
    }
  else
    {
      assert(list->head == ibefore);
      list->head = iitem;
    }
  
  ibefore->prev = iitem;
  iitem->next = ibefore;
  
  list->size++;
  
  return;
}

void irlist_insert_after(irlist_t *list, void *item, void *after_this)
{
  irlist_item_t *iitem = IRLIST_EXT_TO_INT(item);
  irlist_item_t *iafter = IRLIST_EXT_TO_INT(after_this);
  
  updatecontext();
  
  assert(list->size > 0);
  assert(!iitem->next);
  assert(!iitem->prev);
  
  if (iafter->next)
    {
      iitem->next = iafter->next;
      iitem->next->prev = iitem;
    }
  else
    {
      assert(list->tail == iafter);
      list->tail = iitem;
    }
  
  iafter->next = iitem;
  iitem->prev = iafter;
  
  list->size++;
  
  return;
}


void* irlist_delete(irlist_t *list, void *item)
{
  irlist_item_t *iitem = IRLIST_EXT_TO_INT(item);
  void *retval;
  
  updatecontext();
  
  retval = irlist_remove(list, item);
  
  mydelete(iitem);
  
  return retval;
}


void* irlist_remove(irlist_t *list, void *item)
{
  irlist_item_t *iitem = IRLIST_EXT_TO_INT(item);
  irlist_item_t *next;
  
  updatecontext();
  
  assert(list->size > 0);
  assert(list->head);
  assert(list->tail);
  
  next = iitem->next;
  
  if (list->head == iitem)
    {
      assert(!iitem->prev);
      list->head = iitem->next;
    }
  if (list->tail == iitem)
    {
      assert(!iitem->next);
      list->tail = iitem->prev;
    }
  
  if (iitem->next)
    {
      assert(iitem->next->prev == iitem);
      iitem->next->prev = iitem->prev;
    }
  
  if (iitem->prev)
    {
      assert(iitem->prev->next == iitem);
      iitem->prev->next = iitem->next;
    }
  
  iitem->next = NULL;
  iitem->prev = NULL;
  
  list->size--;
  assert(list->size >= 0);
  
  if (next)
    {
      return IRLIST_INT_TO_EXT(next);
    }
  else
    {
      return NULL;
    }
}


void irlist_delete_all(irlist_t *list)
{
  void *cur;
  
  updatecontext();
  
  for (cur = irlist_get_head(list); cur; cur = irlist_delete(list, cur));
  
  assert(list->size == 0);
  assert(!list->head);
  assert(!list->tail);
  
  return;
}


void* irlist_get_head(const irlist_t *list)
{
  updatecontext();
  
  if (list->head)
    {
      assert(list->size > 0);
      assert(list->tail);
      assert(!list->head->prev);
      return IRLIST_INT_TO_EXT(list->head);
    }
  else
    {
      assert(list->size == 0);
      assert(!list->tail);
      return NULL;
    }
}

void* irlist_get_tail(const irlist_t *list)
{
  updatecontext();
  
  if (list->tail)
    {
      assert(list->size > 0);
      assert(list->head);
      assert(!list->tail->next);
      return IRLIST_INT_TO_EXT(list->tail);
    }
  else
    {
      assert(list->size == 0);
      assert(!list->head);
      return NULL;
    }
}


void* irlist_get_next(const void *cur)
{
  const irlist_item_t *iitem = IRLIST_EXT_TO_INT_CONST(cur);
  
  if (iitem->next)
    {
      assert(iitem->next->prev == iitem);
      return IRLIST_INT_TO_EXT(iitem->next);
    }
  else
    {
      return NULL;
    }
}


void* irlist_get_prev(const void *cur)
{
  const irlist_item_t *iitem = IRLIST_EXT_TO_INT_CONST(cur);
  
  updatecontext();
  
  if (iitem->prev)
    {
      assert(iitem->prev->next == iitem);
      return IRLIST_INT_TO_EXT(iitem->prev);
    }
  else
    {
      return NULL;
    }
}

int irlist_size(const irlist_t *list)
{
  updatecontext();
  
  assert(list->size >= 0);
  
  return list->size;
}

void* irlist_get_nth(irlist_t *list, int nth)
{
  irlist_item_t *iitem;
  
  updatecontext();
  
  assert(nth >= 0);
  
  for (iitem = list->head; (iitem && nth--); iitem = iitem->next) ;
  
  if (iitem)
    {
      return IRLIST_INT_TO_EXT(iitem);
    }
  else
    {
      return NULL;
    }
}

int irlist_sort_cmpfunc_string(void *userdata, const void *a, const void *b)
{
  return strcmp((const char *)a, (const char *)b);
}

int irlist_sort_cmpfunc_int(void *userdata, const void *a, const void *b)
{
  int ai, bi;
  ai = *(const int*)a;
  bi = *(const int*)b;
  return ai - bi;
}

int irlist_sort_cmpfunc_off_t(void *userdata, const void *a, const void *b)
{
  off_t ai, bi;
  ai = *(const off_t*)a;
  bi = *(const off_t*)b;
  return ai - bi;
}

void irlist_sort(irlist_t *list,
                 int (*cmpfunc)(void *userdata, const void *a, const void *b),
                 void *userdata)
{
  irlist_t newlist = {};
  void *cur, *try;
  
  while ((cur = irlist_get_head(list)))
    {
      irlist_remove(list, cur);
      
      try = irlist_get_head(&newlist);
      if (!try)
        {
          irlist_insert_head(&newlist, cur);
          continue;
        }
      
      while (try)
        {
          if (cmpfunc(userdata, cur, try) < 0)
            {
              irlist_insert_before(&newlist, cur, try);
              break;
            }
          try = irlist_get_next(try);
        }
      
      if (!try)
        {
          irlist_insert_tail(&newlist, cur);
        }
    }
  
  
  *list = newlist;
  return;
}

transfer* does_tr_id_exist(int tr_id)
{
  transfer *tr;
  
  tr = irlist_get_head(&gdata.trans);
  while(tr)
    {
      if (tr->id == tr_id)
        {
          return tr;
        }
      tr = irlist_get_next(tr);
    }
  
  return NULL;
}

int get_next_tr_id(void)
{
  transfer *tr;
  
  while(1)
    {
      gdata.next_tr_id++;
      gdata.next_tr_id %= max2((MAXTRANS * 3) / 2, 1000);
      gdata.next_tr_id = max2(1,gdata.next_tr_id);
      tr = does_tr_id_exist(gdata.next_tr_id);
      if (!tr)
        {
          return gdata.next_tr_id;
        }
    }
}

void ir_listen_port_connected(ir_uint16 port)
{
  ir_listen_port_item_t *lp;
  
  lp = irlist_get_head(&gdata.listen_ports);
  while(lp)
    {
      if (lp->port == port)
        {
          if (gdata.debug > 0)
            {
              ioutput(CALLTYPE_NORMAL,OUT_S,COLOR_YELLOW,
                      "listen complete port %d", port);
            }
          lp = irlist_delete(&gdata.listen_ports, lp);
        }
      else
        {
          lp = irlist_get_next(lp);
        }
    }
  return;
}

static int ir_listen_port_is_in_list(ir_uint16 port)
{
  int retval = 0;
  ir_listen_port_item_t *lp;
  
  lp = irlist_get_head(&gdata.listen_ports);
  while(lp)
    {
      if ((lp->listen_time + LISTEN_PORT_REUSE_TIME) < gdata.curtime)
        {
          if (gdata.debug > 0)
            {
              ioutput(CALLTYPE_NORMAL,OUT_S,COLOR_YELLOW,
                      "listen expire port %d", lp->port);
            }
          lp = irlist_delete(&gdata.listen_ports, lp);
        }
      else
        {
          if (lp->port == port)
            {
              retval = 1;
            }
          lp = irlist_get_next(lp);
        }
    }
  return retval;
}

int ir_bind_listen_socket(int fd, struct sockaddr_in *sa)
{
  ir_listen_port_item_t *lp;
  int retry;
  int max;
  ir_uint16 port;
  SIGNEDSOCK int addrlen;
  
  max = (MAXTRANS+MAXUPLDS+MAXCHATS+irlist_size(&gdata.listen_ports));
  
  for (retry = 0; retry < max; retry++)
    {
      if (gdata.tcprangestart)
        {
          port = gdata.tcprangestart + retry;
          
          if (ir_listen_port_is_in_list(port))
            {
              continue;
            }
          
          sa->sin_port = htons(port);
        }
      else
        {
          sa->sin_port = htons(0);
        }
      
      if (bind(fd, (struct sockaddr *)sa, sizeof(struct sockaddr_in)) < 0)
        {
          if (!gdata.tcprangestart)
            {
              /* give up */
              retry = max;
              break;
            }
        }
      else
        {
          break;
        }
    }

  if (retry == max)
    {
      return -1;
    }
  
  addrlen = sizeof (struct sockaddr_in);
  
  if ((getsockname (fd, (struct sockaddr *)sa, &addrlen)) < 0)
    {
      outerror(OUTERROR_TYPE_WARN_LOUD,"Couldn't get Port Number, Aborting");
      return -1;
    }
  
  if (gdata.debug > 0)
    {
      ioutput(CALLTYPE_NORMAL,OUT_S,COLOR_YELLOW,
              "listen got port %d", ntohs(sa->sin_port));
    }
  
  lp = irlist_add(&gdata.listen_ports,sizeof(ir_listen_port_item_t));
  lp->port = ntohs(sa->sin_port);
  lp->listen_time = gdata.curtime;
  
  return 0;
}


int ir_boutput_write(ir_boutput_t *bout, const void *buffer, int buffer_len)
{
  ir_boutput_segment_t *segment;
  int cur;
  int len;
  const char *buffer_c = (const char*)buffer;
  
  cur = 0;
  
 begin_again:
  
  segment = irlist_get_tail(&bout->segments);
  
  if (!segment)
    {
      /* first */
      segment = irlist_add(&bout->segments, sizeof(ir_boutput_segment_t));
    }
  
  while (cur < buffer_len)
    {
      assert(segment->begin <= segment->end);
      assert(segment->begin <= IR_BOUTPUT_SEGMENT_SIZE);
      assert(segment->end <= IR_BOUTPUT_SEGMENT_SIZE);
      
      if (segment->end == IR_BOUTPUT_SEGMENT_SIZE)
        {
          /* segment is full */
          if ((bout->flags & BOUTPUT_NO_LIMIT) ||
              (irlist_size(&bout->segments) < IR_BOUTPUT_MAX_SEGMENTS))
            {
              segment = irlist_add(&bout->segments, sizeof(ir_boutput_segment_t));
            }
          else
            {
              /* too much data in buffer, attempt flush */
              if (!(bout->flags & BOUTPUT_NO_FLUSH) &&
                  (ir_boutput_attempt_flush(bout) > 0))
                {
                  /* flush wrote something so start over fresh */
                  goto begin_again;
                }
              
              /* unable to flush, drop characters */
              bout->count_dropped += buffer_len - cur;
              return cur;
            }
        }
      
      len = min2(IR_BOUTPUT_SEGMENT_SIZE - segment->end,
                 buffer_len - cur);
      
      if (bout->md5sum)
        {
          MD5Update(bout->md5sum, buffer_c+cur, len);
        }
      
      memcpy(segment->buffer+segment->end, buffer_c+cur, len);
      
      cur += len;
      segment->end += len;
      bout->count_written += len;
    }
  
  return buffer_len;
}

int ir_boutput_need_flush(ir_boutput_t *bout)
{
  return irlist_size(&bout->segments);
}

int ir_boutput_attempt_flush(ir_boutput_t *bout)
{
  ir_boutput_segment_t *segment;
  int count = 0;
  
  if (bout->flags & BOUTPUT_NO_FLUSH)
    {
      return 0;
    }
  
  while ((segment = irlist_get_head(&bout->segments)))
    {
      int retval;
      
      assert(segment->begin <= segment->end);
      assert(segment->begin <= IR_BOUTPUT_SEGMENT_SIZE);
      assert(segment->end <= IR_BOUTPUT_SEGMENT_SIZE);
      
      retval = write(bout->fd,
                     segment->buffer + segment->begin,
                     segment->end - segment->begin);
      
      if ((retval < 0) && (errno != EAGAIN))
        {
          /* write failure */
          count = -1;
          break;
        }
      else if (retval < 0)
        {
          /* EAGAIN, that's all for now */
          break;
        }
      else
        {
          segment->begin += retval;
          count += retval;
          bout->count_flushed += retval;
        }
      
      assert(segment->begin <= segment->end);
      if (segment->begin == segment->end)
        {
          irlist_delete(&bout->segments, segment);
        }
    }
  
  return count;
}

void ir_boutput_init(ir_boutput_t *bout, int fd, int flags)
{
  memset(bout, 0, sizeof(*bout));
  bout->fd = fd;
  bout->flags = flags;
  if (bout->flags & BOUTPUT_MD5SUM)
    {
      bout->md5sum = mycalloc(sizeof(struct MD5Context));
      MD5Init(bout->md5sum);
    }
  return;
}

void ir_boutput_set_flags(ir_boutput_t *bout, int flags)
{
  bout->flags = flags;
  if ((bout->flags & BOUTPUT_MD5SUM) && (!bout->md5sum))
    {
      bout->md5sum = mycalloc(sizeof(struct MD5Context));
      MD5Init(bout->md5sum);
    }
  else if (!(bout->flags & BOUTPUT_MD5SUM) && bout->md5sum)
    {
      mydelete(bout->md5sum);
    }
  return;
}


void ir_boutput_delete(ir_boutput_t *bout)
{
  irlist_delete_all(&bout->segments);
  mydelete(bout->md5sum);
  memset(bout, 0, sizeof(*bout));
  return;
}

void ir_boutput_get_md5sum(ir_boutput_t *bout, MD5Digest digest)
{
  if (bout->md5sum)
    {
      MD5Final(digest, bout->md5sum);
      bout->flags &= ~BOUTPUT_MD5SUM;
      mydelete(bout->md5sum);
    }
  return;
}

const char *transferlimit_type_to_string(transferlimit_type_e type)
{
  switch (type)
    {
      case TRANSFERLIMIT_DAILY:
        return "daily";
      case TRANSFERLIMIT_WEEKLY:
        return "weekly";
      case TRANSFERLIMIT_MONTHLY:
        return "monthly";
      default:
        return "unknown";
    }
}

/* End of File */






