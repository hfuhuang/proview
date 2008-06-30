/* 
 * Proview   $Id: cnv_readwbl.cpp,v 1.11 2008-06-30 05:53:27 claes Exp $
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

#include <stdio.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <iostream.h>
#include <fstream.h>

extern "C" {
#include "pwr.h"
#include "co_time.h"
#include "co_dcli.h"
#include "co_cdh.h"
}
#include "co_lng.h"
#include "cnv_ctx.h"
#include "cnv_readwbl.h"
#include "cnv_wblto.h"
#include "cnv_ctx.h"

#define CNV__UNKNOWN_LINETYPE 2

static int tlog = 0;

int CnvReadWbl::read_wbl( char *filename) 
{
  int sts;
  int return_sts = 1;
  char line[400];
  char	line_part[6][80];
  int nr;
  int object_level = 0;
  int classdef_level = 0;
  char *s;
  int line_cnt = 0;

  doc_init();

  // Get source directory
  strcpy( current_file, filename);
  strcpy( source_dir, filename);
  if ( (s = strrchr( source_dir, '/')))
    *(s+1) = 0;
  else if ( (s = strrchr( source_dir, '>')))
    *(s+1) = 0;
  else if ( (s = strrchr( source_dir, ']')))
    *(s+1) = 0;
  else if ( (s = strrchr( source_dir, ':')))
    *(s+1) = 0;


  fp = fopen( filename, "r");
  if ( !fp)
    return 0;

  state = 0;
  doc_fresh = 0;

  while( 1) {
    sts = CnvCtx::read_line( line, sizeof(line), fp);
    if ( !sts)
      linetype = cread_eLine_EOF;
    else {
      line_cnt++;
      CnvCtx::remove_spaces( line, line);
      if ( strcmp( line, "") == 0)
        continue;

      if ( line[0] == '!' && 
           strncmp( line, "!/**", 4) != 0 &&
           !(state & cread_mState_Doc))
        continue;

      if ( state & cread_mState_StringAttr) {
	// Look for termination of string
	int terminated = 0;
	char *s = line; 
	while ( *s) {
	  if ( *s == '\"' && *(s-1) != '\\') {
	    terminated = 1;
	    break;
	  }
	  s++;
	}
	if ( terminated)
	  state &= ~cread_mState_StringAttr;
	continue;
      }

      nr = dcli_parse( line, " 	=", "", (char *)line_part,
                	sizeof( line_part) / sizeof( line_part[0]), 
			sizeof( line_part[0]), 0);

      if ( strcmp( low( line_part[0]), "sobject") == 0)
        linetype = cread_eLine_SObject;
      else if ( strcmp( low( line_part[0]), "endsobject") == 0)
        linetype = cread_eLine_EndSObject;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 1 &&
                strcmp( low( line_part[1]), "template") == 0)
        linetype = cread_eLine_Template;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "$classdef") == 0)
        linetype = cread_eLine_ClassDef;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "pwr_eclass_classdef") == 0)
        linetype = cread_eLine_ClassDef;
      else if ( strcmp( low( line_part[0]), "body") == 0 &&
		nr > 1 &&
                strcmp( low( line_part[1]), "sysbody") == 0)
        linetype = cread_eLine_SysBody;
      else if ( strcmp( low( line_part[0]), "dbody") == 0 &&
		nr > 1 &&
                strcmp( low( line_part[1]), "sysbody") == 0)
        linetype = cread_eLine_SysBody;
      else if ( strcmp( low( line_part[0]), "body") == 0 &&
		nr > 1 &&
                strcmp( low( line_part[1]), "rtbody") == 0)
        linetype = cread_eLine_RtBody;
      else if ( strcmp( low( line_part[0]), "body") == 0 &&
		nr > 1 &&
                strcmp( low( line_part[1]), "devbody") == 0)
        linetype = cread_eLine_DevBody;
      else if ( strcmp( low( line_part[0]), "endbody") == 0)
        linetype = cread_eLine_EndBody;
      else if ( strcmp( low( line_part[0]), "enddbody") == 0)
        linetype = cread_eLine_EndBody;
      else if ( strcmp( low( line_part[0]), "endobject") == 0)
        linetype = cread_eLine_EndObject;
      else if ( strcmp( low( line_part[0]), "enddobject") == 0)
        linetype = cread_eLine_EndObject;
      else if ( strcmp( low( line_part[0]), "attr") == 0)
        linetype = cread_eLine_Attr;
      else if ( strcmp( low( line_part[0]), "dattr") == 0)
        linetype = cread_eLine_DAttr;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "pwr_eclass_param") == 0)
        linetype = cread_eLine_Attribute;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "pwr_eclass_typedef") == 0)
        linetype = cread_eLine_TypeDef;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "$typedef") == 0)
        linetype = cread_eLine_TypeDef;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "pwr_eclass_type") == 0)
        linetype = cread_eLine_Type;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "$objbodydef") == 0)
        linetype = cread_eLine_ObjBodyDef;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "pwr_eclass_objbodydef") == 0)
        linetype = cread_eLine_ObjBodyDef;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "$input") == 0)
        linetype = cread_eLine_Input;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "$output") == 0)
        linetype = cread_eLine_Output;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "$intern") == 0)
        linetype = cread_eLine_Intern;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "$attribute") == 0)
        linetype = cread_eLine_Attribute;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "$param") == 0)
        linetype = cread_eLine_Attribute;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "$objxref") == 0)
        linetype = cread_eLine_ObjXRef;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "$graphplcnode") == 0)
        linetype = cread_eLine_GraphPlcNode;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "$graphplcconnection") == 0)
        linetype = cread_eLine_GraphPlcCon;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "$buffer") == 0)
        linetype = cread_eLine_Buffer;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "$bit") == 0)
        linetype = cread_eLine_Bit;
      else if ( strcmp( low( line_part[0]), "object") == 0 &&
		nr > 2 &&
                strcmp( low( line_part[2]), "$value") == 0)
        linetype = cread_eLine_Value;
      else if ( strcmp( low( line_part[0]), "object") == 0)
        linetype = cread_eLine_Object;
      else if ( strcmp( low( line_part[0]), "object") == 0)
        linetype = cread_eLine_Object;
      else if ( strcmp( low( line_part[0]), "!/**") == 0)
        linetype = cread_eLine_Doc;
      else if ( strcmp( low( line_part[0]), "!*/") == 0)
        linetype = cread_eLine_DocEnd;
      else if ( strcmp( low( line_part[0]), "!") == 0)
        linetype = cread_eLine_Comment;
      else if ( strcmp( low( line_part[0]), "volume") == 0)
        linetype = cread_eLine_Volume;
      else if ( strcmp( low( line_part[0]), "endvolume") == 0)
        linetype = cread_eLine_EndVolume;
      else if ( strcmp( low( line_part[0]), "range") == 0)
        linetype = cread_eLine_Range;
      else if ( strcmp( low( line_part[0]), "buffer") == 0)
        linetype = cread_eLine_Buff;
      else if ( strcmp( low( line_part[0]), "endbuffer") == 0)
        linetype = cread_eLine_EndBuff;
      else      
        linetype = cread_eLine_Unknown;

      sts = 1;
      switch( linetype) {
        case cread_eLine_Doc:
          state |= cread_mState_Doc;
          sts = object_close();
          doc_init();
          break;
        case cread_eLine_DocEnd:
          doc_close();
          state &= ~cread_mState_Doc;
          break;
        case cread_eLine_Comment:
          doc_add( line);
          break;
        case cread_eLine_SObject:
          state |= cread_mState_SObject;
          strcpy( sobject_name, line_part[1]);
          volume_init();
          break;
        case cread_eLine_ClassDef:
          state |= cread_mState_ClassDef;
	  classdef_level = object_level;
          object_state = cread_mState_ClassDef;
          class_init();
          if ( line_part[1][0] == '$')
            strcpy( class_name, &line_part[1][1]);
          else
            strcpy( class_name, line_part[1]);
          if ( nr > 3)
            strcpy( class_id, line_part[3]);
	  if ( nr > 5)
	    sprintf( class_version, "%s %s", line_part[4], line_part[5]);
	  else {
	    pwr_tTime t;
	    sts = dcli_file_time( filename, &t);
	    if ( ODD(sts))
	      time_AtoAscii( &t, time_eFormat_DateAndTime, class_version, sizeof(class_version));
	    else
	      strcpy( class_version, "");
	  }
	  if (tlog) printf( "Cd %7d %3d %s\n", line_cnt, object_level, line);
          break;
        case cread_eLine_TypeDef:
          state |= cread_mState_TypeDef;
          object_state = cread_mState_TypeDef;
	  classdef_level = object_level;
          typedef_init();
          if ( line_part[1][0] == '$')
            strcpy( typedef_name, &line_part[1][1]);
          else
            strcpy( typedef_name, line_part[1]);
          break;
        case cread_eLine_Type:
          state |= cread_mState_Type;
          object_state = cread_mState_Type;
          typedef_init();
          if ( line_part[1][0] == '$')
            strcpy( typedef_name, &line_part[1][1]);
          else
            strcpy( typedef_name, line_part[1]);
          break;
        case cread_eLine_Bit:
          state |= cread_mState_Bit;
          object_state = cread_mState_Bit;
          strcpy( bit_name, line_part[1]);
          strcpy( bit_type, "Bit");
	  bit_init();
          object_level++;
          break;
        case cread_eLine_Value:
          state |= cread_mState_Value;
          object_state = cread_mState_Value;
          strcpy( bit_name, line_part[1]);
          strcpy( bit_type, "Value");
	  bit_init();
          object_level++;
          break;
        case cread_eLine_SysBody:
          state |= cread_mState_SysBody;
          break;
        case cread_eLine_RtBody:
          state |= cread_mState_RtBody;
          break;
        case cread_eLine_DevBody:
          state |= cread_mState_DevBody;
          break;
        case cread_eLine_EndBody:
          sts = object_close();
          state &= ~cread_mState_SysBody;
          state &= ~cread_mState_RtBody;
          state &= ~cread_mState_DevBody;
          break;
        case cread_eLine_ObjBodyDef:
          sts = object_close();
          state |= cread_mState_ObjBodyDef;
          object_state = cread_mState_ObjBodyDef;
          body_init();
          strcpy( ObjBodyDef, line_part[1]);
          strcpy( body_name, line_part[1]);
          break;
        case cread_eLine_Template:
          state |= cread_mState_Template;
          object_state = cread_mState_Template;
          template_init();
          break;
        case cread_eLine_DParam:
          sts = object_close();
          state |= cread_mState_DParam;
          object_state = cread_mState_DParam;
          strcpy( attr_name, line_part[1]);
          strcpy( attr_type, "Attribute");
          attribute_init();
          break;
        case cread_eLine_Input:
          sts = object_close();
          state |= cread_mState_Input;
          object_state = cread_mState_Input;
          strcpy( attr_name, line_part[1]);
          strcpy( attr_type, "Input");
          attribute_init();
          break;
        case cread_eLine_Output:
          sts = object_close();
          state |= cread_mState_Output;
          object_state = cread_mState_Output;
          strcpy( attr_name, line_part[1]);
          strcpy( attr_type, "Output");
          attribute_init();
          break;
        case cread_eLine_Intern:
          sts = object_close();
          state |= cread_mState_Intern;
          object_state = cread_mState_Intern;
          strcpy( attr_name, line_part[1]);
          strcpy( attr_type, "Intern");
          attribute_init();
          break;
        case cread_eLine_Attribute:
          sts = object_close();
          state |= cread_mState_Attribute;
          object_state = cread_mState_Attribute;
          strcpy( attr_name, line_part[1]);
          strcpy( attr_type, "Attribute");
          attribute_init();
          break;
        case cread_eLine_ObjXRef:
          sts = object_close();
          state |= cread_mState_ObjXRef;
          object_state = cread_mState_ObjXRef;
          strcpy( attr_name, line_part[1]);
          strcpy( attr_type, "ObjXRef");
          attribute_init();
          strcpy( attr_typeref, "Objid");
          break;
        case cread_eLine_EndSObject:
          state &= ~cread_mState_SObject;
          break;
        case cread_eLine_EndObject:
          if ( state & cread_mState_Input)
            state &= ~cread_mState_Input;
          else if ( state & cread_mState_Output)
            state &= ~cread_mState_Output;
          else if ( state & cread_mState_Intern)
            state &= ~cread_mState_Intern;
          else if ( state & cread_mState_Attribute)
            state &= ~cread_mState_Attribute;
          else if ( state & cread_mState_DParam)
            state &= ~cread_mState_DParam;
          else if ( state & cread_mState_ObjXRef)
            state &= ~cread_mState_ObjXRef;
          else if ( state & cread_mState_Buffer)
            state &= ~cread_mState_Buffer;
          else if ( state & cread_mState_ObjBodyDef) {
            state &= ~cread_mState_ObjBodyDef;
            if ( wblto->type() == Cnv_eWblToType_Html ||
		 wblto->type() == Cnv_eWblToType_H)
	      wblto->body_close();
          }
          else if ( state & cread_mState_GraphPlcNode) {
            state &= ~cread_mState_GraphPlcNode;
            if ( wblto->type() == Cnv_eWblToType_Html)
              wblto->body_close();
          }
	  else if ( state & cread_mState_GraphPlcCon) {
            state &= ~cread_mState_GraphPlcCon;
            if ( wblto->type() == Cnv_eWblToType_Html)
              wblto->body_close();
          }
          else if ( state & cread_mState_Template) {
            state &= ~cread_mState_Template;
	    if ( wblto->type() == Cnv_eWblToType_Html)
              wblto->body_close();
          }
          else if ( state & cread_mState_ClassDef && classdef_level == object_level) {
	    if (tlog) printf( "Cd %7d %3d %s\n", line_cnt, object_level, line);
            state &= ~cread_mState_ClassDef;
            if ( wblto->type() == Cnv_eWblToType_Html &&
		 wblto->class_open() &&
		 classdef_level == object_level)
              wblto->class_close();
            if ( wblto->type() == Cnv_eWblToType_H &&
		 wblto->class_open() &&
		 classdef_level == object_level)
              wblto->class_close();
          }
          else if ( state & cread_mState_Bit) {
            state &= ~cread_mState_Bit;
	    // object_state = cread_mState_TypeDef;
	    object_level--;
          }
          else if ( state & cread_mState_Value) {
            state &= ~cread_mState_Value;
	    // object_state = cread_mState_TypeDef;
	    object_level--;
          }
	  else if ( state & cread_mState_TypeDef && classdef_level == object_level) {
            state &= ~cread_mState_TypeDef;
            if ( wblto->type() == Cnv_eWblToType_Html &&
		 wblto->class_open())
              wblto->typedef_close();
            if ( wblto->type() == Cnv_eWblToType_H &&
		 wblto->class_open())
              wblto->typedef_close();
          }
	  else if ( state & cread_mState_Type) {
            state &= ~cread_mState_Type;
#if 0
            if ( generate_html && html_class_open)
              html_class_close();
	    if ( generate_struct && struct_class_open)
              struct_class_close();
#endif
          }
	  else if ( state & cread_mState_Object) {
            object_level--;
            if ( object_level <= 0)
              state &= ~cread_mState_Object;
	    if (tlog) printf( "Oo %7d %3d %s\n", line_cnt, object_level, line);
          }
          else
            printf( "Error: mismatch in Object-Endobject nesting, at line %d\n",
			line_cnt);
          break;
        case cread_eLine_Attr: {
	  char attr_name[80];
          char attr_value[100];
          int oreq = 0;

          if ( strcmp( line_part[2], "|") == 0) {
            strcpy( line_part[2], line_part[3]);
            oreq = 1;
          }

          strcpy( attr_name, line_part[1]);
          strcpy( attr_value, line_part[2]);
          switch ( object_state) {
            case cread_mState_Input:
            case cread_mState_Output:
            case cread_mState_Intern:
            case cread_mState_Attribute:
            case cread_mState_ObjXRef:
            case cread_mState_Buffer:
              attribute_attr( attr_name, attr_value);
              break;
            case cread_mState_GraphPlcNode:
              graphplcnode_attr( attr_name, attr_value);
              break;
            case cread_mState_GraphPlcCon:
              graphplccon_attr( attr_name, attr_value);
              break;
            case cread_mState_ClassDef:
              class_attr( attr_name, attr_value);
              break;
            case cread_mState_TypeDef:
            case cread_mState_Type:
              typedef_attr( attr_name, attr_value);
              break;
            case cread_mState_Bit:
            case cread_mState_Value:
              bit_attr( attr_name, attr_value);
              break;
            case cread_mState_ObjBodyDef:
              body_attr( attr_name, attr_value);
              break;
            case cread_mState_Template:
              template_attr( attr_name, attr_value);
              break;
            default:
              ;
          }

	  // Check if unterminated string
	  int terminated = 1;
	  char *s = line;
	  while ( *s) {
	    if ( *s == '\"' && *(s-1) != '\\')
	      terminated = !terminated;
	    s++;
	  }
	  if ( !terminated)
	    state |= cread_mState_StringAttr;

          break;
        }
        case cread_eLine_DAttr: {
	  char attr_name[80];
          char attr_value[100];
          int oreq = 0;

          if ( strcmp( line_part[3], "|") == 0) {
            strcpy( line_part[3], line_part[4]);
            oreq = 1;
          }

          strcpy( attr_name, line_part[1]);
          strcpy( attr_value, line_part[3]);
          switch ( object_state) {
            case cread_mState_DParam:
            case cread_mState_Input:
            case cread_mState_Output:
            case cread_mState_Intern:
            case cread_mState_Attribute:
            case cread_mState_ObjXRef:
            case cread_mState_Buffer:
              attribute_attr( attr_name, attr_value);
              break;
            case cread_mState_GraphPlcNode:
              graphplcnode_attr( attr_name, attr_value);
              break;
            case cread_mState_GraphPlcCon:
              graphplccon_attr( attr_name, attr_value);
              break;
            case cread_mState_ClassDef:
              class_attr( attr_name, attr_value);
              break;
            case cread_mState_TypeDef:
            case cread_mState_Type:
              typedef_attr( attr_name, attr_value);
              break;
            case cread_mState_ObjBodyDef:
              body_attr( attr_name, attr_value);
              break;
            case cread_mState_Template:
              template_attr( attr_name, attr_value);
              break;
            default:
              ;
          }
          break;
        }
        case cread_eLine_Buffer: {
          sts = object_close();
          state |= cread_mState_Buffer;
          object_state = cread_mState_Buffer;
          strcpy( attr_name, line_part[1]);
          strcpy( attr_type, "Buffer");
          attribute_init();
          break;
        }
        case cread_eLine_GraphPlcNode: {
          sts = object_close();
          state |= cread_mState_GraphPlcNode;
          object_state = cread_mState_GraphPlcNode;
          graphplcnode_init();
          strcpy( graphplcnode_name, line_part[1]);
          break;
        }
        case cread_eLine_GraphPlcCon: {
          sts = object_close();
          state |= cread_mState_GraphPlcCon;
          object_state = cread_mState_GraphPlcCon;
          graphplccon_init();
          strcpy( graphplccon_name, line_part[1]);
          break;
        }
        case cread_eLine_Object: {
          sts = object_close();
          state |= cread_mState_Object;
          object_state = cread_mState_Object;
          object_level++;
	  if (tlog) printf( "Oo %7d %3d %s\n", line_cnt, object_level, line);
          break;
        }
        case cread_eLine_Volume: {
          state |= cread_mState_Volume;
	  strcpy( volume_name, line_part[1]);
          break;
        }
        case cread_eLine_EndVolume: {
          if ( state & cread_mState_Volume)
            state &= ~cread_mState_Volume;
          break;
        }
        case cread_eLine_Range:
          break;
        case cread_eLine_Buff:
        case cread_eLine_EndBuff:
          break;
        case cread_eLine_EOF:
          break;

        default:
          printf( "Error, unknown linetype, %s, at line %d\n", filename, line_cnt);
          return sts = CNV__UNKNOWN_LINETYPE;
      }
      if ( EVEN(sts)) {
        printf( ", at line %d\n", line_cnt);
        return_sts = sts;
      }
    }

    if ( linetype ==  cread_eLine_EOF)
      break;
  }

  if ( wblto->type() == Cnv_eWblToType_Html &&
       wblto->class_open())
    wblto->class_close();
  if ( wblto->type() == Cnv_eWblToType_H &&
       wblto->class_open())
    wblto->class_close();
  if ( wblto->type() == Cnv_eWblToType_Xtthelp &&
       wblto->index_open())
    wblto->class_close();
  if ( wblto->type() == Cnv_eWblToType_Ps)
    wblto->class_close();

  fclose(fp);
  return return_sts;
}


