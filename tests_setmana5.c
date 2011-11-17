#include <stdio.h>
#include "block.h"

#define MAX_BLOCS_ALEATORIS 10

int main(int argc, char **argv) {
	int ni;
	int i, j;
	int *inodes;
	int *blocs_aleatoris[MAX_BLOCS_ALEATORIS];

	if (argc != 2) {
        puts("Nombre de paràmetres incorrecte.");
        puts("Ús: tests_setmana5 <n inodes a reservar>");
		return -1;
	}

	ni = atoi(argv[1]);
	inodes = malloc(sizeof(int)*ni);
	blocs_aleatoris = malloc(sizeof(int)*MAX_BLOCS_ALEATORIS*ni);

	/* Reservar un parell de inodes */
	for (i = 0; i < ni; i++) {
		inodes[i] = alloc_inode(FILE_INODE);
		printf("Reservat inode %d, es el %d\n", i, inodes[i]);
	}
	
	/* assignar blocs de dades de manera aleatoria a punters
	 * de segon i tercer nivell (fitxer ralo) */
	/* Alerta mirar que hi hagi suficients blocs al sistema de fitxers */
	for (i = 0; i < ni; i++) {
		for (j = 0, j < MAX_BLOCS_ALEATORIS; j++) {

		}
	}

	/* Comprovar els ocupats mirant el mapa de bits */

	/* Alliberar inodes */

	/* Comprovar mapa de bits (ha de estar buit) */

	/* Tornam a reservar exactament el mateix */
	for (i = 0; i < ni; i++) {
		inodes[i] = alloc_inode(FILE_INODE);
		printf("Reservat inode %d, es el %d\n", i, inodes[i]);
	}
	for (i = 0; i < ni; i++) {
		for (j = 0, j < MAX_BLOCS_ALEATORIS; j++) {

		}
	}

	/* Truncam els fitxers per la meitat */

	/* Comprovam que s'hi alliberat el que toca al mapa de bits 
	 * i als inodes */

	/* Truncam a 0 (borram) */

	/* Tornam a Comprovar */


}


