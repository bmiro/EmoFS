#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "block.h"
#include "inode.h"
#include "bitmap.h"
#include "block_lib.h"

int bitmap_size(int n_block) {
	int no_exacte;

	no_exacte = ((n_block % MAP_SIZE) != 0);
	return (n_block/MAP_SIZE) + no_exacte;
}

/** Càlcul del nombre d'inodes. Permetrem tenir tants de inodes com blocs de
 * dades aixi el nombre de fitxers petits i no buits serà màxim. Recodem que el
 * tamany de l'inode esta triat per a que sigui un divisor del tamany del bloc
 * per no tenir problemes */

int inode_array_size(int n_inode) {
	int no_exacte, n_blocks;
	if ((BLOCK_SIZE % sizeof(emofs_inode)) != 0) {
		printf("Error en el calcul del tamany de l'array de inodes\n");
		return -1;
	}
	if (n_inode*sizeof(emofs_inode) % BLOCK_SIZE) {
		no_exacte = 1;
	} else {
		no_exacte = 0;
	}
	n_blocks = n_inode * sizeof(emofs_inode) / BLOCK_SIZE;
	return n_blocks + no_exacte;
}

int init_superblock(int n_block) {
	emofs_superblock sb;
	int i;
	
	/* El superbloc nomes ocupa el primer bloc */
	sb.first_bitm_block = 1; 
	sb.last_bitm_block = sb.first_bitm_block + bitmap_size(n_block) - 1;
	sb.first_inode_block = sb.last_bitm_block + 1;
	/* Hem establert que tindem un inode per cada block */
	sb.last_inode_block = sb.first_inode_block + \
		              inode_array_size(n_block) - 1;
	sb.first_data_block = sb.last_inode_block + 1;
	/* Començam a comptar a partir de 0 per tant -1 */
	sb.last_data_block = n_block - 1;
	sb.root_inode = 0;
	sb.first_free_inode = 0;
	sb.free_block_count = sb.last_data_block - sb.first_data_block + 1;
	sb.free_inode_count = (1 + sb.last_inode_block - sb.first_inode_block) \
		              * BLOCK_SIZE/sizeof(emofs_inode);
	sb.total_block_count = n_block;
	sb.total_inode_count = sb.free_inode_count;
	sb.total_data_block_count = sb.last_data_block-sb.first_data_block+1;
	
	for(i = 0; i < PADDING_BYTES; i++) {
		sb.padding[i] = 'p';
	}
	return sbwrite(&sb);
}

/** Posar a zero tots els blocs lliures */
int init_bitmap() {
	int i;
	emofs_superblock sb;
	emofs_block one;
	
	one = block_of_one();

	sbread(&sb);
	for(i = sb.first_bitm_block; i <= sb.last_bitm_block; i++) {
		bwrite(i, &one);
	}
	for(i = sb.first_data_block; i < sb.total_data_block_count; i++) {
		bm_write(i, 0);
	}
	return 0;
}

int init_inode_array() {
	int i;
	emofs_inode inode;
	emofs_superblock sb;

	sbread(&sb);

	/* Inicialitzam l'inode */
	inode.type = FREE_INODE;
	inode.size = 0;
	inode.mtime = 0;
	inode.block_count = 0;
	inode.link_count = 0;
	for(i = 0; i < DIRECT_POINTER_COUNT; i++) {
		inode.direct_pointer[i] = 0;
	}
	for(i = 0; i < INDIRECT_POINTER_COUNT; i++) {
		inode.indirect_pointer[i] = 0;
	}

	/* Enllaçar inodes */
	for (i = 0; i < (sb.total_inode_count - 1); i++) {
		inode.size = i + 1;
		write_inode(i, &inode);
	}

	/* Indicam que es el darrer inode */
	inode.size = -1;
	write_inode(i, &inode);
	sb.first_free_inode = 0;
	sbwrite(&sb);
	return 0;
}


/** Reserva un bloc. S'encarrega de busca-lo, reservar-lo, deixar-lo marcat al
 * mapa de bits i tornar-lo per poder treballar sobre ell. Modifica el nombre
 * total de blocs del superbloc. Torna -1 si hi no havia blocs lliures.
 * @return: el número del bloc reservat
 */
int alloc_block() {
	int block;
	emofs_superblock sb;
	sbread(&sb);

	if (sb.free_block_count == 0) {
		printf("alloc_block: No hi ha blocs lliures\n");
		return -1;
	}

	block = find_first_free_block();
	if (block < 0) {
		printf("alloc_block: No s'ha trobat bloc lliure\n");
		return -2;
	}

	bm_write(block, 1);
	sb.free_block_count--;
	sbwrite(&sb);
        return block;
}

int free_block(int n_block) {
	emofs_superblock sb;
	if (bm_read(n_block) == 0) {
		puts("free_block: s'ha intentat alliberar un bloc lliure");
		return -1;
	}

	sbread(&sb);
        bm_write(n_block, 0);
	sb.free_block_count++;
	sbwrite(&sb);
	return 0;
}