void CnvReadWbl::attribute_init()
{
  strcpy( attr_flags, "");
  strcpy( attr_typeref, "");
  strcpy( attr_typeref_volume, "");
  strcpy( attr_pgmname, "");
  strcpy( attr_elements, "");
  attr_pointer = 0;
  attr_array = 0;
  attr_rtvirtual = 0;
  attr_elem = 0;
  attr_isclass = 0;
}

int CnvReadWbl::attribute_attr( char *name, char *value)
{
  char 	*s;
  int	nr;

  if ( strcmp( low( name), "typeref") == 0) {
    if ((s = strrchr( value, '-'))) {
      s++;
      if ( *s == '$')
        s++;
      strcpy( attr_typeref, s);
      strcpy( attr_typeref_volume, value);
      if ( (s = strchr( attr_typeref_volume, ':')))
	*s = 0;
      else
	strcpy( attr_typeref_volume, "");
    }
    else
      strcpy( attr_typeref, value);
  }
  else if ( strcmp( low( name), "flags") == 0) {
    if ( strncmp( value, "PWR_MASK_", 9) == 0) {
      if ( strcmp( attr_flags, "") != 0)
	strcat( attr_flags, " | ");
      strcat( attr_flags, &value[9]);

      if ( strcmp( value, "PWR_MASK_POINTER") == 0)
	attr_pointer = 1;
      if ( strcmp( value, "PWR_MASK_ARRAY") == 0)
	attr_array = 1;
      if ( strcmp( value, "PWR_MASK_RTVIRTUAL") == 0)
	attr_rtvirtual = 1;
      if ( strcmp( value, "PWR_MASK_CLASS") == 0)
	attr_isclass = 1;
    }
    else {
      int flags_value;

      nr = sscanf( value, "%d", &flags_value);
      if ( nr != 1) {
	return 1;
      }
      if ( strcmp( attr_flags, "") != 0)
	strcat( attr_flags, " | ");
      strcat( attr_flags, flags_to_string( flags_value));

      if ( flags_value & pwr_mAdef_pointer)
	attr_pointer = 1;
      if ( flags_value & pwr_mAdef_array)
	attr_array = 1;
      if ( flags_value & pwr_mAdef_rtvirtual)
	attr_rtvirtual = 1;
      if ( flags_value & pwr_mAdef_class)
	attr_isclass = 1;
    }
  }
  else if ( strcmp( low( name), "elements") == 0) {
    strcpy( attr_elements, value);
    nr = sscanf( attr_elements, "%d", &attr_elem);
    if ( nr == 0)
      attr_elem = 0;    
  }
  else if ( strcmp( low( name), "class") == 0) {
    strcpy( attr_typeref, value);
  }
  else if ( strcmp( low( name), "pgmname") == 0) {
    strcpy( attr_pgmname, value);
  }
  return 1;
}

