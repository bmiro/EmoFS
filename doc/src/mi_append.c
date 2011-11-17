#include <string.h>

#include "common.h"
#include "dir.h"

/** Escriu al final del fitxer especificat al primer paramentre
 * la cadena del segon paramentre. 
 * mi_apend <path> <string> */
int main(int argc, char **argv) {
	char path[MAX_PATH_LEN];
	emofs_inode_stat info;
	
	if (argc != 3) {
		puts("Nombre de parametres incorrecte.\n");
		puts("Us: mi_append <fitxer> <cadena>\n");
		return -1;
	}
	bmount();

	strcpy(path, argv[1]);
	if (!emofs_file_exists(path)) {
		emofs_create(path);
	}

	if (emofs_is_file(path) == 0) {
		puts("append: no es poden fer appends a directoris");
		bumount();
		return -2;
	}
	emofs_stat(path, &info);
	emofs_write(path, argv[2], info.size, strlen(argv[2]));
	bumount();
	return 0;
}
