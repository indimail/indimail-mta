%{

#include <string.h>
#include "parser.h"

unsigned int in_block = 0;
extern unsigned int lineno;

%}

%s INBLOCK
%s INVALUE

%%

<INVALUE>\s*\n		{ lineno++;BEGIN(INBLOCK); }
<INVALUE>[^\n]+	{ printf("INMANDA FIELD\n"); yylval.string = (char *)strdup(yytext); return STRING; }

<INBLOCK>[\s\t]+	{ ; }
<INBLOCK>\n		{ lineno++; }
<INBLOCK>"#".*\n	{ lineno++; }
<INBLOCK>"}"   { 
	if(in_block) {
		BEGIN 0; 
		in_block = 0;
		printf("finished block\n"); 
        } else {
		yyerror("ERRO: NOT in block"); 
	}
}
<INBLOCK>match|regex { printf("OPA MATCH %d\n",in_block); return MATCH; }
<INBLOCK>command|exec[ute]{0,3} { printf("OPA EXEC %d\n",in_block); return EXECUTE; }
<INBLOCK>[match_]{0,6}sleep { printf("OPA SLEEP %d\n",in_block); return SLEEP; }
<INBLOCK>retry { printf("OPA RETRY %d\n",in_block); return RETRY; }
<INBLOCK>user { printf("OPA USER %d\n",in_block); return USER; }
<INBLOCK>readall { printf("OPA READALL %d\n",in_block); return READALL; }
<INBLOCK>verbose { printf("OPA VEROBSE %d\n",in_block); return VERBOSE; }
<INBLOCK>"=" { printf("achei igual\n");BEGIN(INVALUE);return EQUAL; }


[ \t]+		{ ; }
\n		{ lineno++; }
"#".*\n		{ lineno++; }
filename { return FILENAME; }

"{"   { 
	if(in_block) {
		yyerror("already insde block"); 
        } else {
		BEGIN(INBLOCK); 
		in_block = 1;
		printf("\nentered in block\n"); 
	}
}
"}" {
	 yyerror("not inside a block");
}
[^ \t\n=]+	{ yylval.string = (char *)strdup(yytext); return STRING; }

%%

