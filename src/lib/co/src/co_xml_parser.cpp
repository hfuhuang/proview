/* 
 * Proview   $Id$
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

/* co_xml_parser.cpp -- Parse xml file */

#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "co_dcli.h"
#include "co_dcli_msg.h"
#include "co_xml_parser.h"

typedef enum {
  xml_eState_Init 		= 1 << 0,
  xml_eState_Tag		= 1 << 1,
  xml_eState_MetaTag		= 1 << 2,
  xml_eState_TagNameFound    	= 1 << 3,
  xml_eState_TagName		= 1 << 4,
  xml_eState_Attribute	= 1 << 5,
  xml_eState_AttributeName	= 1 << 6,
  xml_eState_AttributeNameFound = 1 << 7,
  xml_eState_AttributeValue	= 1 << 8,
  xml_eState_AttributeValueFound = 1 << 9,
  xml_eState_TagValue		= 1 << 10,
  xml_eState_TagValueFound   	= 1 << 11,
  xml_eState_EndTag 		= 1 << 12
} xml_eState;

co_xml_parser::co_xml_parser( co_xml_interpreter *i) : 
  interpreter(i),logglevel(0), first_token(true), 
  state( xml_eState_Init), line_cnt(1), 
  char_cnt(0), in_comment(false), c(0), c_f(0), c_ff(0), c_sts(0), c_f_sts(0), 
  c_ff_sts((void *)1), current_tag_idx(0), current_attribute_name_idx(0),
  current_attribute_value_idx(0), current_tag_value_idx(0)
{
  interpreter->parser = this;
}

void *co_xml_parser::next_token()
{
  char t;
  void *sts;

  if ( first_token) {
    first_token = false;

    c_sts = fp.get(c);
    if ( !c_sts) return c_sts;
    
    c_f_sts = fp.get(c_f);
    if ( c_f_sts)
      c_ff_sts = fp.get(c_ff);
  }
  else {
        
    if ( c_ff_sts)
      sts = fp.get(t);

    c = c_f;
    c_sts = c_f_sts;
    c_f = c_ff;
    c_f_sts = c_ff_sts;
    if ( c_ff_sts)
      c_ff_sts = sts;
    if ( sts)
      c_ff = t;
  }
  if ( c_sts) {
    if ( c == '\n') {
      line_cnt++;
      char_cnt = 1;
    }
    else
      char_cnt++;
  }
  return c_sts;
}

void co_xml_parser::error_message_line( const char *msg)
{
  printf( "** XML-parser error, %s, line %d, %d\n", msg, line_cnt, char_cnt);
}

void co_xml_parser::error_message( const char *format, const char *value)
{
  printf( "** XML-parser error, ");
  printf( format, value);
  printf( "\n");
}

bool co_xml_parser::is_space( const char c)
{
  return c == ' ' || c == '	' || c == '\n' || c == '\r';
}

