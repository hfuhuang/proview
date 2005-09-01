/* 
 * Proview   $Id: tools_msg2cmsg.c,v 1.2 2005-09-01 14:58:00 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
* Filename:
*   tools_msg2cmsg.c
*
* Revision history:
*   Rev     Edit    Date        Who  Comment
*   ------- ----    ----------  ---- -------
*   X0.1       1    1996-10-01  ML   Created
*
* Description:	Creates a .c_msg-file and a .h-file from a .msg-file
*/





#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pwr_lst.h"
#include "pwr_msg.h"
#include "msg2cmsg.h"


#define MSG_NEW_STRING(str) strcpy(malloc(strlen(str) + 1), str)


typedef struct s_FacilityCB sFacilityCB;
typedef struct s_MsgCB sMsgCB;

LstType(sFacilityCB);
LstType(sMsgCB);



struct s_MsgCB {
  LstLink(sMsgCB)   MsgL;
  msg_sMsg	    m;
  msg_eSeverity	    Severity;
};


struct s_FacilityCB {
  LstLink(sFacilityCB)	FacL;
  LstHead(sMsgCB)	MsgH;
  msg_sFacility		f;
};



/* defined by lexyy.c */
extern int lineno;



static int SyntaxError;

static LstHead(sFacilityCB)     lFacH;




/*
 * Local functions
 */


static void WriteFiles(char *fname, FILE *cfp, FILE *hfp);

static void TranslateFormatSpec(const char *msgstr, char **transstr);



/*
* Main
*/
int main(int argc, char **argv)
{
  extern FILE *yyin;
  extern int yylex();

  FILE *cfp = NULL;
  FILE *hfp = NULL;


  if (argc != 4) {
      printf("Usage: co_msg2cmsg msg-file c_msg-file h_file\n");
      exit(2);
  }

  if (!(yyin = fopen(argv[1], "r"))) {
      printf("Can't open input file: %s\n", argv[1]);
      exit(2);
  }

  if (!(cfp = fopen(argv[2], "w"))) {
      printf("Can't open c_msg-output file: %s\n", argv[2]);
      fclose(yyin);
      exit(2);
  }

  if (!(hfp = fopen(argv[3], "w"))) {
      printf("Can't open h-output file: %s\n", argv[3]);
      fclose(yyin);
      fclose(cfp);
      exit(2);
  }


  LstIni(&lFacH);
  SyntaxError = 0;
  lineno = 1;
  yylex();

  if (!SyntaxError) {
    char fname[256];
    char *p;

#if defined OS_VMS || defined OS_ELN
    p = strpbrk(argv[2], "]>");
    if (!p)
      p = strchr(argv[2], ':');
    else {
      char *p2;
      while(p2 = strpbrk(p + 1, "]>"))
	p = p2;
    }
    if (p)
      for (i = 0; p[i+1]; i++)
	fname[i] = tolower(p[i+1]);
    else
      for (i = 0; argv[2][i]; i++)
	fname[i] = tolower(argv[2][i]);
    fname[i] = '\0';
#else
    if ((p = strrchr(argv[2], '/')))
      strcpy(fname, p+1);
    else
      strcpy(fname, argv[2]);
    
#endif

    if ((p = strchr(fname, '.')))
      *p = '\0';

    WriteFiles(fname, cfp, hfp);
  }

  fclose(yyin);
  fclose(cfp);
  fclose(hfp);

#if defined OS_VMS || defined OS_ELN
  exit(1);
#elif defined OS_LYNX || defined OS_LINUX
  exit(EXIT_SUCCESS);
#endif
}

/*
 * Routines which are called by lexyy.c
 */


void lex_FacName(const char *FacName)
{
  sFacilityCB *facp = (sFacilityCB *)calloc(1, sizeof(sFacilityCB));

  LstIni(&facp->MsgH);
  facp->f.FacName = MSG_NEW_STRING(FacName);

  LstIns(&lFacH, facp, FacL);

}



void lex_FacNum(int FacNum)
{
  LstObj(LstLas(&lFacH))->f.FacNum = FacNum;
}



void lex_FacPrefix(const char *Prefix)
{
  LstObj(LstLas(&lFacH))->f.Prefix = MSG_NEW_STRING(Prefix);
}



void lex_MsgName(const char *MsgName)
{
  sMsgCB *msgp = (sMsgCB *)calloc(1, sizeof(sMsgCB));
  int i;
  int len = strlen(MsgName);
  msgp->m.MsgName = malloc(len + 1);

  for (i = 0; i < len; i++)
    msgp->m.MsgName[i] = toupper(MsgName[i]);
  msgp->m.MsgName[i] = '\0';

  (void) LstIns(&LstObj(LstLas(&lFacH))->MsgH, msgp, MsgL);

}

