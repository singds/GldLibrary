#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#define L_IMAGE_DIRECTORY ".././Image/Bmp/"
#define L_FILENAME_PREFIX "guiimg_"
#define L_VARIABLE_PREFIC "guiimg_"

void convert_bmp( char* name ){
	FILE *read_file;
	FILE *write_file;
	char filename[50];
	
	/* Apertura file bmp in lettura */
	snprintf (filename, sizeof (filename), "%s%s", L_IMAGE_DIRECTORY, name);
	read_file = fopen( filename, "rb" );

	strcpy( filename, name );
	filename[ strlen( filename )-4 ] = '\0';
	
	char tmp_str[50];
	snprintf (tmp_str, sizeof (tmp_str), L_IMAGE_DIRECTORY L_FILENAME_PREFIX "%s.dat", filename);
	/* Apertura file dat in scrittura */
	write_file = fopen( tmp_str, "w" );
	
	/* Scrittura dichiarazione immagina */
	snprintf (tmp_str, sizeof (tmp_str), "const char " L_VARIABLE_PREFIC "%s[]={\n", filename);
	fwrite( tmp_str, 1, strlen( tmp_str ), write_file );
	
	#define MAX_LINE_CNT 12
	
	unsigned char buffer[MAX_LINE_CNT];
	int size;
	int k;
	do{
		size = fread( buffer, 1, MAX_LINE_CNT, read_file );
		for( k=0; k< size; k++ ){
			fputc( ' ', write_file );
			fprintf( write_file, "0x%02X", buffer[k] );
			fputc( ',', write_file );
		}
		fputc( '\n', write_file );
		
	}while( size != 0 );
	
	fprintf( write_file, "};\n" );
	
	fclose( write_file );
	fclose( read_file );
}

int main( int argc, char* argv[] ){
	
	DIR           *d;
	struct dirent *dir;
	d = opendir(L_IMAGE_DIRECTORY);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if( strstr( dir->d_name, ".bmp" ) != NULL ){
				convert_bmp( dir->d_name );
				printf("Convertito file %s%s\n", L_IMAGE_DIRECTORY, dir->d_name);
			}
		}
		closedir(d);
	}
	printf( "Scansione directory completata" );
	
	getchar( );
}
