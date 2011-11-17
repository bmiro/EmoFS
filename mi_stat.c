#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common.h"
#include "dir.h"

/*
 * Mostra informació d'un fitxer
 * mi_cat <nom> 
 * @return: 0 èxit, -1 nombre de paràmetres incorrecte
 */
int main(int argc, char **argv) {
	char path[MAX_PATH_LEN];
	char msg[7*80]; /* 7 linies de 80 caràcters */
	emofs_inode_stat info;

	if (argc != 2) {
		puts("Nombre de paràmetres incorrecte.");
		puts("Ús: mi_cat <nom>");
		return -1;
	}

	strcpy(path, argv[1]);
	bmount();

	if (!emofs_file_exists(path)) {
		puts("El fitxer o directori no existeix");
		return -1;
	}

	emofs_stat(path, &info);
	if (info.type == DIRECTORY_INODE) {
		sprintf(msg, "Tipus: directori\n");
	} else {
		sprintf(msg, "Tipus: fitxer\n");
	}
	sprintf(msg, "%sTamany en bytes: %d\n", msg, info.size);
	sprintf(msg, "%sData de modificació: %d\n", msg, info.mtime);
	sprintf(msg, "%sBlocs físics asignats: %d\n", msg, info.block_count);
	sprintf(msg, "%sNombre d'enllaços: %d\n", msg, info.link_count);
	fwrite(msg, strlen(msg), 1, stdout);
	
	bumount();

	return 0;

}
