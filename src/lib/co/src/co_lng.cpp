
#include <string.h>
#include <stdio.h>
 
extern "C" {
#include "co_dcli.h"
}
#include "co_lng.h"

lng_eLanguage Lng::lang = lng_eLanguage_en_us;
tree_sTable *Lng::tree = 0;

char *Lng::get_language_str()
{
  return lang_to_str( lang);
}

lng_eLanguage Lng::str_to_lang( char *str)
{
  if ( strcmp( str, "en_us") == 0)
    return lng_eLanguage_en_us;
  if ( strcmp( str, "sv_se") == 0)
    return lng_eLanguage_sv_se;
  if ( strcmp( str, "ru_ru") == 0)
    return lng_eLanguage_ru_ru;

  return lng_eLanguage_;
}

char *Lng::lang_to_str( lng_eLanguage language)
{
  static char result[20];

  switch( language) {
    case lng_eLanguage_sv_se:
      strcpy( result, "sv_se");
      break;
    case lng_eLanguage_ru_ru:
      strcpy( result, "ru_ru");
      break;
    default:
      strcpy( result, "en_us");
  }
  return result;
}

static int compKey( tree_sTable *tp, tree_sNode *x, tree_sNode *y)
{
    lang_sKey *xKey = (lang_sKey *) (tp->keyOffset + (char *) x);
    lang_sKey *yKey = (lang_sKey *) (tp->keyOffset + (char *) y);

    char type = (xKey->type == 0) ? yKey->type : xKey->type;
    switch ( type) {
    case 'E':
      return strcmp(xKey->text, yKey->text);
    case 'B': {
      char *s = strrchr( xKey->text, ',');
      if ( s)
	return strncmp(xKey->text, yKey->text, 
		       (unsigned int)s - (unsigned int)xKey->text);
      return strcmp(xKey->text, yKey->text);
    }
    }
    return 0;
}

char *Lng::translate( char *text) 
{
  static char result[200];
  lang_sKey key;
  lang_sRecord *record;
  int sts;
  char *in_p;

  if ( lang == lng_eLanguage_en_us || tree == 0) {
    // No translation is needed
    strncpy( result, text, sizeof(result));
    result[sizeof(result)-1] = 0;
    return result;
  }

  // Remove space
  for ( in_p = text; *in_p == ' ' && *in_p; in_p++)
    ;

  strcpy( key.text, text);
  key.type = 0;
  record = (lang_sRecord *) tree_Find( &sts, tree, &key);
  if ( ODD(sts)) {
    switch ( record->key.type) {
    case 'B': {
      char *s = strrchr( record->transl, ',');
      if ( s) {
	char *t = strrchr( text, ',');
	if ( t) {
	  strncpy( result, record->transl, 
		   (unsigned int)s - (unsigned int)record->transl + 1);
	  strcat( result, t+1);
	}
	else
	  strcpy( result, record->transl);
      }
      else
	strcpy( result, record->transl);
      break;
    }
    default:
      strcpy( result, record->transl);
    }
  }
  else {
    strncpy( result, text, sizeof(result));
    result[sizeof(result)-1] = 0;
  }
  return result;
}

int Lng::translate( char *text, char *out) 
{
  char result[200];
  lang_sKey key;
  lang_sRecord *record;
  int sts;
  char *in_p;

  if ( lang == lng_eLanguage_en_us || tree == 0) {
    // No translation is needed
    return 0;
  }

  // Remove space
  for ( in_p = text; *in_p == ' ' && *in_p; in_p++)
    ;

  strcpy( key.text, text);
  key.type = 0;
  record = (lang_sRecord *) tree_Find( &sts, tree, &key);
  if ( ODD(sts)) {
    switch ( record->key.type) {
    case 'B': {
      char *s = strrchr( record->transl, ',');
      if ( s) {
	char *t = strrchr( text, ',');
	if ( t) {
	  strncpy( result, record->transl, 
		   (unsigned int)s - (unsigned int)record->transl + 1);
	  strcat( result, t+1);
	}
	else
	  strcpy( result, record->transl);
      }
      else
	strcpy( result, record->transl);
      break;
    }
    default:
      strcpy( result, record->transl);
    }
    strcpy( out, result);
    return 1;
  }
  // Not found
  return 0;
}