int CnvReadWbl::attribute_close()
{
  int sts;

  sts = wblto->attribute_exec();

  doc_fresh = 0;
  return sts;
}

int CnvReadWbl::bit_close()
{
  int sts;

  sts = wblto->bit_exec();

  doc_fresh = 0;
  return sts;
}

void CnvReadWbl::class_init()
{
  strcpy( class_name, "");
  strcpy( class_editor, "");
  strcpy( class_method, "");
  strcpy( class_popeditor, "");
  strcpy( class_flags, "");
  class_devonly = 0;
}

int CnvReadWbl::class_attr( char *name, char *value)
{
  if ( strcmp( low( name), "editor") == 0) {
    if ( strncmp( value, "pwr_eEditor_", 12) == 0)
      strcpy( class_editor, &value[12]);
    else
      strcpy( class_editor, value);

  }
  else if ( strcmp( low( name), "method") == 0) {
    if ( strncmp( value, "pwr_eMethod_", 12) == 0)
      strcpy( class_method, &value[12]);
    else
      strcpy( class_method, value);

  }
  else if ( strcmp( low( name), "popeditor") == 0) {
    strcpy( class_method, value);
  }
  else if ( strcmp( low( name), "flags") == 0) {
    if ( strcmp( class_flags, "") != 0)
      strcat( class_flags, " | ");
    if ( strncmp( value, "pwr_mClassDef_", 14) == 0)
      strcat( body_flags, &value[14]);
    else
      strcat( body_flags, value);

    if ( strcmp( value, "pwr_mClassDef_DevOnly") == 0)
      class_devonly = 1;
  }
  return 1;
}

