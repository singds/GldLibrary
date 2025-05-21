#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#define IMAGE_FILENAME                  "../Image/GuiImg"
#define TEXT_FILENAME                   "../Text/GuiTxt"

FILE *read_file;
FILE *write_file;

char name[50];

void menuImages( void )
{
	/* Apertura file dat in scrittura */
	write_file = fopen( IMAGE_FILENAME".h", "w" );
	
	int ck=0;
	int index = 0;
	for( ; ; )
	{
		char c = fgetc( read_file );
		if( c == EOF )
			break;
		
		if( c == '"' ){
			if( ck ){
				name[ index - 4 ] = '\0'; 
				fprintf( write_file, "extern const char %s[];\n", name );
			}
			ck = !ck;
			index = 0;
			continue;
		}
		
		if( ck )
		{
			if (c == '/' || c == '\\')
				index = 0; // ripristino l'identificazione del nome file
			else
				name[ index++ ]=c;
		}
	}
	
	
	
	fclose( write_file );
}

void menuText( void )
{
	/* Apertura file dat in scrittura */
	write_file = fopen( TEXT_FILENAME".h", "w" );
	
	int ck=0;
	int index = 0;
	for( ; ; )
	{
		char c = fgetc( read_file );
		if( c == EOF )
			break;
		
		if( c == '*' ){
			ck = 1;
			continue;
		}
		
		if( c == '[' ){
			name[ index ] = '\0'; 
			fprintf( write_file, "extern const char *%s[];\n", name );
			ck = 0;
			index = 0;
			continue;
		}
		
		if( ck )
			name[ index++ ]=c;
	}
	
	fclose( write_file );
}

int main( int argc, char* argv[] ){
	
	read_file = fopen( IMAGE_FILENAME".c", "rb" );
	if( read_file == NULL ){
		printf( "errore: file <"IMAGE_FILENAME".c> non trovato\n");
	}
	else {
		menuImages( );
		printf( "fine scrittura <"IMAGE_FILENAME".h>\n");
		fclose( read_file );
	}
	
	read_file = fopen( TEXT_FILENAME".c", "rb" );
	if( read_file == NULL ){
		printf( "errore: file <"TEXT_FILENAME".c> non trovato\n");
	}
	else {
		menuText( );
		printf( "fine scrittura <"TEXT_FILENAME".h>\n");
		fclose( read_file );
	}
	
	getchar( );
}
