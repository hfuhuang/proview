#ifndef wb_wblnode_h
#define wb_wblnode_h

#include <iostream.h>
#include "antlr/CommonAST.hpp"
#include "pwr.h"
#include "wb_name.h"
#include "wb_wblfile.h"


ANTLR_USING_NAMESPACE(std)
ANTLR_USING_NAMESPACE(antlr)

class wb_wblnode;
class wb_vrepwbl;
class wb_vrep;
class wb_dbs;
class wb_import;
class wb_treeimport;

typedef enum {
  wbl_eNodeType_No,
  wbl_eNodeType_Type,
  wbl_eNodeType_TypeDef,
  wbl_eNodeType_ClassDef,
  wbl_eNodeType_ObjBodyDef,
  wbl_eNodeType_Attribute,
  wbl_eNodeType_BodyNode,
  wbl_eNodeType_AttrNode,
  wbl_eNodeType_Template,
  wbl_eNodeType_Volume,
  wbl_eNodeType_Buffer
} wbl_eNodeType;

typedef antlr::ASTRefCount<wb_wblnode> ref_wblnode;

typedef struct {
  char	*sym;
  int	value;
} wbl_sSym;

class wbl_class {
 public:
  wbl_class() : cid(0), cix(0), templ(0) {};
    pwr_tCid cid;
    int cix;
    ref_wblnode templ;
};

class wbl_type {
 public:
  wbl_type() : tid(0), type(pwr_eType_), elements(0), size(0) {};
    pwr_tTid tid;
    pwr_eType type;
    int elements;
    size_t size;
};

class wbl_body {
 public:
  wbl_body() : bix(pwr_eBix__), size(0) {};
    pwr_eBix bix;
    size_t size;
};

class wbl_attribute {
 public:
  wbl_attribute() : tid(0), size(0), offset(0), elements(0), flags(0),
    index(0) {};
    pwr_tTid tid;
    size_t size;
    size_t offset;
    int elements;
    int flags;
    int index;
    pwr_eType type;
};

class wbl_object {
 public:
  wbl_object() : rbody_size(0), dbody_size(0), rbody(0), dbody(0),
    m_cid(0), m_tid(0), fth(0), bws(0), fws(0), fch(0),
    is_built(0) 
    { 
      strcpy( cname, ""); 
      m_flags.m = 0;
      m_oid.oix = 0;
      m_oid.vid = 0;
    };
  ~wbl_object() {
      if ( rbody_size) free( rbody);
      if ( dbody_size) free( dbody);
  }    
  size_t rbody_size;
  size_t dbody_size;
  void *rbody;
  void *dbody;
  pwr_tCid m_cid;
  pwr_tTid m_tid;
  pwr_tOid m_oid;
  char cname[32];
  pwr_mClassDef m_flags;
  wb_wblnode *fth;
  wb_wblnode *bws;
  wb_wblnode *fws;
  wb_wblnode *fch;
  int is_built;

  wbl_class c;
  wbl_type ty;
  wbl_body b;
  wbl_attribute a;
};


class wb_wblnode : public CommonAST {

  friend class wb_vrepwbl;

public:
    wb_wblnode() : 
      node_type(wbl_eNodeType_No), 
      line_number(0), file(0), o(0)
    {
    }
    wb_wblnode(antlr::RefToken t) : 
      node_type(wbl_eNodeType_No), 
      line_number(0), file(0), o(0)
    {
      CommonAST::setType(t->getType());
      CommonAST::setText(t->getText());
    }
    ~wb_wblnode() 
    {
      if ( o)
	delete o;
    }

    void initialize(int t, const std::string& txt)
    {
      CommonAST::setType(t);
      CommonAST::setText(txt);
    }

    void initialize( ref_wblnode t)
    {
      CommonAST::setType(t->getType());
      CommonAST::setText(t->getText());
    }
    void initialize( RefAST t )
   {
      CommonAST::initialize(t);
    }
    void initialize( antlr::RefToken t);

    void setText(const std::string& txt)
    {
      CommonAST::setText(txt);
    }

    void setType(int type)
    {
      CommonAST::setType(type);
    }

    void addChild( ref_wblnode c )
    {
      BaseAST::addChild( RefAST(c));
    }

