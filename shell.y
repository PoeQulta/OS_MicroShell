
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */
%define parse.error verbose
%token	<string_val> WORD

%token 	NOTOKEN GREAT APPEND NEWLINE LESS PIPE AMP EXIT

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
        ;

simple_command:	
	EXIT NEWLINE{
		printf("\n\t Good bye!!\n");
		exit(0);
	}
	| command_and_args iomodifier_opt_list NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| command_and_args iomodifier_opt_list AMP NEWLINE {
		Command::_currentCommand._background = 1;
		printf("   Yacc: Execute command in Background\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;


command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	| command_and_args PIPE command_word arg_list
	{
			Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;
iomodifier_opt_list:
	iomodifier_opt_list iomodifier_opt
	|
	;
iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	| APPEND WORD {
		printf("   Yacc: insert Appending output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._append = 1;
	}
	| LESS WORD{
		printf("   Yacc: insert Input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	;
	| LESS WORD GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $4);
		Command::_currentCommand._outFile = $4;
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
