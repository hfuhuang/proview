/** 
 * Proview   $Id: co_lng.h,v 1.9 2008-10-31 12:51:30 claes Exp $
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
 **/

#ifndef co_lng_h
#define co_lng_h

/* co_lng.h -- Language */

#include <iostream>
#include <fstream>
#include <string.h>

using namespace std;

#if defined __cplusplus
extern "C" {
#endif

#ifndef pwr_h
# include "pwr.h"
#endif

#ifndef co_tree_h
# include "co_tree.h"
#endif

typedef enum {
  lng_eLanguage_,
  lng_eLanguage_af_ZA,
  lng_eLanguage_ar_AE,
  lng_eLanguage_ar_BH,
  lng_eLanguage_ar_DZ,
  lng_eLanguage_ar_EG,
  lng_eLanguage_ar_IN,
  lng_eLanguage_ar_IQ,
  lng_eLanguage_ar_JO,
  lng_eLanguage_ar_KW,
  lng_eLanguage_ar_LB,
  lng_eLanguage_ar_LY,
  lng_eLanguage_ar_MA,
  lng_eLanguage_ar_OM,
  lng_eLanguage_ar_QA,
  lng_eLanguage_ar_SA,
  lng_eLanguage_ar_SD,
  lng_eLanguage_ar_SY,
  lng_eLanguage_ar_TN,
  lng_eLanguage_ar_YE,
  lng_eLanguage_be_BY,
  lng_eLanguage_bg_BG,
  lng_eLanguage_br_FR,
  lng_eLanguage_bs_BA,
  lng_eLanguage_ca_ES,
  lng_eLanguage_cs_CZ,
  lng_eLanguage_cy_GB,
  lng_eLanguage_da_DK,
  lng_eLanguage_de_AT,
  lng_eLanguage_de_BE,
  lng_eLanguage_de_CH,
  lng_eLanguage_de_DE,
  lng_eLanguage_de_LU,
  lng_eLanguage_el_GR,
  lng_eLanguage_en_AU,
  lng_eLanguage_en_BW,
  lng_eLanguage_en_CA,
  lng_eLanguage_en_DK,
  lng_eLanguage_en_GB,
  lng_eLanguage_en_HK,
  lng_eLanguage_en_IE,
  lng_eLanguage_en_IN,
  lng_eLanguage_en_NZ,
  lng_eLanguage_en_PH,
  lng_eLanguage_en_SG,
  lng_eLanguage_en_US,
  lng_eLanguage_en_ZA,
  lng_eLanguage_en_ZW,
  lng_eLanguage_es_AR,
  lng_eLanguage_es_BO,
  lng_eLanguage_es_CL,
  lng_eLanguage_es_CO,
  lng_eLanguage_es_CR,
  lng_eLanguage_es_DO,
  lng_eLanguage_es_EC,
  lng_eLanguage_es_ES,
  lng_eLanguage_es_GT,
  lng_eLanguage_es_HN,
  lng_eLanguage_es_MX,
  lng_eLanguage_es_NI,
  lng_eLanguage_es_PA,
  lng_eLanguage_es_PE,
  lng_eLanguage_es_PR,
  lng_eLanguage_es_PY,
  lng_eLanguage_es_SV,
  lng_eLanguage_es_US,
  lng_eLanguage_es_UY,
  lng_eLanguage_es_VE,
  lng_eLanguage_et_EE,
  lng_eLanguage_eu_ES,
  lng_eLanguage_fa_IR,
  lng_eLanguage_fi_FI,
  lng_eLanguage_fo_FO,
  lng_eLanguage_fr_BE,
  lng_eLanguage_fr_CA,
  lng_eLanguage_fr_CH,
  lng_eLanguage_fr_FR,
  lng_eLanguage_fr_LU,
  lng_eLanguage_ga_IE,
  lng_eLanguage_gl_ES,
  lng_eLanguage_gv_GB,
  lng_eLanguage_he_IL,
  lng_eLanguage_hi_IN,
  lng_eLanguage_hr_HR,
  lng_eLanguage_hu_HU,
  lng_eLanguage_id_ID,
  lng_eLanguage_is_IS,
  lng_eLanguage_it_CH,
  lng_eLanguage_it_IT,
  lng_eLanguage_iw_IL,
  lng_eLanguage_ja_JP,
  lng_eLanguage_ka_GE,
  lng_eLanguage_kl_GL,
  lng_eLanguage_ko_KR,
  lng_eLanguage_kw_GB,
  lng_eLanguage_lt_LT,
  lng_eLanguage_lv_LV,
  lng_eLanguage_mi_NZ,
  lng_eLanguage_mk_MK,
  lng_eLanguage_mr_IN,
  lng_eLanguage_ms_MY,
  lng_eLanguage_mt_MT,
  lng_eLanguage_nl_BE,
  lng_eLanguage_nl_NL,
  lng_eLanguage_nn_NO,
  lng_eLanguage_no_NO,
  lng_eLanguage_oc_FR,
  lng_eLanguage_pl_PL,
  lng_eLanguage_pt_BR,
  lng_eLanguage_pt_PT,
  lng_eLanguage_ro_RO,
  lng_eLanguage_ru_RU,
  lng_eLanguage_ru_UA,
  lng_eLanguage_se_NO,
  lng_eLanguage_sk_SK,
  lng_eLanguage_sl_SI,
  lng_eLanguage_sq_AL,
  lng_eLanguage_sr_YU,
  lng_eLanguage_sv_FI,
  lng_eLanguage_sv_SE,
  lng_eLanguage_ta_IN,
  lng_eLanguage_te_IN,
  lng_eLanguage_tg_TJ,
  lng_eLanguage_th_TH,
  lng_eLanguage_tl_PH,
  lng_eLanguage_tr_TR,
  lng_eLanguage_uk_UA,
  lng_eLanguage_ur_PK,
  lng_eLanguage_uz_UZ,
  lng_eLanguage_vi_VN,
  lng_eLanguage_wa_BE,
  lng_eLanguage_yi_US,
  lng_eLanguage_zh_CN,
  lng_eLanguage_zh_HK,
  lng_eLanguage_zh_TW,
  lng_eLanguage__
} lng_eLanguage;

typedef struct {
  char		text[80];
  char		type;
} lang_sKey;

typedef struct {
  tree_sNode 	n;
  lang_sKey	key;
  char		transl[80];
} lang_sRecord;

class Row {
 public:
  char type;
  int n1;
  int n2;
  int n3;
  char text[200];
  int row;
  ifstream& fp;
  char fname[200];

  Row( ifstream& f, char *filename) : row(0), fp(f) { 
    strcpy( text, ""); strcpy( fname, filename);}
  bool eq( Row& r) { return n1 == r.n1 && n2 == r.n2 && n3 == r.n3;}
  bool lt( Row& r) {
    if ( n1 < r.n1)
      return true;
    else if ( n1 > r.n1)
      return false;
    if ( n2 < r.n2)
      return true;
    else if ( n2 > r.n2)
      return false;
    if ( n3 < r.n3)
      return true;
    return false;
  }
  bool gt( Row& r) {
    if ( n1 > r.n1)
      return true;
    else if ( n1 < r.n1)
      return false;
    if ( n2 > r.n2)
      return true;
    else if ( n2 < r.n2)
      return false;
    if ( n3 > r.n3)
      return true;
    return false;
  }
};

class Lng {
  public:
    Lng() {};
    static lng_eLanguage lang;
    static const int Help        = 0;
    static const int Help_Class  = 1;
    static const int Graph       = 2;
    static const int Trend       = 3;
    static const int Open_Object = 4;
    static const int Open_Object___ = 5;
    static const int RtNavigator = 6;
    static const int Class_Graph = 7;
    static const int Crossreferences = 8;
    static const int DataSheet   = 9;
    static const int Collect     = 10;
    static const int Type_Graph___ = 11;
    static const int Open_URL___ = 12;
    static const int Open_Plc___ = 13;
    static char items[][2][40];

    static tree_sTable *tree;

    static bool read();
    static bool read_line( Row& r);
    static char *translate( const char *text);
    static char *get_language_str();
    static int translate( char *in, char *out);
    static void unload();
    static pwr_tStatus set( lng_eLanguage language);
    static lng_eLanguage current() { return lang;}
    static void set( char *language);
    static char *get( int item)
      { return items[lang][item];}
    static char *get( lng_eLanguage language, int item)
      { return items[language][item];}
    static void get_uid( char *in, char *out);
    static lng_eLanguage str_to_lang( char *str);
    static char *lang_to_str( lng_eLanguage language);
    static char *lang_to_locale( lng_eLanguage language);
    static bool is_installed( lng_eLanguage language);
};

#if defined __cplusplus
}
#endif
#endif