void Lng::unload()
{
  int sts;

  tree_DeleteTable( &sts, tree);
  tree = 0;
}

bool Lng::read()
{
  char fname1[120] = "$pwr_exe/en_us/xtt_lng.dat";
  char fname2[120] = "$pwr_exe/%s/xtt_lng.dat";
  pwr_tFileName filename1, filename2;
  int sts;

  sprintf( filename2, fname2, get_language_str());
  dcli_translate_filename( filename1, fname1);
  dcli_translate_filename( filename2, filename2);

  ifstream fp1( filename1);
  if ( !fp1)
    return false;

  ifstream fp2( filename2);
  if ( !fp2)
    return false;

  if ( tree)
    tree_DeleteTable( &sts, tree);

  tree = tree_CreateTable( &sts, sizeof(lang_sKey), offsetof(lang_sRecord, key),
			   sizeof(lang_sRecord), 100, compKey);

  Row r1( fp1, fname1);
  Row r2( fp2, fname2);
  
  bool hit = true;
  for (;;) {
    if ( hit) {
      if ( !read_line( r1))
	break;

      if ( !read_line( r2))
	break;
    }
    else if ( r1.lt( r2)) {
      if ( !read_line( r1))
	break;
    }
    else {
      if ( !read_line( r2))
	break;
    }

    hit = false;
    if ( r1.eq( r2))
      hit = true;
    if ( hit) {
      lang_sKey key;
      lang_sRecord *record;

      strcpy( key.text, r1.text);
      key.type = r1.type;
      record = (lang_sRecord *) tree_Insert( &sts, tree, &key);
      strcpy( record->transl, r2.text);
      // printf ( "%c %d.%d.%d '%s' '%s'\n", r1.type, r1.n1, r1.n2, r1.n3, r1.text,r2.text);
    }
  }    
  return true;
}

bool Lng::read_line( Row& r)
{
  int nr;
  char line[200];
  char *s, *t;

  for(;;) {
    if ( !r.fp.getline( line, sizeof( line)))
      return false;

    r.row++;
    if ( line[0] == '#' || line[0] == 0)
      continue;

      
    nr = sscanf( line, "%c%d.%d.%d", &r.type, &r.n1, &r.n2, &r.n3);
    if ( nr != 4) {
      printf( "Error in line %d, file %s\n", r.row, r.fname);
      continue;
    }

    bool in_text = false;
    for ( s = line, t = r.text; *s; s++) {
      if ( in_text) {
	if ( *s == '\"') {
	  if ( *(s-1) == '\\')
	    *(t-1) = '\"';
	  else
	    break;
	}
	else
	  *t++ = *s;;	
      }
      if ( !in_text && *s == '\"')
	in_text = 1;
    }      
    *t = 0;
    break;
  }
  return true;
}


void Lng::get_uid( char *in, char *out)
{
  char result[200];

#if defined OS_VMS
  {
    char dev[80], dir[80], file[80], type[80];
    int version;
    char c;

    dcli_parse_filename( "pwr_exe:", dev, dir, file, type, &version);
    sprintf( result, "%s%s", dev, dir);
    c = result[strlen(result)-1];
    sprintf( &result[strlen(result)-1], ".%s%c%s%", get_language_str(), 
	     c, in);
  }
#else
  sprintf( result, "$pwr_exe/%s/%s", get_language_str(), in);
  dcli_translate_filename( result, result);
#endif
  strcpy( out, result);
}

void Lng::set( char *language) 
{ 
  lng_eLanguage l = str_to_lang( language);
  if ( l != lng_eLanguage_)
    set( l);
}

void Lng::set( lng_eLanguage language) 
{ 
  if ( lang == language)
    return;

  lang = language;
  if ( lang == lng_eLanguage_en_us)
    unload();
  else
    read();
}











