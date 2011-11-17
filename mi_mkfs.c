#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "block.h"
#include "inode.h"

#include "block_lib.h"

#include "dir.h"

#define DEFAULT_BLOCKS 1000

/* Some fun */
#define AUTHOR_COUNT 2
#define AUTHOR_B "bartomeumiro.ascii"
#define AUTHOR_P "paurullan.ascii"

/*
 * Creació del fitxer on es té el sistema de fitxers.
 * mi_mkfs <cantidad_bloques>
 * codis de retorn:
 * 	0: correcte
 * 	-1: nombre de paràmetres incorrecte
 */
int main(int argc, char **argv) {
	int i = 0;
	int nombre_blocs;
	int n_inode;
	emofs_superblock sb;
	emofs_inode inode;
	/* Some fun */
	char author[][20] = { AUTHOR_B, AUTHOR_P };
	char buf[80];
	char path[MAX_PATH_LEN];
	int ascii_art; /* ASCII art, descriptor de fitxer */
	int pos;
	int read_bytes;

	emofs_block bloc_zero = block_of_zero();

	if (argc != 2) {
		nombre_blocs = DEFAULT_BLOCKS;
	} else {
		nombre_blocs = atoi(argv[1]);
	}

	bmount();

	/* Inicialitzam tot el sistema de fitxers */
	for (i = 0; i < nombre_blocs; i++) {
		bwrite(i, &bloc_zero);
	}
 
	init_superblock(nombre_blocs);
	init_bitmap();
	init_inode_array();

	/* Problema de bootstrap. Es necessita una ruta per crear un fitxer. */
	n_inode = alloc_inode(DIRECTORY_INODE);
	sbread(&sb);
	sb.root_inode = n_inode;
	sbwrite(&sb);

	/* JUST FOR FUN */
	/* Cream directori /authors */
	if (fork() == 0) {
		execl("mi_mkdir", "mi_mkdir", "/authors", (char *)0);
	} else {
		wait3(NULL, 0, NULL);
	}
	/* Cream i omplim /authors/author[i].ascii" */
	for (i = 0; i < AUTHOR_COUNT; i++) {	
		ascii_art = open(author[i], O_RDONLY, S_IRUSR);
		strcpy(path, "/authors/");	
		strcat(path, author[i]);
		emofs_create(path);
		pos = 0;
		read_bytes = 1; /* per entrar al bucle */
		while (read_bytes > 0) { /* Sortira amb el break */
			lseek(ascii_art, pos, SEEK_SET);
			read_bytes = read(ascii_art, buf, 80);
			if (!read_bytes) break;
			emofs_write(path, buf, pos, read_bytes);
			pos += read_bytes;
		}


		close(ascii_art);
	}
	/* ending fun */

    	bumount();

	return 0;
}
