/*
 * Make sure to run antlr.Tool on the lexer.g file first!
 */
header {
#include "wb_wblnode.h"
}

header "post_include_hpp" {
extern int wblparser_error_cnt;

# This declarations should be moved inside parser class in wb_wblparser.hpp !!
void reportError(const RecognitionException& ex);
}
header "post_include_cpp" {
#include "co_msgwindow.h"
int wblparser_error_cnt = 0;

void wb_wblparser::reportError(const RecognitionException& ex)
{
	MsgWindow::message( 'E', ex.toString().c_str());
	wblparser_error_cnt++;
	ANTLR_USE_NAMESPACE(std)cerr << ex.toString().c_str() << ANTLR_USE_NAMESPACE(std)endl;
}
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
	: VOLUME^ cid OID (body)* ((DOCBLOCK)? object)* ENDVOLUME
	;

sobject
	: SOBJECT^ ((DOCBLOCK)? object)* ENDSOBJECT
	;

object
	: OBJECT^ cid (oix)? (body)* ((DOCBLOCK)? object)* ENDOBJECT
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











