/*
 * Make sure to run antlr.Tool on the lexer.g file first!
 */
header {
#include "wb_wblnode.h"
}

options {
	mangleLiteralPrefix = "t_";
	language="Cpp";
}

class wb_wblparser extends Parser;
options {
	importVocab=wb_wblvocab;	// use vocab generated by lexer
        buildAST=true;
	ASTLabelType="ref_wblnode";
}


unit
	: (volume | sobject | object)+
	;

volume
	: VOLUME^ cid OID (body)* (object)* ENDVOLUME
	;

sobject
	: SOBJECT^ (object)* ENDSOBJECT
	;

object
	: OBJECT^ cid (oix)? (body)* (object)* ENDOBJECT
	;

body
	: BODY^ (attribute)* ENDBODY
	;

attribute
	: ATTRIBUTE^ (EQ | OREQ) value
	| BUFFER^ (INDEX)? (attribute)* ENDBUFFER
	;

cid
	: VALUE
	| STRING_LITERAL
	;

oix
	: VALUE
	| INT
	;

value
	: VALUE
	| STRING_LITERAL
	| NUM_FLOAT
	| INT
	;

oname
	: VALUE
	;







