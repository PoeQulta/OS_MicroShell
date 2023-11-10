
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "command.h"
#include <ctime>
#include <time.h>
#include <iostream>
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time
/*	Constants	*/
char LOG_FILE_NAME[] = "/child-log.txt";
int next_dir = 0;
long size;
char *path_to_current_dir;
char *ptr_to_current_dir;

int changeCurrentDirectory(const char* dir) {
	int result = chdir(dir);
	free(path_to_current_dir);
	if ((path_to_current_dir = (char *)malloc((size_t)size)) != NULL)
    	ptr_to_current_dir = getcwd(path_to_current_dir, (size_t)size);
	return result;
}


SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("Num of Cmds: %d",_numberOfSimpleCommands);
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
		printf("\n");
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background        Append\n" );
	printf( "  ------------ ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO",_append?"YES":"NO");
	printf( "\n\n" );
	
}
static void logChild(int sig)
{
	int fd = open("Log", O_RDWR  | O_APPEND | O_CREAT,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << std::endl;
	write(fd,ss.str().c_str(),strlen(ss.str().c_str()));
	close(fd);
}
void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}

	// Print contents of Command data structure
	print();

	// Add execution here
	int defaultin = dup( 0 );
	int defaultout = dup( 1 );
	int defaulterr = dup( 2 );
	int pid;

	//Create Pipes
	int ** pipes = (int **) malloc( (_numberOfSimpleCommands+1) * sizeof( int * ));

	for(int i=0;i <= _numberOfSimpleCommands;i++)
		pipes[i] = (int *) malloc(2*sizeof(int));

	for(int i=1;i<_numberOfSimpleCommands;i++)
	{
		if ( pipe(pipes[i]) == -1) {
			perror( "pipe");
			exit( 2 );
		}
	}
	//Setup input and output files
	if(_inputFile)
	{
		pipes[0][0] = open(_inputFile, O_RDONLY);
	}
	else
	{
		pipes[0][0] = defaultin;
	}
	if(_outFile)
	{
		if(_append)
			pipes[_numberOfSimpleCommands][1] = open(_outFile, O_RDWR  | O_APPEND | O_CREAT,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		else
			pipes[_numberOfSimpleCommands][1] = open(_outFile, O_RDWR  | O_CREAT | O_TRUNC,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	}
	else
	{
		pipes[_numberOfSimpleCommands][1] = defaultout;
	}
	//Forking & executing
	for(int i=1;i<=_numberOfSimpleCommands;i++)
	{
		printf("Exec %s\n",_simpleCommands[i-1]->_arguments[0]);
		// Redirect input 
		dup2( pipes[i-1][0], 0 );
		// Redirect output to pipe
		dup2( pipes[i][1], 1 );
		// Redirect err (use stderr)
		dup2( defaulterr, 2 );
		//fork
		pid = fork();
		if (pid == -1 ) {
			perror( "fork");
			exit( 2 );
		}
		
		if (pid == 0) {
			//Child

			// close file descriptors that are not needed
			close(pipes[i-1][0]);
			execvp(_simpleCommands[i-1]->_arguments[0],_simpleCommands[i-1]->_arguments);

			// exec() is not suppose to return, something went wrong
			perror( "cat_grep: exec grep");
			exit( 2 );

		}
		else
		{
		// Close file descriptors that are not needed
		close(pipes[i][1]);
		}


	}
	if(!_background)
		waitpid( pid, 0, 0 );
	// Clear to prepare for next command
	for ( int i = 0; i <= _numberOfSimpleCommands; i++ )
			free ( pipes[i]);
	free (pipes);
	clear();
	dup2( defaultin, 0 );
	dup2( defaultout, 1 );
	dup2( defaulterr, 2 );
	// Print new prompt
	prompt();
}

// Shell implementation

void
Command::prompt()
{
	
	printf("PoSh> %s $ ",ptr_to_current_dir);
	fflush(stdout);
}
// ignore ctrl-c handler
void catchSIGINT(int sig_num)
{
	Command::_currentCommand.clear();
	printf("\r\033[0J"); // Erase ctrl-C
	Command::_currentCommand.prompt();
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()
{
	size = pathconf(".", _PC_PATH_MAX);
	changeCurrentDirectory("");
	signal(SIGCHLD,logChild);
	signal(SIGINT, catchSIGINT);
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}