int CnvReadWbl::class_close()
{
  if ( ctx->first_class) {
    wblto->init( class_name);

    ctx->first_class = 0;
  }

  wblto->class_exec();

  doc_fresh = 0;
  return 1;
}

void CnvReadWbl::body_init()
{
  strcpy( body_name, "");
  strcpy( body_structname, "");
  strcpy( body_flags, "");
  body_rtvirtual = 0;
}

int CnvReadWbl::body_attr( char *name, char *value)
{
  if ( strcmp( low( name), "structname") == 0) {
    strcpy( body_structname, value);
  }
  else if ( strcmp( low( name), "flags") == 0) {
    if ( strcmp( body_flags, "") != 0)
      strcat( body_flags, " | ");
    if ( strncmp( value, "pwr_mObjBodyDef_", 16) == 0)
      strcat( body_flags, &value[9]);
    else
      strcat( body_flags, value);

    if ( strcmp( value, "pwr_mObjBodyDef_RtVirtual") == 0)
      body_rtvirtual = 1;
  }
  return 1;
}

int CnvReadWbl::body_close()
{
  int sts;

  if ( wblto->type() == Cnv_eWblToType_H) {
    sts = wblto->body_exec();
    if ( EVEN(sts)) return sts;
  }
  else
    wblto->body_exec();

  doc_fresh = 0;
  return 1;
}

