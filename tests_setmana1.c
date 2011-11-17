# include <stdio.h>
# include <sys/time.h>
# include "block.h"

int main() {
	int num_blocs = 100;
	int i, j, bloc_aleatori;
	emofs_block blck, blck_zero, blck_one, blck_0xee;
	int f_fs;
	int correcte = 1;
	char nom_fs[80];
	char buf;
        struct timeval tv;
	sprintf(nom_fs, EMOFS_IMAGE_FILE);

	/* Test 0: Muntar i omplir de zeros */
	printf("Tests de la setmana 1:\n\n");
	printf("\t - Cream un sistema de fitxers dins el fitxer %s \
de %d blocs \n\t de %d bytes cada un.\n", nom_fs, num_blocs, BLOCK_SIZE);
	f_fs = bmount();
	printf("\t - Sistema muntat, ara **l'omplirem de 0s**.\n");
	blck_zero = block_of_zero();
	for (i = 0; i < num_blocs; i++) {
		bwrite(i, &blck_zero);
	}
	for (i = 0; i < num_blocs; i++) {
		bread(i, &blck);
		for (j = 0; j < BLOCK_SIZE; j++){
			if (blck.valor[j] != 0x00) {
				printf("\t -t La posicio %d del bloc %d no son \
tot zeros si no %c (valor del char)\n", j, i, blck.valor[j]);
				correcte = 0;
			}
		}
	}
	if (correcte == 1) {
		printf("\t - Sembla que el sistema de fitxers s'ha omplert de \
zeros correctament\n");
	}
	bumount();
	printf("\n\nDesmuntat sistema de fitxers, primera part del test \
passada correctament, ara pots mirar el fitxer (han de ser tot zeros).  \
\nPitja qualsevol tecla per continuar.\n");
	getc(stdin);

	/* Test 1: Canviam els zeros dels blocs senars per uns */
	f_fs = bmount(nom_fs);
	blck_one = block_of_one();
	for (i = 0; i < num_blocs; i++) {
		if (i % 2 == 1) {
			bwrite(i, &blck_one);
		}
	}
	for (i = 0; i < num_blocs; i++) {
		bread(i, &blck);
		if (i % 2 == 0) {
		/* Han de ser zeros */
			for (j = 0; j < BLOCK_SIZE; j++){
				if (blck.valor[j] != 0x00) {
					printf("\t - La posicio %d del bloc \
 %d no son tot zeros sino %c (valor del char)\n", j, i, blck.valor[j]);
					correcte = 0;
				}
			}
		} else {
			for (j = 0; j < BLOCK_SIZE; j++){
				if (blck.valor[j] != 0xff) {
					printf("\t - La posicio %d del bloc \
 %d no son tot uns sino %c (valor del char)\n", j, i, blck.valor[j]);
					correcte = 0;
				}
			}
		}
	}
	if (correcte == 1) {
		printf("\t - Sembla que el sistema de fitxers s'ha omplert de \
zeros i uns correctament\n");
	}
	bumount();
	printf("\n\nDesmuntat sistema de fitxers, segona part del test \
passada correctament, ara pots mirar el fitxer (zeros i uns alternats).\
\nPitja qualsevol tecla per continuar.\n");
	getc(stdin);

	/* Test 2: Posam el valor 0xee a tots els bytes d'un block aleatori */
	f_fs = bmount(nom_fs);

	for (i = 0; i < BLOCK_SIZE; i++){
		blck_0xee.valor[i] = 0xee;
	}

        gettimeofday(&tv,NULL);
        srand(tv.tv_usec);
	bloc_aleatori = rand() % num_blocs;

	bwrite(bloc_aleatori, &blck_0xee);
	bread(bloc_aleatori, &blck);

	for(i = 0; i < BLOCK_SIZE && correcte; i++){
		if (blck.valor[i] != blck_0xee.valor[i]) {
			correcte = 0;
		}
	}

	if (correcte) {
		printf("El bloc %d, s'ha omplert satifactoriament \
de \"0xee\". \n", bloc_aleatori);
	} else {
		printf("El bloc %d, *NO* s'ha omplert satifactoriament \
de \"0xee\". \n", bloc_aleatori);
	}

}