int co_xml_parser::read( const char *filename)
{
  int sts;

  fp.open( filename);
  if ( !fp)
    return DCLI__NOFILE;

  while ( next_token()) {
    if ( in_comment) {
      if ( c == '-' && c_f == '-' && c_ff == '>') {
	// End of comment
	in_comment = false;
	next_token();
	next_token();
	continue;
      }
      else
	continue;
    }
    //
    //  State MetaTag
    //
    if ( state & xml_eState_MetaTag) {
      if ( !(state & xml_eState_TagNameFound)) {
	if ( is_space(c))
	  continue;
	else {
	  // Start of tag name
	  state |= xml_eState_TagName;
	  state |= xml_eState_TagNameFound;
	  current_tag_idx = 0;
	  current_tag[current_tag_idx++] = c;
	}
      }
      else {
	if ( state & xml_eState_TagName) {
	  if ( is_space(c)) {
	    // End of tag name
	    current_tag[current_tag_idx] = 0;	
	    state &= ~xml_eState_TagName;
	    sts = interpreter->metatag( current_tag);	
	    if ( EVEN(sts)) {
	      fp.close();
	      return sts;
	    }
	  }
	  else if ( c == '?' && c_f == '>') {
	    // End of meta tag
	    next_token();
	    current_tag[current_tag_idx] = 0;	
	    if ( state & xml_eState_AttributeName || 
		 state & xml_eState_AttributeValue)
	      error_message_line( "Syntax error");
	    state &= ~xml_eState_TagName;
	    state &= ~xml_eState_TagNameFound;
	    state &= ~xml_eState_MetaTag;
	    sts = interpreter->metatag( current_tag);	
	    if ( EVEN(sts)) {
	      fp.close();
	      return sts;
	    }
	  }
	  else {
	    // Next tag character
	    if ( current_tag_idx >= sizeof(current_tag) - 1)
	      error_message_line( "Buffer to small");
	    else
	      current_tag[current_tag_idx++] = c;
	  }
	}
	else {
	  if ( is_space(c) && !(state & xml_eState_AttributeValue))
	    continue;
	  else if ( c == '?' && c_f == '>') {
	    // End of meta tag
	    next_token();
	    if ( state & xml_eState_AttributeName || 
		 state & xml_eState_AttributeValue)
	      error_message_line( "Syntax error");
	    state &= ~xml_eState_AttributeNameFound;
	    state &= ~xml_eState_AttributeValueFound;
	    state &= ~xml_eState_TagNameFound;
	    state &= ~xml_eState_MetaTag;
	    sts = interpreter->metatag_end( current_tag);
	    if ( EVEN(sts)) {
	      fp.close();
	      return sts;
	    }
	  }
	  else {
	    if ( !(state & xml_eState_AttributeNameFound)) {
	      // Start of attribute name
	      state |= xml_eState_Attribute;
	      state |= xml_eState_AttributeName;
	      state |= xml_eState_AttributeNameFound;
	      current_attribute_name_idx = 0;
	      current_attribute_name[current_attribute_name_idx++] = c;
	    }
	    else if ( state & xml_eState_AttributeName) {
	      if ( is_space(c)) {
		// End of attribute name
		current_attribute_name[current_attribute_name_idx] = 0;
		state &= ~xml_eState_AttributeName;
	      }
	      else if ( c == '=') {
		// End of attribute name
		current_attribute_name[current_attribute_name_idx] = 0;
		state &= ~xml_eState_AttributeName;
	      }
	      else {
		// Next char of attribute name
		if ( current_attribute_name_idx >= sizeof(current_attribute_name) - 1)
		  error_message_line( "Buffer to small");
		else
		  current_attribute_name[current_attribute_name_idx++] = c;
	      }
	    }
	    else if ( state & xml_eState_AttributeNameFound) {
	      if ( !(state & xml_eState_AttributeValueFound)) {
		if ( c == '\"') {
		  // Start of attribute value
		  state |= xml_eState_AttributeValue;
		  state |= xml_eState_AttributeValueFound;
		  current_attribute_value_idx = 0;
		}
		else
		  continue;
	      }
	      else if ( state & xml_eState_AttributeValue) {
		if ( c == '\\' && c_f == '\"') {
		  // '"' sign in string
		  if ( current_attribute_value_idx >= sizeof(current_attribute_value) - 1)
		    error_message_line( "Buffer to small");
		  else
		    current_attribute_value[current_attribute_value_idx++] = c_f;		 
		  next_token();
		}
		else if ( c == '\"') {
		  // End of attribute value
		  current_attribute_value[current_attribute_value_idx] = 0;
		  state &= ~xml_eState_AttributeValue;
		  state &= ~xml_eState_Attribute;
		  state &= ~xml_eState_AttributeNameFound;
		  state &= ~xml_eState_AttributeValueFound;
		  sts = interpreter->tag_attribute( current_attribute_name, current_attribute_value);
		  if ( EVEN(sts)) {
		    fp.close();
		    return sts;
		  }
		}
		else {
		  // Next char in attribute value
		  if ( current_attribute_value_idx >= sizeof(current_attribute_value) - 1)
		    error_message_line( "Buffer to small");
		  else
		    current_attribute_value[current_attribute_value_idx++] = c;
		}
	      }
      	    }
	  }
	}
      }
    }
    //
    //  State Tag
    //
    else if ( state & xml_eState_Tag) {
      if ( !(state & xml_eState_TagNameFound)) {
	if ( is_space(c))
	  continue;
	else {
	  // Start of tag name
	  state |= xml_eState_TagName;
	  state |= xml_eState_TagNameFound;
	  current_tag_idx = 0;
	  current_tag[current_tag_idx++] = c;
	}
      }
      else {
	if ( state & xml_eState_TagName) {
	  if ( is_space(c)) {
	    // End of tag name
	    current_tag[current_tag_idx] = 0;	
	    state &= ~xml_eState_TagName;
	    sts = interpreter->tag( current_tag);
	    if ( EVEN(sts)) {
	      fp.close();
	      return sts;
	    }
	  }
	  else if ( c == '/' && c_f == '>') {
	    // End of tag
	    next_token();
	    current_tag[current_tag_idx] = 0;
	    sts = interpreter->tag( current_tag);
	    if ( EVEN(sts)) {
	      fp.close();
	      return sts;
	    }
	    sts = interpreter->tag_end( current_tag);
	    if ( EVEN(sts)) {
	      fp.close();
	      return sts;
	    }
	    if ( state & xml_eState_AttributeName || 
		 state & xml_eState_AttributeValue)
	      error_message_line( "Syntax error");
	    state &= ~xml_eState_TagName;
	    state &= ~xml_eState_TagNameFound;
	    state &= ~xml_eState_Tag;
	  }
	  else if ( c == '>') {
	    // End of tag
	    current_tag[current_tag_idx] = 0;	
	    if ( state & xml_eState_AttributeName || 
		 state & xml_eState_AttributeValue)
	      error_message_line( "Syntax error");
	    state &= ~xml_eState_TagName;
	    state &= ~xml_eState_TagNameFound;
	    state &= ~xml_eState_Tag;
	    state |= xml_eState_TagValue;
	    current_tag_value_idx = 0;
	    sts = interpreter->tag( current_tag);
	    if ( EVEN(sts)) {
	      fp.close();
	      return sts;
	    }
	  }
	  else {
	    // Next tag character
	    if ( current_tag_idx >= sizeof(current_tag) - 1)
	      error_message_line( "Buffer to small");
	    else
	      current_tag[current_tag_idx++] = c;
	  }
	}
	else {
	  if ( is_space(c) && !(state & xml_eState_AttributeValue))
	    continue;
	  else if ( c == '/' && c_f == '>') {
	    // End of tag
	    next_token();
	    if ( state & xml_eState_AttributeName)
	      error_message_line( "Syntax error");
	    else if ( state & xml_eState_AttributeValue) {
	      // Next char in attribute value
	      if ( current_attribute_value_idx >= sizeof(current_attribute_value) - 1)
		error_message_line( "Buffer to small");
	      else
		current_attribute_value[current_attribute_value_idx++] = c;
	      continue;
	    }
	    sts = interpreter->tag_end( current_tag);
	    if ( EVEN(sts)) {
	      fp.close();
	      return sts;
	    }
	    state &= ~xml_eState_AttributeNameFound;
	    state &= ~xml_eState_AttributeValueFound;
	    state &= ~xml_eState_TagNameFound;
	    state &= ~xml_eState_Tag;	    
	    suppress_msg = 0;
	  }
	  else if ( c == '>') {
	    // End of tag
	    if ( state & xml_eState_AttributeName)
	      error_message_line( "Syntax error");
	    else if ( state & xml_eState_AttributeValue) {
	      // Next char in attribute value
	      if ( current_attribute_value_idx >= sizeof(current_attribute_value) - 1) {
		if ( !suppress_msg) {
		  error_message_line( "Buffer size to small, value cut off");
		  suppress_msg = 1;
		}
	      }
	      else
		current_attribute_value[current_attribute_value_idx++] = c;
	      continue;
	    }
	    state &= ~xml_eState_AttributeNameFound;
	    state &= ~xml_eState_AttributeValueFound;
	    state &= ~xml_eState_TagNameFound;
	    state &= ~xml_eState_Tag;
	    suppress_msg = 0;
	  }
	  else {
	    if ( !(state & xml_eState_AttributeNameFound)) {
	      // Start of attribute name
	      state |= xml_eState_Attribute;
	      state |= xml_eState_AttributeName;
	      state |= xml_eState_AttributeNameFound;
	      current_attribute_name_idx = 0;
	      current_attribute_name[current_attribute_name_idx++] = c;
	    }
	    else if ( state & xml_eState_AttributeName) {
	      if ( is_space(c)) {
		// End of attribute name
		current_attribute_name[current_attribute_name_idx] = 0;
		state &= ~xml_eState_AttributeName;
	      }
	      else if ( c == '=') {
		// End of attribute name
		current_attribute_name[current_attribute_name_idx] = 0;
		state &= ~xml_eState_AttributeName;
	      }
	      else {
		// Next char of attribute name
		if ( current_attribute_name_idx >= sizeof(current_attribute_name) - 1)
		  error_message_line( "Buffer size to small");
		else
		  current_attribute_name[current_attribute_name_idx++] = c;
	      }
	    }
	    else if ( state & xml_eState_AttributeNameFound) {
	      if ( !(state & xml_eState_AttributeValueFound)) {
		if ( c == '\"') {
		  // Start of attribute value
		  state |= xml_eState_AttributeValue;
		  state |= xml_eState_AttributeValueFound;
		  current_attribute_value_idx = 0;
		}
		else
		  continue;
	      }
	      else if ( state & xml_eState_AttributeValue) {
		if ( c == '\\' && c_f == '\"') {
		  // '"' sign in string
		  if ( current_attribute_value_idx >= sizeof(current_attribute_value) - 1) {
		    if ( !suppress_msg) {
		      error_message_line( "Buffer size to small, value cut off");
		      suppress_msg = 1;
		    }
		  }
		  else
		    current_attribute_value[current_attribute_value_idx++] = c_f;		 
		  next_token();
		}
		else if ( c == '\"') {
		  // End of attribute value
		  current_attribute_value[current_attribute_value_idx] = 0;
		  state &= ~xml_eState_AttributeValue;
		  state &= ~xml_eState_Attribute;
		  state &= ~xml_eState_AttributeNameFound;
		  state &= ~xml_eState_AttributeValueFound;
		  suppress_msg = 0;
		  sts = interpreter->tag_attribute( current_attribute_name, current_attribute_value);
		  if ( EVEN(sts)) {
		    fp.close();
		    return sts;
		  }
		}
		else {
		  // Next char in attribute value
		  if ( current_attribute_value_idx >= sizeof(current_attribute_value) - 1) {
		    if ( !suppress_msg) {
		      error_message_line( "Buffer size to small, value cut off");
		      suppress_msg = 1;
		    }
		  }
		  else
		    current_attribute_value[current_attribute_value_idx++] = c;
		}
	      }
      	    }
	  }
	}
      }
    }
    //
    //  State TagValue
    //
    else if ( state & xml_eState_TagValue) {
      if ( c == '<' && c_f == '!' && c_ff == '-' && !(state & xml_eState_AttributeValue)) {
	in_comment = true;
	next_token();
	next_token();
      }
      else if ( c == '<' && c_f == '/') {
	// Start of EndTag
	next_token();
	current_tag_value[current_tag_value_idx] = 0;
	if ( state & xml_eState_TagValueFound) {
	  sts = interpreter->tag_value( current_tag_value);
	  if ( EVEN(sts)) {
	    fp.close();
	    return sts;
	  }
	  state &= ~xml_eState_TagValueFound;
	}
	state &= ~xml_eState_TagValue;
	state |= xml_eState_EndTag;
	current_tag_idx = 0;
      }
      else if ( c == '<' && !(state & xml_eState_TagValueFound)) {
	// New tag, no value found
	state &= ~xml_eState_TagValue;
	state |= xml_eState_Tag;
	current_tag_idx = 0;
      }
      else {
	if ( !is_space(c))
	  state |= xml_eState_TagValueFound;
	if ( current_tag_value_idx >= sizeof(current_tag_value) - 1)
	  error_message_line( "Buffer to small");
	else
	  current_tag_value[current_tag_value_idx++] = c;
      }
    }
    //
    //  State EndTag
    //
    else if ( state & xml_eState_EndTag) {
      if ( !(state & xml_eState_TagNameFound)) {
	if ( is_space(c))
	  continue;
	else {
	  // Start of tag name
	  state |= xml_eState_TagName;
	  state |= xml_eState_TagNameFound;
	  current_tag_idx = 0;
	  current_tag[current_tag_idx++] = c;
	}
      }
      else {
	if ( state & xml_eState_TagName) {
	  if ( is_space(c)) {
	    // End of tag name
	    current_tag[current_tag_idx] = 0;	
	    state &= ~xml_eState_TagName;
	    sts = interpreter->tag_end( current_tag);
	    if ( EVEN(sts)) {
	      fp.close();
	      return sts;
	    }
	  }
	  else if ( c == '>') {
	    // End of tag
	    current_tag[current_tag_idx] = 0;	
	    state &= ~xml_eState_TagName;
	    state &= ~xml_eState_TagNameFound;
	    state &= ~xml_eState_EndTag;
	    sts = interpreter->tag_end( current_tag);
	    if ( EVEN(sts)) {
	      fp.close();
	      return sts;
	    }
	  }
	  else {
	    // Next tag character
	    if ( current_tag_idx >= sizeof(current_tag) - 1)
	      error_message_line( "Buffer to small");
	    else
	      current_tag[current_tag_idx++] = c;
	  }
	}
	else {
	  if ( is_space(c))
	    continue;
	  else if ( c == '>') {
	    // End of tag
	    state &= ~xml_eState_TagNameFound;
	    state &= ~xml_eState_EndTag;
	  }
	}
      }
    }
    else if ( state & xml_eState_Init) {
      if ( c == '<' && c_f == '?') {
	state |= xml_eState_MetaTag;
	current_tag_idx = 0;
	next_token();
      }
      else if ( c == '<' && c_f == '!' && c_ff == '-') {
	in_comment = true;
	next_token();
	next_token();
      }
      else if ( c == '<' && c_f == '/') {
	state |= xml_eState_EndTag;
	current_tag_idx = 0;
	next_token();
      }
      else if ( c == '<') {
	state |= xml_eState_Tag;
	current_tag_idx = 0;
      }
    }
  }

  fp.close();

  return DCLI__SUCCESS;
}