void CnvReadWbl::graphplcnode_init()
{
  doc_cnt = 0;
}

int CnvReadWbl::graphplcnode_attr( char *name, char *value)
{
  strcpy( doc_text[doc_cnt++], name);
  strcpy( doc_text[doc_cnt++], value);

  // Description of methods
  if ( strcmp( low( name), "graphmethod") == 0) {
    if ( strcmp( value, "0") == 0)
      strcat( doc_text[doc_cnt-1], " (standard, individual attributes)");
    else if ( strcmp( value, "1") == 0)
      strcat( doc_text[doc_cnt-1], " (standard, common attributes)");
    else if ( strcmp( value, "2") == 0)
      strcat( doc_text[doc_cnt-1], " (standard, two textfield)");
    else if ( strcmp( value, "3") == 0)
      strcat( doc_text[doc_cnt-1], " (text)");
    else if ( strcmp( value, "4") == 0)
      strcat( doc_text[doc_cnt-1], " (special)");
    else if ( strcmp( value, "5") == 0)
      strcat( doc_text[doc_cnt-1], " (grafcet order)");
    else if ( strcmp( value, "6") == 0)
      strcat( doc_text[doc_cnt-1], " (document)");
    else if ( strcmp( value, "7") == 0)
      strcat( doc_text[doc_cnt-1], " (Get,Set)");
  }
  else if ( strcmp( low( name), "compmethod") == 0)
    ;
  else if ( strcmp( low( name), "tracemethod") == 0)
    ;
  else if ( strcmp( low( name), "executeordermethod") == 0)
    ;
  return 1;
}

