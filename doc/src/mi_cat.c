#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "dir.h"

#define C_BUF 2048

/*
 * Mostra el contingut d'un fitxer. Aquest cat es especial i permet mostrar el
 * contingut d'un directori.
 * mi_cat <nom> 
 * @return: 0 exit, -1 nombre de parametres incorrecte
 */
int main(int argc, char **argv) {
	char path[MAX_PATH_LEN];
	int n_entries;
	int error = 0;
	emofs_inode_stat info;
	char *buffer;
	int offset = 0;
	int bytes_left = 0;
	int to_read = 0; /* bytes llegits */
	int read_bytes = 0;

	if (argc != 2) {
		puts("Nombre de parametres incorrecte.");
		puts("Us: mi_cat <nom>");
		return -1;
	}

	strcpy(path, argv[1]);
	bmount();

	if(!emofs_file_exists(path)) {
		bumount();
		return -2;
	}

	emofs_stat(path, &info);
	for (bytes_left = info.size; bytes_left > 0; bytes_left -= C_BUF) {
		to_read = (bytes_left < C_BUF) ? (bytes_left % C_BUF) : C_BUF;
		read_bytes = emofs_read(path, (void *) &buffer, offset, to_read);
		fwrite(buffer, read_bytes, 1, stdout);
		/* A la ultima iteracio no es cert pero ens estalvam un acces
		 * a memoria a cada iteracio. */
		offset += C_BUF;
	}
	puts("");
	bumount();
	return error;
}
