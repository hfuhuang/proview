#ifndef wb_erep_h
#define wb_erep_h

#include <map>
#include <string>
#include <vector>
#include "wb_pwrs.h"

using namespace std;

class wb_merep;
class wb_vrep;
class wb_cdrep;
class wb_orep;
class wb_tdrep;
class wb_adrep;
class wb_name;

class wb_erep
{
  typedef map<pwr_tVid, wb_vrep*>::iterator vrep_iterator;
  typedef map< string, wb_tMethod>::iterator methods_iterator;
  typedef vector<wb_vrep*>::iterator buffer_iterator;

  unsigned int m_nRef;
  wb_merep *m_merep;
  map<pwr_tVid, wb_vrep*> m_vrepdb;
  map<pwr_tVid, wb_vrep*> m_vrepdbs;
  map<pwr_tVid, wb_vrep*> m_vrepextern;
  vector<wb_vrep*> m_vrepbuffer;
  map< string, wb_tMethod> m_methods;

  char m_dir_list[10][200];
  int m_dir_cnt;
  int m_volatile_idx;
  int m_buffer_max;
  bool m_ref_merep_occupied;

public:
  wb_erep();
  ~wb_erep();
  void unref();
  wb_erep *ref();

  //map<string
  wb_merep *merep() { return m_merep;}
  wb_vrep *volume(pwr_tStatus *sts);
  wb_vrep *volume(pwr_tStatus *sts, pwr_tVid vid);
  wb_vrep *volume(pwr_tStatus *sts, const char *name);
  wb_vrep *createVolume(pwr_tStatus *sts, pwr_tVid vid, pwr_tCid cid, const char *name);
  wb_vrep *nextVolume(pwr_tStatus *sts, pwr_tVid vid);
  wb_vrep *externVolume(pwr_tStatus *sts, pwr_tVid vid);
  wb_vrep *bufferVolume(pwr_tStatus *sts);
  wb_vrep *bufferVolume(pwr_tStatus *sts, char *name);
  wb_vrep *findBuffer( pwr_tVid vid);
  void addDb( pwr_tStatus *sts, wb_vrep *vrep);
  void addDbs( pwr_tStatus *sts, wb_vrep *vrep);
  void addExtern( pwr_tStatus *sts, wb_vrep *vrep);
  void addBuffer( pwr_tStatus *sts, wb_vrep *vrep);
  void removeDb( pwr_tStatus *sts, wb_vrep *vrep);
  void removeDbs( pwr_tStatus *sts, wb_vrep *vrep);
  void removeExtern( pwr_tStatus *sts, wb_vrep *vrep);
  void removeBuffer( pwr_tStatus *sts, wb_vrep *vrep);
  void load( pwr_tStatus *sts, char *db);

  wb_orep *object( pwr_tStatus *sts, pwr_tOid oid);
  wb_orep *object( pwr_tStatus *sts, wb_name &name);
  wb_orep *object( pwr_tStatus *sts, const char *name);

  wb_cdrep *cdrep( pwr_tStatus *sts, const wb_orep& o);
  wb_tdrep *tdrep( pwr_tStatus *sts, const wb_adrep& a);
  void method( pwr_tStatus *sts, char *methodName, wb_tMethod *method);
  int nextVolatileVid( pwr_tStatus *sts, char *name);
  void setRefMerep( wb_merep *merep);
  void resetRefMerep();
  bool refMerepOccupied() { return m_ref_merep_occupied;}

  static void volumeNameToFilename( pwr_tStatus *sts, char *name, char *filename);

private:
  void loadDirList( pwr_tStatus *status);
  void loadCommonMeta( pwr_tStatus *status);
  void loadMeta( pwr_tStatus *status, char *db);
  void loadLocalWb( pwr_tStatus *sts);
  void bindMethods();

  static void at_exit();
};

#endif







