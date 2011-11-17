#include <stdio.h>
#include <stdlib.h>
#include "block.h"
#include "bitmap.h"
#include "inode.h"
#include "block_lib.h"

int comprovar(int n_inodes, int n_blocs);

int main(int argc, char **argv) {
	int n, ni, nb, i;
	int *inodes;
	int *blocs;
	emofs_superblock sb;
	emofs_inode inode;

	if (argc != 4) {
        puts("Nombre de paràmetres incorrecte.");
        puts("Ús: tests_setmana3 <nom> <n inodes a reservar> \
 <nombre blocs a reservar");
		return -1;
	}

	ni = atoi(argv[2]);
	nb = atoi(argv[3]);

	printf("Iniciat el test sobre el fitxer %s, en un prinicipi es \
reserveran %d inodes i %d blocs.\n", argv[1], ni, nb);

	bmount(argv[1]);

	sbread(&sb);

	inodes = malloc(sizeof(int)*ni);
	blocs = malloc(sizeof(int)*nb);

	/* Reservar ni inodes i nb blocks */
	for (i = 0; i < ni; i++) {
		inodes[i] = alloc_inode(FILE_INODE);
		printf("Reservat inode %d, es el %d\n", i, inodes[i]);
	}

	for (i = 0; i < nb; i++) {
		blocs[i] = alloc_block();
		printf("Reservat el bloc %d, es el %d\n", i, blocs[i]);
	}

	/* Comprovar */
	comprovar(ni, nb);


	printf("Alliberam la primera meitat\n");
	/* Alliberar ni/2 inodes i nb/2 blocks */
	for (i = 0; i < ni/2; i++) {
		free_inode(inodes[i]);
	}

	for (i = 0; i < nb/2; i++) {
		free_block(blocs[i]);
	}

	/* Comprovar */
	printf("Comprovam que hem alliberat bé la primera meitat\n");
	comprovar((ni/2 + (ni % 2)), (nb/2 + (nb % 2)));

	/* Alliberar tots */	
	for (i = 0; i < sb.total_inode_count; i++) {
		read_inode(i, &inode);
		if (inode.type != FREE_INODE) {
			free_inode(i);
		}
	}

	for (i = 0; i < sb.total_block_count; i++) {
		if (bm_read(i) != 0) {
			free_block(i);
		}
	}

	/* Comprovar */
	comprovar(0, 0);
	
	puts("Anam a reservar tot el que poguem");
	/* Reservar tots */
	for (i = 0; i < sb.total_inode_count; i++) {
		alloc_inode(FILE_INODE);
	}
	puts("Reservats tots els inodes");
	puts("Anam a reservar tots els blocs de dades");
	for (i = 0; i < sb.total_data_block_count; i++) {
		alloc_block();
	}
	puts("Reservats tots els blocs de dades");
	/* Comprovar */
	comprovar(sb.total_inode_count, sb.total_data_block_count);

	
	puts("Tornam a alliberar-ho tot.");
	/* Alliberar tots */	
	for (i = 0; i < sb.total_inode_count; i++) {
		read_inode(i, &inode);
		if (inode.type != FREE_INODE) {
			free_inode(i);
		}
	}

	for (i = 0; i < sb.total_block_count; i++) {
		if (bm_read(i) != 0) {
			free_block(i);
		}
	}

	/* Comprovar */
	comprovar(0, 0);

	bumount();
}

int comprovar(int n_inodes, int n_blocs) {
	int i, inodes_usats, blocs_usats;
	emofs_superblock sb;
	emofs_inode inode;

	sbread(&sb);

	printf("contador inodes: %d\n", sb.total_inode_count);
	inodes_usats = 0;
	for (i = 0; i < sb.total_inode_count; i++) {
		read_inode(i, &inode);
		if (inode.type != FREE_INODE) {
			inodes_usats++;
		}
	}

	if (inodes_usats != n_inodes) {
		printf("Error: S'havien de reservar %d inodes ", n_inodes);
		printf("i se n'han reservat %d\n", inodes_usats);
	} else {
		printf("S'ha reservat el nombre d'inodes correcte.\n");
	}

	if ((sb.total_inode_count - sb.free_inode_count) != n_inodes ) {
		printf("L'informació del SB respecte els inodes ocupats es incorrecta");
		printf("hi ha %d i haurien de ser %d\n", 
		(sb.total_inode_count - sb.free_inode_count), n_inodes);
	} else {
		printf("La informació del SB respecte els inodes ocupats és correcta\n");
	}
	
	blocs_usats = 0;
	for (i = 0; i <= (sb.last_data_block - sb.first_data_block); i++) {
		if (bm_read(i) == 1) {
			blocs_usats++;
		}
	}

	if (blocs_usats != n_blocs) {
		printf("Error: S'havien de reservar %d blocs ", n_blocs);
		printf("i se n'han reservat %d\n", blocs_usats);
	} else {
		printf("S'ha reservat el nombre blocs correcte.\n");
	}

}