//
// Convert binary data to octet string
//
int co_xml_parser::data_to_ostring( unsigned char *data, int size, char *str, int strsize)
{
  int len = 0;
  for ( int i = 0; i < size; i++) {
    if ( i == size - 1) {
      if ( len + 5 >= strsize)
	return 0;
      len += sprintf( &str[i*5], "0x%02hhx", data[i]);
    }
    else {
      if ( len + 4 >= strsize)
	return 0;
      len += sprintf( &str[i*5], "0x%02hhx,", data[i]);
    }
  }
  return 1;
}

//
// Convert octet string to binary
// 
int co_xml_parser::ostring_to_data( unsigned char **data, const char *str, int size, int *rsize)
{
  char valstr[40];
  int valcnt;
  unsigned int val;
  const char *s, *t;
  int sts;

  *data = (unsigned char *) calloc( 1, size);
  t = str;
  valcnt = 0;
  for ( s = str;; s++) {
    if ( valcnt > size) {
      printf( "** Size error");
      break;
    }
    if ( *s == ',' || *s == 0) {
      strncpy( valstr, t, s - t);
      valstr[s - t] = 0;
      dcli_remove_blank( valstr, valstr);
      if ( valstr[0] == '0' && valstr[1] == 'x')
	sts = sscanf( &valstr[2], "%x", &val);
      else
	sts = sscanf( valstr, "%d", &val);
      *(*data + valcnt++) = (unsigned char) val;
      if ( sts != 1)
	error_message( "Syntax error in octet string, %s", str);
 
      t = s+1;
    }
    if ( *s == 0)
      break;
  }
  if ( rsize)
    *rsize = valcnt;
  return 1;
}