    static antlr::RefAST factory( void )
    {
      antlr::RefAST ret = RefAST(new wb_wblnode);
      return ret;
    }

    ref_wblnode wb_wblnode::getFirstChild()
    {
      return ref_wblnode(BaseAST::getFirstChild());
    }

    ref_wblnode wb_wblnode::getNextSibling()
    {
       return ref_wblnode(BaseAST::getNextSibling());
    }


    // Heterogeneous part
    void info( int level) {
      for ( int i = 0; i < level; i++)
        cout << "  ";
      cout << getType() << " " << getText() << endl;
      ref_wblnode t = getFirstChild();
      if ( t)
        t->info( level + 1);

      t = getNextSibling();
      if ( t)
        t->info( level);
    }

    bool isType() { return (node_type == wbl_eNodeType_Type);}
    bool isTypeDef() { return (node_type == wbl_eNodeType_TypeDef);}
    bool isClassDef() { return (node_type == wbl_eNodeType_ClassDef);}
    bool isObjBodyDef() { return (node_type == wbl_eNodeType_ObjBodyDef);}
    bool isAttribute() { return (node_type == wbl_eNodeType_Attribute);}
    bool isBodyNode() { return (node_type == wbl_eNodeType_BodyNode);}
    bool isAttrNode() { return (node_type == wbl_eNodeType_AttrNode);}
    bool isTemplate() { return (node_type == wbl_eNodeType_Template);}
    bool isVolume() { return (node_type == wbl_eNodeType_Volume);}
    bool isBuffer() { return (node_type == wbl_eNodeType_Buffer);}

    void setFile( wb_wblfile *f);
    char *getFileName() { if ( file) return file->file_name; else return "";}
    pwr_tTime getFileTime() 
    {
      if ( file) return file->time; 
      else { pwr_tTime t = {0,0}; return t;} 
    }
    void registerNode( wb_vrepwbl *vol);
    void build( bool recursive);
    void postBuild();
    void buildObjBodyDef( ref_wblnode classdef);
    void buildAttribute( ref_wblnode classdef, ref_wblnode objbodydef, 
			 int *bindex, size_t *boffset);
    void buildBuffer( ref_wblnode classdef, ref_wblnode objbodydef, 
			 int *bindex, size_t *boffset);
    void buildTemplate( ref_wblnode classdef);
    void buildBody( ref_wblnode object);
    void buildAttr( ref_wblnode object, pwr_eBix bix);
    void buildBuff( ref_wblnode object, pwr_eBix bix, pwr_tCid buffer_cid,
		    int buffer_offset, int buffer_size);
    void buildBuffAttr( ref_wblnode object, pwr_eBix bix, pwr_tCid buffer_cid,
			size_t buffer_offset, size_t buffer_size);
    void link( wb_vrepwbl *vol, ref_wblnode father_node);
    void info_link( int level);
    ref_wblnode find( wb_name *oname, int level);
    ref_wblnode get_o_lch();
    int classNameToCid( char *cname, pwr_tCid *cid);
    int stringToOix( const char *buf, pwr_tOix *oix) const;
    pwr_tCid Cid() { return o->c.cid;}
    int attrStringToValue( int type_id, char *value_str, 
			   void *buffer_ptr, size_t buff_size, size_t attr_size);

    bool exportHead(wb_import &i);
    bool exportDbody(wb_import &i);
    bool exportRbody(wb_import &i);
    bool exportTree(wb_treeimport &i, bool isRoot);

#if 0
    void iterObject(wb_dbs *);
    void iterRbody(wb_dbs *);
    void iterDbody(wb_dbs *);

    void iterObject( void *udata, 
		     pwr_tStatus (*bc)(void *, pwr_tOid, pwr_tCid, pwr_tOid, pwr_tOid,
				       pwr_tOid, pwr_tOid, pwr_tOid, char *,
				       pwr_tTime, int, int));
    void iterBody( void *udata, 
		   pwr_tStatus (*bc)(void *, pwr_tOid, void *, void *));
#endif

    static int lookup( int *type, const char *keyword, wbl_sSym *table);
    static int convconst( int *val, char *str);
    const char *name() { return getText().c_str();}


    wbl_eNodeType node_type;
    wb_vrepwbl *m_vrep;
    int line_number;
    wb_wblfile *file;

    wbl_object *o;
};


#endif