void lex_MsgText(const char *Text)
{
  LstLink(sMsgCB) *ml = LstLas(&LstObj(LstLas(&lFacH))->MsgH);

  TranslateFormatSpec(Text, &LstObj(ml)->m.MsgTxt);  /* convert any VMS-style form spec */
}

void lex_MsgSeverity(msg_eSeverity Severity)
{
  LstLink(sMsgCB) *ml = LstLas(&LstObj(LstLas(&lFacH))->MsgH);
  LstObj(ml)->Severity = Severity;
}

void lex_LexError(int Lineno, char *Str)
{
  printf("LexError: line %d, '%s'\n", Lineno, Str);
  SyntaxError = 1;
}



/*
 * Local routines
 */
static void WriteFiles(char *fname, FILE *cfp, FILE *hfp)
{
  LstLink(sFacilityCB) *fl;
  LstLink(sMsgCB) *ml;
  int idx;
  int facid;
  char prefix[32];
  char name[64];
  char msgName[64];
  int msg;

  fprintf(hfp, "#ifndef %s_h\n", fname);
  fprintf(hfp, "#define %s_h\n\n", fname);

  for (fl = LstFir(&lFacH); fl != LstEnd(&lFacH); fl = LstNex(fl)) {
    facid = 0x800 + LstObj(fl)->f.FacNum;
#if defined OS_VMS || OS_ELN
    sprintf(name, "%s$_FACILITY", LstObj(fl)->f.FacName);
#else
    sprintf(name, "%s_FACILITY", LstObj(fl)->f.FacName);
#endif
    fprintf(hfp, "#define %-29s %9d /* x%08x */\n", name, facid, facid);
    facid = facid << 16;

    if (LstObj(fl)->f.Prefix)
      strcpy(prefix, LstObj(fl)->f.Prefix);
    else
      sprintf(prefix, "%s_", LstObj(fl)->f.FacName);

    sprintf(msgName, "%smsg", LstObj(fl)->f.FacName);
    fprintf(cfp, "static msg_sMsg %s[] = {\n", msgName);


    for (idx = 1, ml = LstFir(&LstObj(fl)->MsgH); ml != LstEnd(&LstObj(fl)->MsgH); ml = LstNex(ml), idx++) {
      if (idx != 1)
	fprintf(cfp, ",\n");

      msg = facid + 0x8000 + (idx << 3) + LstObj(ml)->Severity;
      sprintf(name, "%s%s", prefix, LstObj(ml)->m.MsgName);
      fprintf(hfp, "#define %-29s %9.9d /* x%08x */\n", name, msg, msg);
      fprintf(cfp, "\t{\"%s\", \"%s\"}", LstObj(ml)->m.MsgName, LstObj(ml)->m.MsgTxt);
    }
    fprintf(cfp, "\n};\n\n");

    fprintf(cfp, "static msg_sFacility %sfacility[] = {\n\t", LstObj(fl)->f.FacName);
    fprintf(cfp, "{%d, \"%s\", \"%s\", MSG_NOF(%s), %s}\n",
      LstObj(fl)->f.FacNum,
      LstObj(fl)->f.FacName,
      prefix,
      msgName, msgName);
    fprintf(cfp, "};\n\n");

  }

  fprintf(hfp, "\n#endif\n");

}

/*
 * TranslateFormatSpec
 *
 *  search the msgstr and replace any occurance of VMS-like specifiers
 *  (!AF !UL !XL, are replaced by %s %u %x)
 *
 */
static void TranslateFormatSpec(const char *msgstr, char **transstr)
{
  char  *l;
  const char *m = msgstr + 1; /* Skip '<' */
  int extra = 0;
  int len = 0;
  
 /* '"' will be substituted with  '\"', allocate space for it */
  while (*m) {
    if (*m == '"')
      extra ++;
    m++;
    len++;
  }
  
  *transstr = (char*) malloc(len + extra + 1);
  l = *transstr;
  m = msgstr + 1; /* Skip '<' */

  while (*m != '\0') {
    if (*m == '"') {
      	*l++ = '\\';
      	*l++ = '"';
      	m++;
    }
    else if (*m++ == '!') {
      /*
       * could be a format spec, if so we also skip '!'
       */
      if (*m == 'A' && *(m + 1) == 'F') {
        *l++ = '%';   m++;
        *l++ = 's';   m++;
      }
      else if (*m == 'U' && *(m + 1) == 'L') {
        *l++ = '%';   m++;
        *l++ = 'u';   m++;
      }
      else if (*m == 'X' && *(m + 1) == 'L') {
        *l++ = '%';   m++;
        *l++ = 'x';   m++;
      }
      else
        *l++  = '!'; /* nope, put back the '!' */

    }
    else /* just copy */
      *l++ = *(m-1);
  }
  *(l - 1) = '\0'; /* Skip '>' */
}

