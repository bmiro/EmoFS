#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "dir.h"

/*
 * Crea un diretori.
 * mi_mkdir <cami>
 * @return: 0 exit, -1 nombre de parametres incorrecte
 */
int main(int argc, char **argv) {
	char path[MAX_PATH_LEN];
	int error = 0;
	int len;

	if (argc != 2) {
		puts("Nombre de parametres incorrecte.");
		puts("Us: mi_mkdir <cami>");
		return -1;
	}
	bmount();
	strcpy(path, argv[1]);
	len = strlen(path);
	if (path[len-1] != '/') {
		strcat(path, "/");
	}
	error =	emofs_create(path);
	if(error < 0) {
		puts("mi_mkdir: el cami ja existeix");
	}
	bumount();
	return error;

}
