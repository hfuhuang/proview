#ifndef wb_pkg_h
#define wb_pkg_h

#include <iostream.h>
#include <vector.h>
#include <string>
#include "pwr.h"
#include "pwr_class.h"
#include "wb_nav_macros.h"
#include "wb_lfu.h"

class pkg_node;

class pkg_file {
  friend class pkg_node;

 private:
  char m_source[200];
  char m_target[200];
  char m_arname[80];
  pwr_tTime m_date;

 public:
  pkg_file( char *source, char *target);
  char *source() { return m_source;}
  char *target() { return m_target;}
  pwr_tTime date() { return m_date;}

};

class pkg_pattern {
  friend class pkg_node;

 private:
  char m_source[200];
  char m_target[200];  
  vector<pkg_file> m_filelist;

 public:
  pkg_pattern( char *source, char *target) {
    strcpy( m_source, source);
    strcpy( m_target, target);
  }
  pkg_pattern( char *source) {
    strcpy( m_source, source);
    strcpy( m_target, "");
  }
  pkg_pattern( const pkg_pattern& x) : m_filelist(x.m_filelist) {
    strcpy( m_source, x.m_source);
    strcpy( m_target, x.m_target);
  }
  char *source() { return m_source;}
  char *target() { return m_target;}
  bool hasTarget() { return m_target[0] != 0;}
  void fetchFiles();
};

class pkg_node {
 private:
  vector<pkg_pattern> m_pattern;
  vector<pkg_file> m_filelist;
  char m_name[80];
  pwr_mOpSys m_opsys;
  int m_bus;
  lfu_eDistrSts m_dstatus;
  bool m_valid;

 public:
  pkg_node( char *name): m_opsys(pwr_mOpSys__), m_bus(0), 
    m_dstatus(lfu_eDistrSts_Normal), m_valid(false) { strcpy( m_name, name);}
  pkg_node( char *name, pwr_mOpSys opsys, int bus, lfu_eDistrSts dstatus) :
    m_opsys(opsys), m_bus(bus), m_dstatus(dstatus),
    m_valid(true) { strcpy( m_name, name); }
  char *name() { return m_name;}
  pwr_mOpSys opsys() { return m_opsys;}
  int bus() { return m_bus;}
  lfu_eDistrSts dstatus() { return m_dstatus;}
  bool valid() { return m_valid;}
  void setOpsys( pwr_mOpSys opsys) { m_opsys = opsys;}
  void setBus( int bus) { m_bus = bus;}
  void setDStatus( lfu_eDistrSts dstatus) { m_dstatus = dstatus;} 
  void setValid() { m_valid = true;}
  void push_back( pkg_pattern& pattern) { 
    m_pattern.push_back( pattern);
  }
  void fetchFiles( bool distribute);
};

class wb_pkg {
 private:
  vector<pkg_node> m_nodelist;
  bool m_allnodes;

  void readConfig();

 public:
  wb_pkg( char *nodelist, bool distribute = true);
  pkg_node& getNode( char *name);
  void fetchFiles( bool distribute) {
    for ( int i = 0; i < (int)m_nodelist.size(); i++) m_nodelist[i].fetchFiles( distribute);
  }

  static void copyPackage( char *pkg_name);
};

#endif




