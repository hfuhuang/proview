#ifndef wb_orepmem_h
#define wb_orepmem_h

#include "wb_orep.h"
#include "wb_vrepmem.h"
#include "pwr.h"

class wb_orepmem : public wb_orep
{
  wb_vrepmem *m_vrep;
  unsigned int m_refCount;
  mem_object *m_memobject;
    
public:


  wb_orepmem(int oix);
  wb_orepmem( wb_vrepmem *v, mem_object *n);
  ~wb_orepmem();

  // void* operator new(size_t size, wb_vrepmem *v);
  // void operator delete(void *p, size_t size, wb_vrepmem *v);
    
  virtual pwr_tOid oid() const;
  virtual pwr_tVid vid() const;
  virtual pwr_tOix oix() const;

  virtual pwr_tCid cid() const;
  virtual pwr_tOid poid() const;
  virtual pwr_tOid foid() const;
  virtual pwr_tOid loid() const;
  virtual pwr_tOid boid() const;
  virtual pwr_tOid aoid() const;
    
  virtual const char * name() const;
  virtual wb_name longName();
  virtual pwr_tTime ohTime() const;
    
  virtual bool isOffspringOf(const wb_orep *o) const;
    
  // Navigational operations

  wb_orep *ancestor(pwr_tStatus *sts);  //< get object at top of hierarchy
  wb_orep *parent(pwr_tStatus *sts);
  wb_orep *after(pwr_tStatus *sts);     //< get next sibling
  wb_orep *before(pwr_tStatus *sts);    //< get previous sibling
  wb_orep *first(pwr_tStatus *sts);     //< get first child
  wb_orep *child(pwr_tStatus *sts, wb_name &name);    //< get named child
  wb_orep *last(pwr_tStatus *sts);      //< get last child
  wb_orep *next(pwr_tStatus *sts);      //< get next in list of objects of same class in one volume
  wb_orep  *previous(pwr_tStatus *sts);  //< get previous in list of objects of same class in one volume

  virtual wb_adrep *attribute(pwr_tStatus*, const char *name);

  virtual wb_adrep *attribute(pwr_tStatus*);

  mem_object *memobject() { return m_memobject;}

  wb_erep *erep() const { return m_vrep->erep();}
  wb_vrep *vrep() const { return m_vrep;}
};

#endif