int CnvReadWbl::graphplcnode_close()
{
  wblto->graphplcnode();

  doc_fresh = 0;
  return 1;
}
void CnvReadWbl::graphplccon_init()
{
  doc_cnt = 0;
}

int CnvReadWbl::graphplccon_attr( char *name, char *value)
{
  strcpy( doc_text[doc_cnt++], name);
  strcpy( doc_text[doc_cnt++], value);

  // Description
  if ( strcmp( low( name), "curvature") == 0)
    ;
  else if ( strcmp( low( name), "corners") == 0)
    ;
  else if ( strcmp( low( name), "color") == 0)
    ;
  else if ( strcmp( low( name), "attributes") == 0)
    ;
  return 1;
}

int CnvReadWbl::graphplccon_close()
{
  wblto->graphplccon();

  doc_fresh = 0;
  return 1;
}
void CnvReadWbl::template_init()
{
  doc_cnt = 0;
}

int CnvReadWbl::template_attr( char *name, char *value)
{
  int doc_size = (int) sizeof(doc_text) / sizeof(doc_text[0]);
  if ( doc_cnt >= doc_size) {
    strcpy( doc_text[doc_size - 2], "Template buffer size exceeded");
    strcpy( doc_text[doc_size - 1], "");
    return 1;
  }
  strcpy( doc_text[doc_cnt++], name);
  strcpy( doc_text[doc_cnt++], value);
  return 1;
}

int CnvReadWbl::template_close()
{
  wblto->template_exec();
  doc_fresh = 0;
  return 1;
}

void CnvReadWbl::volume_init()
{
  char *s;

  strcpy( volume_name, sobject_name);
  if ( (s = strchr( volume_name, ':')))
    *s = 0;

}

void CnvReadWbl::doc_init()
{
  doc_cnt = 0;
  strcpy( doc_author, "");
  strcpy( doc_version, "");
  strcpy( doc_code, "");
  strcpy( doc_summary, "");
  memset( doc_clink_ref, 0, sizeof(doc_clink_ref));
  memset( doc_clink_text, 0, sizeof(doc_clink_text));
  memset( doc_link_ref, 0, sizeof(doc_link_ref));
  memset( doc_link_text, 0, sizeof(doc_link_text));
  memset( doc_groups, 0, sizeof(doc_groups));
  doc_clink_cnt = 0;
  doc_xlink_cnt = 0;
  doc_link_cnt = 0;
  doc_group_cnt = 0;
}

void CnvReadWbl::doc_init_keep()
{
  doc_cnt = 0;
  strcpy( doc_summary, "");
}

int CnvReadWbl::doc_add( char *line)
{
  char	line_part[4][80];
  int nr;
  int i;

  nr = dcli_parse( line, " 	=", "", (char *)line_part,
                	sizeof( line_part) / sizeof( line_part[0]), 
			sizeof( line_part[0]), 0);

  if ( strcmp( low(line_part[1]), "@author") == 0) {
    for ( i = 2; i < nr; i++) {
      if ( i != 2)
        strcat( doc_author, " ");
      strcat( doc_author, line_part[i]);
    }
  }
  else if ( strcmp( low(line_part[1]), "@version") == 0) {
    for ( i = 2; i < nr; i++) {
      if ( i != 2)
        strcat( doc_version, " ");
      strcat( doc_version, line_part[i]);
    }
  }
  else if ( strcmp( low(line_part[1]), "@group") == 0) {
    char str[400];
    
    CnvCtx::remove_spaces( line, str);
    CnvCtx::remove_spaces( &str[6], str);
    doc_group_cnt = dcli_parse( str, " 	,", "", (char *)doc_groups,
                	sizeof( doc_groups) / sizeof( doc_groups[0]), 
			sizeof( doc_groups[0]), 0);
  }
  else if ( strcmp( low(line_part[1]), "@link") == 0) {
    if ( doc_link_cnt >= (int) (sizeof(doc_link_ref)/sizeof(doc_link_ref[0]))) {
      printf("Error: max number of links exceeded\n");
      return 1;
    }
    for ( i = 2; i < nr; i++) {
      if ( i == nr - 1)
        strcpy( doc_link_ref[doc_link_cnt], line_part[i]);
      else {
        if ( i == 2)
	  strcpy( doc_link_text[doc_link_cnt], line_part[i]);
	else {
          strcat( doc_link_text[doc_link_cnt], " ");
	  strcat( doc_link_text[doc_link_cnt], line_part[i]);
	}
      }
    }
    doc_link_cnt++;
  }
  else if ( strcmp( low(line_part[1]), "@classlink") == 0) {
    if ( doc_clink_cnt >= (int) (sizeof(doc_clink_ref)/sizeof(doc_clink_ref[0]))) {
      printf("Error: max number of classlinks exceeded\n");
      return 1;
    }
    for ( i = 2; i < nr; i++) {
      if ( i == nr - 1)
        strcpy( doc_clink_ref[doc_clink_cnt], line_part[i]);
      else {
        if ( i == 2)
	  strcpy( doc_clink_text[doc_clink_cnt], line_part[i]);
	else {
          strcat( doc_clink_text[doc_clink_cnt], " ");
	  strcat( doc_clink_text[doc_clink_cnt], line_part[i]);
	}
      }
    }
    doc_clink_cnt++;
  }
  else if ( strcmp( low(line_part[1]), "@exliblink") == 0) {
    if ( doc_xlink_cnt >= (int) (sizeof(doc_xlink_ref)/sizeof(doc_xlink_ref[0]))) {
      printf("Error: max number of classlinks exceeded\n");
      return 1;
    }
    for ( i = 2; i < nr; i++) {
      if ( i == nr - 1)
        strcpy( doc_xlink_ref[doc_xlink_cnt], line_part[i]);
      else {
        if ( i == 2)
	  strcpy( doc_xlink_text[doc_xlink_cnt], line_part[i]);
	else {
          strcat( doc_xlink_text[doc_xlink_cnt], " ");
	  strcat( doc_xlink_text[doc_xlink_cnt], line_part[i]);
	}
      }
    }
    doc_xlink_cnt++;
  }
  else if ( strcmp( low(line_part[1]), "@code") == 0) {
    if ( nr > 2)
      strcpy( doc_code, line_part[2]);
  }
  else if ( strcmp( low(line_part[1]), "@summary") == 0) {
    char low_line[400];
    char *s;

    cdh_ToLower( low_line, line);
    s = strstr( low_line, "@summary");
    if ( !s)
      return 0;

    strcpy( doc_summary, &line[s - low_line + 9]);
  }
  else {
    if ( doc_cnt > int(sizeof(doc_text)/sizeof(doc_text[0]) - 1))
      return 0;
    strcpy( doc_text[doc_cnt], &line[1]);
    doc_cnt++;
  }
  return 1;
}

