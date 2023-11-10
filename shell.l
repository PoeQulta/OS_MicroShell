
/*
 *
 * CS-413 Spring 98
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include "y.tab.h"

%}

%%

\n 	{
		return NEWLINE;
	}

[ \t] 	{
		/* Discard spaces and tabs */
     	}	

">" 	{
		return GREAT;
	}
	
">>" 	{
		return APPEND;
	}
"<" 	{
		return LESS;
	}
"|" 	{
		return PIPE;
	}
"&" 	{
		return AMP;
	}
	
"exit" {
	return EXIT;
}

"cd" {
	return CD;
}	

[^ (\t\n|\||\<|\>|\&)][^ (\t\n|\||\<|\>|\&)]*  {
		/* Assume that file names have only alpha chars */
		yylval.string_val = strdup(yytext);
		return WORD;
	}

.	{
		/* Invalid character in input */
		return NOTOKEN;
	}

%%
	
