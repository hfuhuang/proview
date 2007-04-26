/* 
 * Proview   $Id: cnv_changelog.cpp,v 1.6 2007-04-26 11:06:05 claes Exp $
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

#include <functional>
#include <algorithm>
#include <iostream.h>
#include <fstream.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern "C" {
#include "pwr.h"
#include "co_dcli.h"
#include "co_cdh.h"
#include "co_time.h"
}
#include "cnv_changelog.h"


CnvChangeLog::CnvChangeLog( CnvCtx *cnv_ctx, char *from_str) :
  ctx(cnv_ctx), from(0)
{
  pwr_tStatus sts;

  if ( strcmp( from_str, "") != 0) {
    sts = time_AsciiToA( from_str, &from_time);
    if ( ODD( sts))
      from = 1;
  }

  read( "src");
  read( "xtt");
  read( "wb");
  read( "nmps");
  read( "remote");
  read( "profibus");
  read( "opc");
  read( "java");
  read( "tlog");
  read( "bcomp");
  read( "othermanu");
  read( "abb");
  read( "siemens");
  read( "ssabox");
  read( "telemecanique");
  read( "inor");
  read( "klocknermoeller");
  print();
  print_html();
}

int CnvChangeLog::read( char *module)
{
  int sts;
  char orig_line[400];
  char line[400];
  char timstr1[40];
  char timstr2[40];
  FILE *fp;
  int i;
  pwr_tFileName fname;
  char *s;

  sprintf( fname, "$pwre_croot/%s/changelog.txt", module);
  dcli_translate_filename( fname, fname);
  fp = fopen( fname, "r");
  if ( !fp)
    return 0;

  while( 1) {
    sts = CnvCtx::read_line( orig_line, sizeof(orig_line), fp);
    if ( !sts)
      break;
    else if ( orig_line[0] == '#')
      continue;
    else {
      if ( isdigit(orig_line[0])) {
	LogEntry p;

	// New log entry
	i = 0;
	for ( s = orig_line; *s && *s != 32 && *s != 9; s++) {
	  if ( i >= (int)sizeof(timstr1))
	    break;
	  timstr1[i++] = *s;
	}
	timstr1[i] = 0;
	  
	for ( ; *s && (*s == 32 || *s == 9); s++) ;

	i = 0;
	for ( ; *s && *s != 32 && *s != 9; s++) {
	  if ( i >= (int)sizeof(p.signature))
	    break;
	  p.signature[i++] = *s;
	}
	p.signature[i] = 0;

	for ( ; *s && (*s == 32 || *s == 9); s++) ;

	i = 0;
	for ( ; *s && *s != 32 && *s != 9; s++) {
	  if ( i >= (int)sizeof(p.component))
	    break;
	  p.component[i++] = *s;
	}
	p.component[i] = 0;

	for ( ; *s && (*s == 32 || *s == 9); s++) ;

	strncpy( p.text, s, sizeof(p.text));
	strncpy( p.module, module, sizeof(p.module));

	sprintf( timstr2, "20%c%c-%c%c-%c%c 00:00", timstr1[0], timstr1[1],
		 timstr1[2], timstr1[3], timstr1[4], timstr1[5]);
	time_FormAsciiToA( timstr2, MINUTE, 0, &p.time);

	entries.push_back( p);
      }
      else {
	// Continuation of log entry
	CnvCtx::remove_spaces( orig_line, line);
	if ( strcmp( line, "") == 0)
	  continue;

	if ( entries.size() == 0)
	  continue;
	strcat( entries[entries.size()-1].text, " ");
	strcat( entries[entries.size()-1].text, line);
      }
    }
  }
  fclose( fp);

  return 1;
}

void CnvChangeLog::sort_time()
{
  int n = entries.size();

  for ( int gap = n/2; 0 < gap; gap /= 2) {
    for ( int i = gap; i < n; i++) {
      for ( int j = i - gap; 0 <= j; j -= gap) {
	if ( entries[j+gap].time.tv_sec < entries[j].time.tv_sec) {
	  LogEntry temp = entries[j];
	  entries[j] = entries[j+gap];
	  entries[j+gap] = temp;
	}
      }
    }
  }
}

void CnvChangeLog::sort_module()
{
  int n = entries.size();

  for ( int gap = n/2; 0 < gap; gap /= 2) {
    for ( int i = gap; i < n; i++) {
      for ( int j = i - gap; 0 <= j; j -= gap) {
	if ( strcmp( entries[j+gap].module, entries[j].module) > 0) {
	  LogEntry temp = entries[j];
	  entries[j] = entries[j+gap];
	  entries[j+gap] = temp;
	}
      }
    }
  }
}

void CnvChangeLog::sort_component()
{
  int n = entries.size();

  for ( int gap = n/2; 0 < gap; gap /= 2) {
    for ( int i = gap; i < n; i++) {
      for ( int j = i - gap; 0 <= j; j -= gap) {
	if ( strcmp( entries[j+gap].component, entries[j].component) > 0) {
	  LogEntry temp = entries[j];
	  entries[j] = entries[j+gap];
	  entries[j+gap] = temp;
	}
      }
    }
  }
}

void CnvChangeLog::sort_signature()
{
  int n = entries.size();

  for ( int gap = n/2; 0 < gap; gap /= 2) {
    for ( int i = gap; i < n; i++) {
      for ( int j = i - gap; 0 <= j; j -= gap) {
	if ( strcmp( entries[j+gap].signature, entries[j].signature) > 0) {
	  LogEntry temp = entries[j];
	  entries[j] = entries[j+gap];
	  entries[j+gap] = temp;
	}
      }
    }
  }
}

void CnvChangeLog::print()
{
  char timstr1[40];

  sort_time();
  for ( int i = 0; i < (int) entries.size(); i++) {
    if ( from) {
      if ( time_Acomp( &entries[i].time, &from_time) < 0)
	continue;
    }

    time_AtoAscii( &entries[i].time, time_eFormat_DateAndTime, timstr1, sizeof(timstr1));
    timstr1[11] = 0;
    
    printf( "%s %4s %-8s %-8s %s\n", timstr1, entries[i].signature, entries[i].module, 
	    entries[i].component, entries[i].text);
  }
}

void CnvChangeLog::print_docbook()
{
  char timstr1[40];
  pwr_tFileName fname = "$pwre_croot/src/doc/man/en_us/changelog.xml";
  dcli_translate_filename( fname, fname);

  ofstream fp( fname);

  fp <<
    "<?xml version=\"1.0\" encoding=\"iso-latin-1\"?>" << endl <<
    "<!DOCTYPE book [" << endl <<
    "<!ENTITY % isopub PUBLIC" << endl <<
    "\"ISO 8879:1986//ENTITIES Publishing//EN//XML\"" << endl <<
    "\"/usr/share/xml/entities/xml-iso-entities-8879.1986/isopub.ent\"> <!-- \"http://www.w3.org/2003/entities/iso8879/isopub.ent\"> -->" << endl <<
    "%isopub;" << endl <<
    "]>" << endl <<
    "<article>" << endl <<
    "<title>Proview Changelog</title>" << endl <<
    "<section><title>Changelog entries</title>" << endl;
#if 0
    "  <info>" << endl <<
    "    <subtitle>" << endl <<
    "    <mediaobject>" << endl <<
    "    <imageobject>" << endl <<
    "    <imagedata fileref=\"pwr_logga.gif\" width=\"5in\" depth=\"6in\"/>" << endl <<
    "    </imageobject>" << endl <<
    "    </mediaobject>" << endl <<
    "    </subtitle>" << endl <<
    "    <subtitle>Changelog</subtitle>" << endl <<
    "  </info>" << endl;
#endif

  sort_time();

  fp << "<table xml:id=\"changelog_\" border=\"1\"><tbody>" << endl <<
    "<tr><td><classname>Date________</classname></td>" << endl <<
    "<td><classname>Module_____</classname></td>" << endl <<
    "<td><classname>Change</classname></td></tr>" << endl;

  for ( int i = (int) entries.size() - 1; i >= 0; i--) {
    if ( from) {
      if ( time_Acomp( &entries[i].time, &from_time) < 0)
	continue;
    }

    time_AtoAscii( &entries[i].time, time_eFormat_DateAndTime, timstr1, sizeof(timstr1));
    timstr1[11] = 0;
    




    fp << "<tr><td>" << timstr1 << "</td><td>" << entries[i].module << "/" << entries[i].component << "</td>" << endl <<
      "<td>" << entries[i].text << " /" << entries[i].signature << "</td></tr>" << endl;

  }
  fp << "</tbody></table></section></article>" << endl;
}

void CnvChangeLog::print_html()
{
  char timstr1[40];
  pwr_tFileName fname = "$pwre_croot/src/doc/man/en_us/changelog.html";
  dcli_translate_filename( fname, fname);

  ofstream fp( fname);

  fp <<
    "<html>" << endl <<
    "  <head>" << endl <<
    "    <link rel=\"stylesheet\" type=\"text/css\" href=\"../pcss.css\">" << endl <<
    "    <title>Proview Cangelog</title>" << endl <<
    "  </head>" << endl <<
    "  <body id=\"news\">" << endl <<
    "    <div id=\"news\"><p id=\"news\">" << endl <<
    "    <h1>Proview Changlog</h1>" << endl <<
    "    <table border=\"1\" cellpadding=\"3\">" << endl;

  sort_time();

  fp << 
    "     <tr><td><b>Date</b></td>" << endl <<
    "         <td><b>Module</b></td>" << endl <<
    "         <td><b>Change</b></td>" << endl <<
    "         <td><b>Sign</b></td></tr>" << endl;

  for ( int i = (int) entries.size() - 1; i >= 0; i--) {
    if ( from) {
      if ( time_Acomp( &entries[i].time, &from_time) < 0)
	continue;
    }

    time_AtoAscii( &entries[i].time, time_eFormat_DateAndTime, timstr1, sizeof(timstr1));
    timstr1[11] = 0;
    




    fp << "<tr><td>" << timstr1 << "</td><td>" << entries[i].module << "/" << entries[i].component << "</td>" << endl <<
      "<td>" << entries[i].text << "</td><td>" << entries[i].signature << "</td></tr>" << endl;

  }
  fp << 
    "    </table></div>" << endl << 
    "  </body>" << endl <<
    "</html>" << endl;
}