int CnvReadWbl::doc_close()
{
  doc_fresh = 1;
  return 1;
}

int CnvReadWbl::object_close()
{
  int sts;

  switch ( object_state) {
    case 0:
      break;
    case cread_mState_Input:
    case cread_mState_Output:
    case cread_mState_Intern:
    case cread_mState_Attribute:
    case cread_mState_ObjXRef:
    case cread_mState_Buffer:
    case cread_mState_DParam:
      sts = attribute_close();
      if ( EVEN(sts)) return sts;
      break;
    case cread_mState_ObjBodyDef:
      sts = body_close();
      if ( EVEN(sts)) return sts;
      break;
    case cread_mState_GraphPlcNode:
      graphplcnode_close();
      break;
    case cread_mState_GraphPlcCon:
      graphplccon_close();
      break;
    case cread_mState_Template:
      template_close();
      break;
    case cread_mState_ClassDef:
      class_close();
      break;
    case cread_mState_TypeDef:
    case cread_mState_Type:
      typedef_close();
      break;
    case cread_mState_Bit:
    case cread_mState_Value:
      bit_close();
      break;
    case cread_mState_Object:
      break;
    default:
      ;
  }
  object_state = 0;
  return 1;
}

void CnvReadWbl::typedef_init()
{
  strcpy( typedef_name, "");
  strcpy( typedef_typeref, "");
  strcpy( typedef_pgmname, "");
  typedef_elements = 0;
}

void CnvReadWbl::bit_init()
{
  strcpy( bit_text, "");
  strcpy( bit_pgmname, "");
  bit_value = 0;
}

int CnvReadWbl::typedef_attr( char *name, char *value)
{
  if ( strcmp( low( name), "typeref") == 0) {
    if ( strncmp( value, "pwrs:Type-$", 11) == 0)
      strcpy( typedef_typeref, &value[11]);
    else
      strcpy( typedef_typeref, value);

  }
  if ( strcmp( low( name), "pgmname") == 0) {
    strcpy( typedef_pgmname, value);
  }
  else if ( strcmp( low( name), "elements") == 0) {
    sscanf( value, "%d", &typedef_elements);
  }
  return 1;
}

int CnvReadWbl::bit_attr( char *name, char *value)
{
  if ( strcmp( low( name), "text") == 0) {
    strcpy( bit_text, value);
  }
  else if ( strcmp( low( name), "pgmname") == 0) {
    strcpy( bit_pgmname, value);
  }
  else if ( strcmp( low( name), "value") == 0) {
    sscanf( value, "%u", &bit_value);
  }
  return 1;
}

int CnvReadWbl::typedef_close()
{

  if ( ctx->first_class) {
    wblto->init( typedef_name);
    
    ctx->first_class = 0;
  }

  if ( wblto->type() == Cnv_eWblToType_Html ||
       wblto->type() == Cnv_eWblToType_Ps)
    wblto->typedef_exec();
  if ( wblto->type() == Cnv_eWblToType_H &&
       object_state == cread_mState_TypeDef)
    wblto->typedef_exec();

  doc_fresh = 0;
  return 1;
}


char *CnvReadWbl::low( char *in)
{
  static char str[400];

  cdh_ToLower( str, in);
  return str;
}

int CnvReadWbl::copy_tmp_file( char *tmpfilename, ofstream& fp_to)
{
  FILE *fp;
  char c;
  char cmd[80];

  fp = fopen( tmpfilename, "r");
  if ( !fp)
    return 0;
  while( (c = fgetc( fp)) != EOF)
    fp_to.put(c);
  fclose(fp);

#if defined OS_VMS
  sprintf( cmd, "delete/noconf/nolog %s.*", tmpfilename);
#else
  sprintf( cmd, "rm %s", tmpfilename);
#endif
  system( cmd);

  return 1;
}

