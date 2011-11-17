#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common.h"
#include "dir.h"

/* Esborra un directori i els seus subdirectoris i fitxers
 * @dir_path: cami del directori a borrar
 * @return 0 en cas d'exit.
 *	   -1 en cas d'error.
 */
int recursive_rm(char *path);

/*
 * Esborra un fitxer. Si es un enllac simplement lleva
 * l'enllac simbolic.
 * mi_rm <cami> 
 * @return: 0 exit, -1 nombre de parametres incorrecte
 */

int main(int argc, char **argv) {
	char path[MAX_PATH_LEN];
	int error = 0;

	if (argc != 2) {
		puts("Nombre de parametres incorrecte.");
		puts("Us: mi_rm <cami>");
		return -1;
	}

	bmount();
	strcpy(path, argv[1]);
	if (!emofs_file_exists(path)) {
		printf("El fitxer/directori %s no existeix\n", path);
		return -2;
	}
		
	recursive_rm(argv[1]);
	bumount();
	return error;
}

int recursive_rm(char *path) {
	int i;
	int entries_count;
	char *dir_content; /* La funcio que l'omple ja reserva la memoria */
	char filename[MAX_FILENAME_LEN];
	char full_path[MAX_PATH_LEN];
	
	if (emofs_is_file(path)) {
		return emofs_unlink(path);
	}
	
	/* Borrarem subdirectoris */
	entries_count = emofs_dir(path, &dir_content);
	for (i=0; i < entries_count; i++) {
		emofs_extract_dir_entry(dir_content, filename);
		/* Afegim path a l'element per poder fer el
		 * emofs_stat */
		sprintf(full_path, "%s/%s", path, filename);
		recursive_rm(full_path);
	}
	free(dir_content);
	/* Borram el propi directori */
	return emofs_unlink(path);
	
}

