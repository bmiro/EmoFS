#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "block.h"

static int emofs_fs = 0;
static int mutex = 0;

emofs_block block_of_zero() {
	emofs_block new_block;
	memset(&new_block, 0x00, BLOCK_SIZE);
	return new_block;
}

emofs_block block_of_one() {
	emofs_block new_block;
	memset(&new_block, 0xff, BLOCK_SIZE);
	return new_block;
}

/** Munta la imatge del sistema de fitxers. Posa en marxa el semàfor.
 * @cami: camí de la imatge
 * @return: 0 si èxit
 */
int bmount(void) {
	emofs_fs = open(EMOFS_IMAGE_FILE, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
	if (emofs_fs == -1) {
		printf("Error en crear el fitxer: %s\n", strerror(errno));
	}
	return emofs_fs;
}

int bumount(void) {
	int error;
	error = close(emofs_fs);
	if (error == -1) {
		printf("Error al tancar el fitxer: %s\n", strerror(errno));
	}
	return error;
}

int bwrite(int bloque, const void *buf) {
	int posicio_bloc = bloque * BLOCK_SIZE;
	int error = 0;
	error = lseek(emofs_fs, posicio_bloc, SEEK_SET);
	if (error == -1) {
		printf("bwrite: error en lseek: %s\n", strerror(errno));
		exit(-1);
	}
	error = write(emofs_fs, buf, BLOCK_SIZE);
	if (error == -1) {
		printf("Error en escriure al fitxer: %s\n", strerror(errno));
		exit(-1);
	}
	return 0;
}

int bread(int bloque, void *buf) {
	int posicio_bloc = bloque * BLOCK_SIZE;
	int error = 0;
	error = lseek(emofs_fs, posicio_bloc, SEEK_SET);
	if (error == -1) {
		printf("bread: error en lseek: %s\n", strerror(errno));
		printf("posicio_bloc: %d\n", posicio_bloc);
		exit(-1);
	}
	error = read(emofs_fs, buf, BLOCK_SIZE);
	if (error == -1) {
		printf("A la lectura: %s\n", strerror(errno));
		exit(-1);
	}
	return 0;
}

