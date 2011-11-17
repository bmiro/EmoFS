#include <stdio.h>
#include <string.h>
#include "dir.h"

int main(int argc, char **argv) {

	char path [255];
	char cap[255];
	char cua[255];

	strcpy(path, argv[1]);

	printf("El path es %s \n", path);

	if ( extract_path(path, cap, cua) ) {
		puts("Es un directori");
	} else {
		puts("Es un fitxer");
	}
	

	printf("El cap es: %s i la cua %s \n", cap, cua);


}

