#ifndef wb_cdef_h
#define wb_cdef_h

#include "pwr.h"
#include "pwr_class.h"
#include "wb_bdef.h"
#include "wb_orep.h"
#include "wb_cdrep.h"
#include "wb_name.h"


class wb_bdef;
class wb_adef;
class wb_cdrep;
class wb_mvrep;

class wb_cdef : public wb_status
{


  public:
    wb_cdrep *m_cdrep;

    wb_cdef();
    wb_cdef( wb_cdrep *cdrep);
    wb_cdef( wb_adef&);  // x = other_object
    wb_cdef(const wb_orep&);  // x = other orep
    wb_cdef(wb_mvrep *, pwr_tCid);
    wb_cdef(const wb_cdef&);
    ~wb_cdef();

    wb_cdef& operator=(const wb_cdef&);
    operator bool() const { return (m_cdrep != 0);}
    operator wb_cdrep*() const;
    bool operator==(wb_cdef&);
    
    size_t size();   // get objects runtime body size
    pwr_tCid cid();
    pwr_tOid oid() { pwr_tOid oid = pwr_cNOid; return oid;} // Fix

    wb_name name(); // get class name
    wb_name name(ldh_eName type);

    wb_bdef bdef(pwr_tOix bix);
    wb_bdef bdef(char *bname);
    wb_bdef bdef(wb_name bname);

  private:
    void check();
    
};

#endif


