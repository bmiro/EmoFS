#include <stdlib.h>
#include <string.h>

#include "common.h"

/*
 * Fes un enllac simbolic d'un fitxer.
 * mi_ln <origen> <desti>
 * @return: 0 exit, -1 nombre de parametres incorrecte
 */
int main(int argc, char **argv) {
	char from[MAX_PATH_LEN];
	char to[MAX_PATH_LEN];
	int error = 0;

	if (argc != 3) {
		puts("Nombre de parametres incorrecte.");
		puts("Us: mi_ln <origen> <desti>");
		return -1;
	}

	strcpy(from, argv[1]);
	strcpy(to, argv[2]);
	bmount();
	if (!emofs_file_exists(from)) {
		puts("El fitxer origen no existeix");
		error = -2;
	} else if (emofs_file_exists(to)) {
		puts("El fitxer desti existeix");
		error = -3;
	} else {	
		emofs_link(from, to);
	}
	bumount();
	return error;
}
