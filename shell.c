/** @file shell.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "log.h"

log_t Log;

int main()
{
	log_t l;
	log_init( &l );

	while(1)
	{
		size_t length = 1;
		char * buff = calloc( length, sizeof( char ) );
		char * cwd = getcwd( buff, length );

		while( cwd == NULL )
		{
			length *= 2;
			buff = realloc( buff, length * sizeof(char) );
			cwd = getcwd( buff, length );
		}

		pid_t pid = getpid();

		printf("(pid=%d)%s$ ", pid, cwd);
		
		free( buff );

		char * input = NULL;
		size_t leng = 0;
		size_t read = 0;

		read = getline( &input, &leng, stdin);

		if( strcmp( input, "\n" ) == 0)
		{
			free( input );
			continue;
		}

		if( *input == '!' )
		{
			printf("Command executed by pid=%d\n", pid);

			if( strcmp( input, "!#\n" ) == 0 ) 
			{ 
				int i;
				for( i = 0; i < l.num; i++ )
				{
					printf("%s", l.histo[i] );
				}

				free( input );
				continue;
			}
			else
			{
				char * result = log_search( &l, input +1 );

				if( result == NULL )
				{
					puts("No Match");
					free( input );
					continue;
				}

				char * temp = malloc( strlen( input + 1) );

				int k;
				for(k = 0; k < strlen( input + 1 ) - 1; k ++)
				{
					temp[k] = input[k+1];
				}
				temp[k] = '\0';

				printf("%s matches %s", temp, result);
				free(temp);

				log_push( &l, result );
				free( input );
				input = calloc( strlen( result ) + 1, sizeof( char ) );
				strcpy( input, result );
			}
		}
		else
		{
			log_push( &l, input );
		}

		/*if( feof( stdin ) )
		{
			log_destroy( &l );
			exit( 0 );
		}*/

		if( strcmp( input, "exit\n" ) == 0 ) 
		{ 
			printf("Command executed by pid=%d\n", pid);
			free( input );

			log_destroy( &l );
			exit( 0 );
		}

		char * token = strtok( input, " \n" );
		char * first = token;
		char ** follower = calloc( 100, sizeof( char * ) );
		int i = 1;

		while( token )
		{
			token = strtok( NULL, " \n" );
			follower[i] = token;
			i++;
		}
		follower[0] = first;
		follower[i] = NULL;

		if( strcmp( first, "cd") == 0 )
		{
			printf("Command executed by pid=%d\n", pid);
			
			if( chdir( follower[1] ) )
				printf("%s: No such file or directory\n", follower[1] );
			free( follower );
			free( input );
			continue;
		}

		pid_t child;
		int status;

		if ( ( child = fork() ) < 0 )
		{
			perror("Filed to fork");
			exit(1);
		}
		
		else if ( child == 0 )
		{
			pid_t pid = getpid();
			printf("Command executed by pid=%d\n", pid);

			execvp( first, follower );

      		printf("%s: not found\n", first);

			free(follower);
			free( input );
			log_destroy( &l );

      		exit(1);
		}
		else
		{
			child = wait( &status );
		}

		free( follower );
		free( input );
	}
	
	log_destroy( &l );
    return 0;
}