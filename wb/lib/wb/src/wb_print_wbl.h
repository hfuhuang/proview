/**
 * @file wb_print_wbl.h
 *
 * Prints a volume on wbl format
 *
 */

#ifndef wb_print_wbl_h
#define wb_print_wbl_h

#include <iostream.h>

#include "pwr_class.h"

class wb_adef;
class wb_attribute;
class wb_cdef;
class wb_object;
class wb_volume;


class wb_print_wbl 
{
protected:
  int  m_errCnt;
  bool m_idxFlag;
  int  m_level;
  int  m_levelInd;
  char m_indBuf[256];
  ostream& m_os;
    

  ostream& indent(int levelIncr = 0);    

  void printAttribute(wb_volume& v, 
                      wb_attribute& attr, 
                      wb_attribute& tattr, ///< template attribute
                      wb_adef& adef);

  void printBody(wb_volume& v, 
                 wb_object& o,
                 wb_object& templ,
                 wb_cdef& cdef, 
                 pwr_eBix bix);

  void printClass(wb_volume& v,
                  wb_attribute& attr,
                  wb_attribute& tattr, ///< template attribute
                  wb_adef& adef);
    
  void printParameter(wb_volume& v, 
                      wb_attribute& attr, 
                      wb_attribute& tattr, ///< template attribute
                      wb_adef& adef);

  void printText(wb_volume& v, 
                 wb_adef& adef,
                 const char* text,
                 int varSize);

  bool printValue(wb_volume& v, 
                  wb_adef& adef,
                  void *val,
                  int varSize,
                  char **svalp);
    

public:
  wb_print_wbl(ostream& os, int levelIndentation = 2);
  ~wb_print_wbl();

  int getErrCnt() const { return m_errCnt;}
  void resetErrCnt() {m_errCnt = 0; }

  void printHierarchy(wb_volume& v, wb_object& o); //< Prints a hierarchy
  void printObject(wb_volume& v, wb_object& o, bool recursive = true); //< Prints an object
  void printVolume(wb_volume& v, bool recursive = true); //< Prints the volume

};

#endif
