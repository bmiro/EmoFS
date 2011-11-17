#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "dir.h"

/** Escriu adins un fitxer.
 * mi_write fitxer buffer <offset = 0>
 * @fitxer: el fitxer on escriure
 * @buffer: la cadena a escriure
 * @offset: el desplacament. Per defecte 0.
 * @return: 0 si exit
 */
int main(int argc, char **argv) {
	int offset = 0;
	int error = 0;
	if ((argc == 1) || (argc > 4)) {
		puts("Nombre de parametres incorrecte.");
		puts("Us: mi_write fitxer buffer <offset=0>");
		return -1;
	}
	if (argc == 4) {
		offset = atoi(argv[3]);
	}
	bmount();
	/* argv[1] -> path */
	if (!emofs_file_exists(argv[1])) {
		error = emofs_create(argv[1]);
	}
	error = emofs_write(argv[1], argv[2], offset, strlen(argv[2]));
	bumount();
	return error > 0;
}
