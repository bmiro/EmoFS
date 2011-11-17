#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "common.h"
#include "dir.h"

/** Crea un fitxer buit
 * mi_touch <data> <nom> 
 * Si s'indica la data s'ha de fer donat el format seguit de: yyyyMMddhhmmss
 * @return: 0 exit, -1 nombre de parametres incorrecte
 * @En un futur programar cas amb els parametres de segons
 */
int main(int argc, char **argv) {
	char path[MAX_PATH_LEN];
	char data[15]; /* yyyymmddhhmmss */
	int error = 0;
	time_t new_time;

	if ((argc == 1) || (argc >= 3)) {
		puts("Nombre de parametres incorrecte.");
		puts("Us: mi_touch <nom> <data>");
		return -1;
	}


	strcpy(path, argv[1]);

	bmount();

	if (argc == 2) {
		if (!emofs_file_exists(path)) {
			error = emofs_create(path);
		}
		error = emofs_update_time(path, time(NULL));
	} else {
		if (!emofs_file_exists(path)) {
			error = emofs_create(path);
		}
		strcpy(data, argv[2]); /* En un futur processar data */
		error = emofs_update_time(path, new_time);
	}
	bumount();
	return error;
}
