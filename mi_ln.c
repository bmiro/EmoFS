#include <stdlib.h>
#include <string.h>

#include "common.h"

/*
 * Fes un enllaç simbòlic d'un fitxer.
 * mi_ln <origen> <destí>
 * @return: 0 èxit, -1 nombre de paràmetres incorrecte
 */
int main(int argc, char **argv) {
	char from[MAX_PATH_LEN];
	char to[MAX_PATH_LEN];
	int error = 0;

	if (argc != 3) {
		puts("Nombre de paràmetres incorrecte.");
		puts("Ús: mi_ln <origen> <destí>");
		return -1;
	}

	strcpy(from, argv[1]);
	strcpy(to, argv[2]);
	bmount();
	if (!emofs_file_exists(from)) {
		puts("El fitxer origen no existeix");
		error = -2;
	} else if (emofs_file_exists(to)) {
		puts("El fitxer destí existeix");
		error = -3;
	} else {	
		emofs_link(from, to);
	}
	bumount();
	return error;
}