int co_xml_interpreter::tag_stack_push( unsigned int id)
{
  if ( tag_stack_cnt >= sizeof(tag_stack)/sizeof(tag_stack[0]) - 1) {
    parser->error_message_line( "Tag stack overflow");
    return 0;
  }
  tag_stack[tag_stack_cnt++] = id;
  return 1;
}

int co_xml_interpreter::tag_stack_pull( unsigned int id)
{
  if ( tag_stack_cnt < 1 || tag_stack[tag_stack_cnt - 1] != id) {
    parser->error_message_line( "Tag stack missmatch");
    return 0;
  }
  tag_stack_cnt--;
  return 1;
}

int co_xml_interpreter::object_stack_push( void *o, unsigned int id)
{
  if ( object_stack_cnt >= sizeof(object_stack)/sizeof(object_stack[0]) - 1) {
    parser->error_message_line( "Object stack overflow");
    return 0;
  }
  object_stack_id[object_stack_cnt] = id;
  object_stack[object_stack_cnt++] = o;
  return 1;
}

int co_xml_interpreter::object_stack_pull( unsigned int id)
{
  if ( id != object_stack_id[object_stack_cnt-1] ||
       object_stack_cnt == 0) {
    parser->error_message_line( "Object stack pull mismatch");
    return 0;
  }

  object_stack_cnt--;
  return 1;
}

unsigned int co_xml_interpreter::get_tag_stack() 
{ 
  if ( tag_stack_cnt) 
    return tag_stack[tag_stack_cnt-1];
  else 
    return 0;
}

unsigned int co_xml_interpreter::get_tag_stack( int p) 
{ 
  if ( p <= (int)tag_stack_cnt - 1) 
    return tag_stack[tag_stack_cnt- 1 - p];
  else 
    return 0;
}

void *co_xml_interpreter::get_object_stack( unsigned int id) 
{ 
  if ( object_stack_cnt) {
    if ( object_stack_id[object_stack_cnt-1] == id)
      return object_stack[object_stack_cnt-1];
  }
  return 0;
}

void *co_xml_interpreter::get_object_stack( int p, unsigned int id) 
{ 
  if ( p <= (int)object_stack_cnt - 1) {
    if ( object_stack_id[object_stack_cnt-1-p] == id)
      return object_stack[object_stack_cnt- 1 - p];
  }
  return 0;
}