/* Totes les operacions d'inodes venen després d'una lectura de bloc. Cal
 * recordar que dins un mateix bloc tenim varis inodes i sols podem modificar
 * amb el que esteim treballant.
 */

/** Reserva un inode. Modifica el nombre d'inodes lliures i totals del
 * superbloc.  Cal inicialitzar totes les dades de l'inode: tipus, tamany,
 * mtime, recomptes i posar a zero els punters directes i indirectes. Modificar
 * la llista enllaçada d'inodes lliures.
 * @return: número d'inode, -1 si no en queden o es produeix un error.
 */
int alloc_inode(int type) {
	emofs_superblock sb;
	emofs_inode inode;
	int i, n_inode, error;

        if (!(type == DIRECTORY_INODE || type == FILE_INODE)) {
                printf("alloc_inode: Incorrect inode type: %d\n", type);
                return -2;
        }
	
	sbread(&sb);

	if (sb.free_inode_count == 0 || sb.first_free_inode == -1) {
		puts("alloc_inode: No free inodes left!");
		return -1;
	}

	sb.free_inode_count--;
	n_inode = sb.first_free_inode;

	error = read_inode(n_inode, &inode);	
	if (error == -1) {
		puts("alloc_inode: Error while reading first free inode");
                sbwrite(&sb);
		return -1;
	}

	if (inode.type != FREE_INODE) {
		puts("alloc_inode: corrupted filesystem");
                puts("alloc_inode: no free inode found in free inode list");
                sbwrite(&sb);
		return -1;
	}

	/* Hem definit que dins el camp size desam el següent inode lliure. Si
	 * fos final de llista deixeriem al SB un -1. D'aquesta manera
	 * detectariem que la llista és buida. Si el primer bloc lliure és el
	 * -1.
	 */
	sb.first_free_inode = inode.size;
	inode.type = type;
	inode.size = 0;
	inode.mtime = time(NULL);
	inode.block_count = 0;
	inode.link_count = 0;

	for(i = 0; i < DIRECT_POINTER_COUNT; i++) {
		inode.direct_pointer[i] = NULL_POINTER;
	}
	for(i = 0; i < INDIRECT_POINTER_COUNT; i++) {
		inode.indirect_pointer[i] = NULL_POINTER;
	}

	write_inode(n_inode, &inode);	
	sbwrite(&sb);

	return n_inode;
	
}

int free_indirect_inode(int n_indirect, int level, emofs_superblock *sb) {
	int i;
	int indirect[INDIRECT_POINTERS_PER_BLOCK];
	bread(n_indirect, &indirect);

	if (level == 0) {
		puts("free_indirect_inode: Ús incorrecte");
	} else if (level == 1) {
		for (i = 0; i < INDIRECT_POINTERS_PER_BLOCK; i++) {
			if (indirect[i] != NULL_POINTER) {
				bm_write(indirect[i], 0);
				sb->free_block_count++;
			}
		}
		bm_write(n_indirect, 0);
		sb->free_block_count++;
	} else {
		for (i = 0; i < INDIRECT_POINTERS_PER_BLOCK; i++) {
			if (indirect[i] != NULL_POINTER) {
				free_indirect_inode(indirect[i], level-1, sb);
			}
		}
	}
		
	return 0;
}

/** Allibera un inode. Fa el recorregut de tots els inodes indirectes. Allibera
 * també tots els blocs de dades.  Modifica el nombre d'inodes i blocs lliures 
 * del superbloc. Després afegeix l'inode actual a la llista d'inodes lliures.
 * @n_inode: número de l'inode a lliberar.
 * @return: -1 si el número d'inode no era correcte.
 */
int free_inode(int n_inode) {
	emofs_superblock sb;
	emofs_inode inode;
	int *pointers = malloc((INDIRECT_POINTER_COUNT+1)*sizeof(int));
	int i, j, k, error;

	sbread(&sb);
	error = read_inode(n_inode, &inode);	
	if (error < 0) {
		puts("free_inode: no s'ha pogut llegir inode");
		return -1;
	}

	if (inode.type == FREE_INODE) {
		puts("free_niode: s'ha intentat alliberar-ne un de lliure");
		return -2;
	}

	for (i = 0; i < DIRECT_POINTER_COUNT; i++) {
		if (inode.direct_pointer[i] != NULL_POINTER) {
			printf("free_inode: alliberam bloc %d\n", \
			       inode.direct_pointer[i]);
			bm_write(inode.direct_pointer[i], 0);
			sb.free_block_count++;
		}	
	}

	for (i = 0; i < INDIRECT_POINTER_COUNT; i++) {
		if (inode.indirect_pointer[i] != NULL_POINTER) {
			free_indirect_inode(inode.indirect_pointer[i], i+1, &sb);
		}
	}

	/* Situam com a lliure i enllaçam la llista */
	inode.type = FREE_INODE;
	inode.size = sb.first_free_inode;
	write_inode(n_inode, &inode);

	/* Establim l'inode com el primer lliure */
	sb.first_free_inode = n_inode;
	sb.free_inode_count++;
	
	sbwrite(&sb);
	return 0;

}