char *CnvReadWbl::flags_to_string( int value)
{
  static char str[512];

  strcpy( str, "");
  if ( value & pwr_mAdef_pointer) 	strcat( str, "Pointer|");
  if ( value & pwr_mAdef_array) 	strcat( str, "Array|");
  if ( value & pwr_mAdef_backup) 	strcat( str, "Backup|");
  if ( value & pwr_mAdef_castattr) 	strcat( str, "CastAttr|");
  if ( value & pwr_mAdef_state) 	strcat( str, "State|");
  if ( value & pwr_mAdef_const) 	strcat( str, "Const|");
  if ( value & pwr_mAdef_rtvirtual) 	strcat( str, "Rtvirtual|");
  if ( value & pwr_mAdef_devbodyref) 	strcat( str, "Devbodyref|");
  if ( value & pwr_mAdef_dynamic) 	strcat( str, "Dynamic|");
  if ( value & pwr_mAdef_objidself) 	strcat( str, "Objidself|");
  if ( value & pwr_mAdef_noedit) 	strcat( str, "Noedit|");
  if ( value & pwr_mAdef_invisible) 	strcat( str, "Invisible|");
  if ( value & pwr_mAdef_refdirect) 	strcat( str, "Refdirect|");
  if ( value & pwr_mAdef_noinvert) 	strcat( str, "Noinvert|");
  if ( value & pwr_mAdef_noremove) 	strcat( str, "Noremove|");
  if ( value & pwr_mAdef_rtdbref) 	strcat( str, "Rtdbref|");
  if ( value & pwr_mAdef_private) 	strcat( str, "Private|");
  if ( value & pwr_mAdef_class) 	strcat( str, "Class|");
  if ( value & pwr_mAdef_superclass) 	strcat( str, "Superclass|");
  if ( value & pwr_mAdef_buffer) 	strcat( str, "Buffer|");
  if ( value & pwr_mAdef_nowbl) 	strcat( str, "Nowbl|");
  if ( value & pwr_mAdef_alwayswbl) 	strcat( str, "Alwayswbl|");
  if ( value & pwr_mAdef_disableattr) 	strcat( str, "DisableAttr|");
  if ( value & pwr_mAdef_rthide) 	strcat( str, "RtHide|");
  if ( str[strlen(str)-1] == '|')
    str[strlen(str)-1] = 0;
  return str;
}


int CnvReadWbl::read_lng( char *cname, char *aname)
{
  char line[400];
  char str[400];
  char key[80];
  char found_key[80];
  int in_class;
  int in_doc;
  int sts;
  int len;
  FILE *fp;
  pwr_tFileName filename = "pwrb_sv_se.txt";

  sprintf( filename, "%s/%s_%s.txt", source_dir, CnvCtx::low(volume_name),
	   Lng::get_language_str());

  strcpy( key, cname);
  if ( aname && strcmp(aname, "") != 0) {
    strcat( key, "-");
    strcat( key, aname);
  }

  dcli_translate_filename( filename, filename);
  fp = fopen( filename, "r");
  if ( !fp) return 0;

  in_class = 0;
  in_doc = 0;
  while( 1) {
    sts = CnvCtx::read_line( line, sizeof(line), fp);
    if ( !sts) break;

    CnvCtx::remove_spaces( line, str);
    if ( cdh_NoCaseStrncmp( str, "<class>", 7) == 0)
      len = 7;
    else if ( cdh_NoCaseStrncmp( str, "<type>", 6) == 0)
      len = 6;
    else if ( cdh_NoCaseStrncmp( str, "<typedef>", 9) == 0)
      len = 9;
    else
      len = 0;

    if ( len) {
      CnvCtx::remove_spaces( &str[len], found_key);
      if ( cdh_NoCaseStrcmp( cname, found_key) == 0) {
	if ( aname && strcmp(aname, "") != 0) {
	  in_class = 1;
	}
	else {
 	  in_doc = 1;
	  in_class = 1;
	  doc_init_keep();
	}
      }
      continue;
    }
    else if ( in_class && 
	      (cdh_NoCaseStrncmp( str, "</class>", 8) == 0 ||
	       cdh_NoCaseStrncmp( str, "</type>", 7) == 0 ||
	       cdh_NoCaseStrncmp( str, "</typedef>", 10) == 0))
      break;
    else if ( in_class && 
	      (cdh_NoCaseStrncmp( str, "<attr>", 6) == 0 ||
	       cdh_NoCaseStrncmp( str, "<value>", 7) == 0)) {
      if ( cdh_NoCaseStrncmp( str, "<attr>", 6) == 0)
	len = 6;
      else
	len = 7;

      if ( !in_class)
	continue;
      if ( in_doc)
	break;

      CnvCtx::remove_spaces( &str[len], found_key);
      if ( cdh_NoCaseStrcmp( aname, found_key) == 0) {
	in_doc = 1;
	doc_init_keep();
      }
    }
    else if ( in_doc && 
	      (cdh_NoCaseStrncmp( str, "</attr>", 8) == 0 ||
	       cdh_NoCaseStrncmp( str, "</value>", 9) == 0)) {
      if ( !in_class)
	continue;
      break;
    }
    else if ( in_doc) {
      strcpy( str, "! ");
      strcat( str, line);
      doc_add( str);
    }
  }
  fclose( fp);
  if ( in_doc) {
    doc_close();
    return 1;
  }
  return 0;
}
