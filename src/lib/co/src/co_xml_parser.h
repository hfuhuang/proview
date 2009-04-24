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

#ifndef co_xml_parser_h
#define co_xml_parser_h

#include <iostream>
#include <fstream>

using namespace std;

class co_xml_parser;

class co_xml_interpreter {
 public:
  co_xml_interpreter() : tag_stack_cnt(0), object_stack_cnt(0) {}

  virtual int tag( const char *name) { return 0;}
  virtual int metatag( const char *name) { return 0;}
  virtual int tag_end( const char *name) { return 0;}
  virtual int metatag_end( const char *name) { return 0;}
  virtual int tag_value( const char *value) { return 0;}
  virtual int tag_attribute( const char *name, const char *value) { return 0;}

  int tag_stack_push( unsigned int id);
  int tag_stack_pull( unsigned int id);
  int object_stack_push( void *o, unsigned int id);
  int object_stack_pull( unsigned int id);
  unsigned int get_tag_stack();
  unsigned int  get_tag_stack( int p);
  void *get_object_stack( unsigned int id);
  void *get_object_stack( int p, unsigned int id);

  co_xml_parser *parser;
  unsigned int tag_stack[100];
  unsigned int tag_stack_cnt;
  void *object_stack[100];
  unsigned int object_stack_id[100];
  unsigned int object_stack_cnt;
};

class co_xml_parser {
 public:
  co_xml_parser( co_xml_interpreter *i);
  ~co_xml_parser() {}
  
  co_xml_interpreter *interpreter;
  ifstream fp;
  int logglevel;
  bool first_token;
  unsigned int state;
  int line_cnt;
  int char_cnt;
  bool in_comment;
  char c;
  char c_f;
  char c_ff;
  void *c_sts;
  void *c_f_sts;
  void *c_ff_sts;
  char current_tag[256];
  char current_attribute_name[256];
  char current_attribute_value[4096];
  char current_tag_value[256];
  unsigned int current_tag_idx;
  unsigned int current_attribute_name_idx;
  unsigned int current_attribute_value_idx;
  unsigned int current_tag_value_idx;
  int suppress_msg;

  int read( const char *filename);
  static int ostring_to_data( unsigned char **data, const char *str, int size, int *rsize);
  static int data_to_ostring( unsigned char *data, int size, char *str, int strsize);

  void *next_token();
  bool is_space( const char c);
  void error_message_line( const char *msg);
  static void error_message( const char *format, const char *value);
};

#endif
