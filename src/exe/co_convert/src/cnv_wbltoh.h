/* 
 * Proview   $Id: cnv_wbltoh.h,v 1.2 2005-09-01 14:57:47 claes Exp $
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

#ifndef cnv_wbltoh_h
#define cnv_wbltoh_h

#if defined __cplusplus
extern "C" {
#endif

#include "pwr.h"
#include "cnv_readwbl.h"
#include "cnv_wblto.h"

using namespace std;

class CnvReadWbl;
  
class CnvWblToH : public CnvWblTo {
 public:
  CnvWblToH( CnvCtx *cnv_ctx) : ctx(cnv_ctx), struct_class_open(0),
    struct_filler_cnt(0), attr_count(0), attr_next_alignlw(0) {}
  virtual ~CnvWblToH() {}

  CnvCtx      *ctx;
  int         struct_class_open;
  ofstream 	fp_struct;
  CnvFile     *cstruc;
  int		struct_cclass_written;
  int		struct_cclass_endwritten;
  int		struct_filler_cnt;
  char	struct_volid[80];
  unsigned int	struct_vid_0;
  unsigned int	struct_vid_1;
  int		attr_count;
  int		attr_next_alignlw;
  
  int init( char *first);
  int close();
  int class_exec();
  int class_close();
  int body_exec();
  int body_close();
  int attribute_exec();
  int typedef_exec();
  int typedef_close();
  int bit_exec();
  int graphplcnode() { return 1;}
  int graphplccon() { return 1;}
  int template_exec() { return 1;}
  Cnv_eWblToType type() { return Cnv_eWblToType_H;}
  int class_open() { return struct_class_open;}
  int index_open() { return 0;}
 
  int volname_to_id();
  void cix_to_classid( unsigned int cix, pwr_tClassId *cid);
  int cixstr_to_classid( char *cix_str, pwr_tClassId *cid);
  static void get_filename( CnvReadWbl *rw, char *struct_file, int hpp);
  int check_typename( char *type_volume, char *type_name);
  
};

#if defined __cplusplus
}
#endif
#endif

