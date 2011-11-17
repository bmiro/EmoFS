#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inode.h"
#include "block.h"
#include "super.h"

/* Totes les operacions d'inodes venen després d'una lectura de bloc. Cal
 * recordar que dins un mateix bloc tenim varis inodes i sols podem modificar
 * amb el que esteim treballant.
 */

/** Escriu un inode.
 * @n_inode: el nombre de l'inode a escriure.
 * @buf: l'inode d'entrada.
 * @date: hora de modificació de l'inode.
 * @return: -1 si l'inode objectiu no existeix.
 */	
int write_inode_time(int n_inode, emofs_inode *buf, time_t date) {
	int inode_block, inode_pos;
	int error;
	emofs_superblock sb;
	emofs_inode inode_array[INODES_PER_BLOCK];

	sbread(&sb);
	inode_block = sb.first_inode_block + (n_inode/INODES_PER_BLOCK); 
	inode_pos = n_inode % INODES_PER_BLOCK;
	error = bread(inode_block, inode_array);
	memcpy(&buf->mtime, &date, sizeof(time_t));
	memcpy(&inode_array[inode_pos], buf, sizeof(emofs_inode));
	bwrite(inode_block, inode_array);	
	return error;
}

/** Escriu un inode.
 * @n_inode: el nombre de l'inode a escriure.
 * @buf: l'inode d'entrada.
 * @return: -1 si l'inode objectiu no existeix.
 */
int write_inode(int n_inode, emofs_inode *buf) {
	return write_inode_time(n_inode, buf, time(NULL));
}

/** Llegeix un inode.
 * @n_inode: el nombre de l'inode a llegir.
 * @buf: buffer on es desa el contingut de l'inode.
 * @return: -1 si el nombre d'inode no existia.
 */
int read_inode(int n_inode, emofs_inode *buf) {
	int inode_block = -1;
	int inode_pos = -1;
	int error = 0;
	emofs_superblock sb;
	emofs_inode inode_array[INODES_PER_BLOCK];

	if (n_inode < 0) {
		printf("read_inode: inode negatiu\n");
		return -2;
	}

	error = sbread(&sb);
	inode_block = sb.first_inode_block + (n_inode / INODES_PER_BLOCK);
	inode_pos = n_inode % INODES_PER_BLOCK;

	if (inode_block < 0) {
		printf("read_inode: bloc d'inode negatiu\n");
		return -3;
	} else if (inode_pos < 0) {
		printf("read_inode: posició d'inode negatiu\n");
		return -4;
	}
	
	bread(inode_block, inode_array);
	memcpy(buf, &inode_array[inode_pos], sizeof(emofs_inode));

	return error;
}

/** Situa dins l'estructura de localització un número de bloc de dades.
 * @n_block: nombre sobre el qual fer el càlcul.
 * @loc: l'estructura de posicionament.
 * @return: 0 si èxit.
 */
int localize_data(int n_block, emofs_data_loc *loc) {
	int level, found;
	int max[INDIRECT_POINTER_COUNT+1];
	int group, pos;
	/* Inicialització del vector de màxims.
	 * maxim nivell = punters directes + \Sum indirectes^nivell
	 */
	max[0] = DIRECT_POINTER_COUNT;
	max[1] = LEVEL_1_POINTERS + max[0];
	max[2] = LEVEL_2_POINTERS + max[1];
	max[3] = LEVEL_3_POINTERS + max[2];

	/* Búsqueda del nivell. */
	found = 0;
	level = 0;
	while ((!found) && (level < INDIRECT_POINTER_COUNT+1)) {
		if (n_block < max[level]) {
			found = 1;
		} else {
			level++;
		}
	}

	if(!found) {
		puts("Petició d'un bloc fora de rang");
		return -1;
	}

	loc->n = level;
	switch(level) {
	case 0:
		loc->l[0] = n_block;
		break;
	case 1:
		loc->l[0] = n_block - max[0];
		break;
	case 2:
		n_block -= max[1];
		loc->l[1] = n_block / LEVEL_2_POINTERS;
		loc->l[0] = n_block % LEVEL_2_POINTERS;
		break;
	case 3:
		n_block -= max[2];
		n_block /= LEVEL_2_POINTERS;
		loc->l[2] = n_block;
		loc->l[1] = n_block / LEVEL_2_POINTERS;
		loc->l[0] = n_block % LEVEL_2_POINTERS;
		break;
	default:
		puts("Nombre de nivells incorrecte");
		return -2;
	}

	return 0;
}

/** Inicialitza un bloc de punters a NULL_POINTER. Funció interna. */
int init_ind_pointers(int ptrs[]) {
	int i;
	
	for (i=0; i < INDIRECT_POINTERS_PER_BLOCK; i++) {
		ptrs[i] = NULL_POINTER;
	}

	return 0;
}

/*
	bread(n_block, &i_block);
	data = i_block[loc->l[n]];
	if ((data == NULL_POINTER) && alloc) {
		data = alloc_block();
		if (data < 0) {
			return -2;
		}
		init_ind_pointers(i_block);
		i_block[loc->l[n]] = data;
		bwrite(n_block, &i_block);
		inode.block_count++;
		write_inode(n_inode, &inode);
	}
*/

int translate_indirect_1(int *block, emofs_inode *inode,
			emofs_data_loc *loc, int alloc);
int translate_indirect_2(int *block, emofs_inode *inode,
			emofs_data_loc *loc, int alloc);
int translate_indirect_3(int *block, emofs_inode *inode,
			emofs_data_loc *loc, int alloc);


/** Busca i reserva un nou bloc de dades dins els punters directes.
 * @data: el nombre del bloc de dades
 * @inode: punter a l'inode amb què treballam
 * @pos: posició dins el vector directe
 * @alloc: si cal reservar en cas que estigui lliure
 * @return: 0 si èxit,
 *         -2 si no hi ha espai,
 *         -1 si es vol llegir sense existir el bloc
 */
int translate_inode_indirect(int *block, emofs_inode *inode,
			     emofs_data_loc *loc, int alloc) {
	int error;

	switch(loc->n) {
	case 1: error = translate_indirect_1(block, inode, loc, alloc); break;
	case 2: error = translate_indirect_2(block, inode, loc, alloc); break;
	case 3: error = translate_indirect_3(block, inode, loc, alloc); break;
	default:
		puts("traduir_indirecte: Nivells incorrectes");
		error = -3;
		break;
	}
	return error;
}

int translate_indirect_1(int *block, emofs_inode *inode,
			 emofs_data_loc *loc, int alloc){
	int n_indirect = inode->indirect_pointer[0];
	int data_indirect[INDIRECT_POINTERS_PER_BLOCK];
	if(n_indirect == NULL_POINTER) {
		if (alloc) {
			n_indirect = alloc_block();
			inode->indirect_pointer[0] = n_indirect;
			/* Inicialitzam el block de punters */
			init_ind_pointers(data_indirect);
			bwrite(n_indirect, data_indirect);
			inode->block_count++;
		} else{
			return -1;
		}
	}
	bread(n_indirect, data_indirect);
	*block = data_indirect[loc->l[0]];
	if (*block == NULL_POINTER) {
		if (alloc) {
			*block = alloc_block();
			if (*block < 0) {
				return -2;
			}
		} else {
			return -1;
		}
		data_indirect[loc->l[0]] = *block;
		bwrite(n_indirect, data_indirect);
		inode->block_count++;
	}
	return 0;
}

int translate_indirect_2(int *block, emofs_inode *inode,
			 emofs_data_loc *loc, int alloc){
	int n_indirect2, n_indirect1;
	int data_indirect2[INDIRECT_POINTERS_PER_BLOCK];
	int data_indirect1[INDIRECT_POINTERS_PER_BLOCK];

	n_indirect2 = inode->indirect_pointer[1];
	if (n_indirect2 == NULL_POINTER) {
		if (alloc) {
			n_indirect2 = alloc_block();
			inode->indirect_pointer[1] = n_indirect2;
			/* Inicialitzam el block de punters a punters */
			init_ind_pointers(data_indirect2);
			bwrite(n_indirect2, data_indirect2);
			inode->block_count++;
		} else {
			return -1;
		}
	}
	bread(n_indirect2, data_indirect2);
	n_indirect1 = data_indirect2[loc->l[1]];
	if (n_indirect1 == NULL_POINTER) {
		if (alloc) {
			n_indirect1 = alloc_block();
			/* Inicialitzam el block de punters a dades */
			init_ind_pointers(data_indirect1);
			bwrite(n_indirect1, data_indirect1);
			data_indirect2[loc->l[1]] = n_indirect1;
			bwrite(n_indirect2, data_indirect2);
			inode->block_count++;
		} else {
			return -1;
		}
	}
	bread(n_indirect1, data_indirect1);
	*block = data_indirect1[loc->l[0]];
	if (*block == NULL_POINTER) {
		if (alloc) {
			*block = alloc_block();
			if (*block < 0) {
				return -2;
			}
		} else {
			return -1;
		}
		data_indirect1[loc->l[0]] = *block;
		bwrite(n_indirect1, data_indirect1);
		inode->block_count++;
	}
	return 0;
}


int translate_indirect_3(int *block, emofs_inode *inode,
			emofs_data_loc *loc, int alloc) {
	int n_indirect3, n_indirect2, n_indirect1;
	int data_indirect3[INDIRECT_POINTERS_PER_BLOCK];
	int data_indirect2[INDIRECT_POINTERS_PER_BLOCK];
	int data_indirect1[INDIRECT_POINTERS_PER_BLOCK];

	n_indirect3 = inode->indirect_pointer[2];
	if(n_indirect3 == NULL_POINTER) {
		if (alloc) {
			n_indirect3 = alloc_block();
			inode->indirect_pointer[2] = n_indirect3;
			/* Inicialitzam el block de punters a punters a punters */
			init_ind_pointers(data_indirect3);
			bwrite(n_indirect3, data_indirect3);
			inode->block_count++;
		} else {
			return -1;
		}
	}
	bread(n_indirect3, data_indirect3);
	n_indirect2 = data_indirect3[loc->l[1]];
        /* Segon nivell */
	if (n_indirect2 == NULL_POINTER) {
		if (alloc) {
			n_indirect2 = alloc_block();
			data_indirect3[loc->l[2]] = n_indirect2;
			bwrite(n_indirect3, data_indirect3);
			/* Inicialitzam el block de punters a punters a dades */
			init_ind_pointers(data_indirect2);
			bwrite(n_indirect2, data_indirect2);
			inode->block_count++;			
		} else {
			return -1;
		}
	}
	bread(n_indirect2, data_indirect2);
	n_indirect1 = data_indirect2[loc->l[1]];
	/* Primer nivell */
        if(n_indirect1 == NULL_POINTER) {
		if (alloc) {
			n_indirect1 = alloc_block();
			data_indirect2[loc->l[1]] = n_indirect1;
			bwrite(n_indirect2, data_indirect2);
			/* Inicialitzam el block de punters a dades*/
			init_ind_pointers(data_indirect1);
			bwrite(n_indirect1, data_indirect1);
			inode->block_count++;
		} else {
			return -1;
		}
	}
	bread(n_indirect1, data_indirect1);
	*block = data_indirect1[loc->l[0]];
	if (*block == NULL_POINTER) {
		if (alloc) {
			*block = alloc_block();
			if (*block < 0) {
				return -2;
			}
		} else {
			return -1;
		}
		data_indirect1[loc->l[0]] = *block;
		bwrite(n_indirect1, data_indirect1);
		inode->block_count++;
	}
	return 0;
}
    
/** Busca i reserva un nou bloc de dades dins els punters directes.
 * @data: el nombre del bloc de dades
 * @inode: punter a l'inode amb què treballam
 * @pos: posició dins el vector directe
 * @alloc: si cal reservar en cas que estigui lliure
 * @return: 0 si èxit,
 *         -2 si no hi ha espai,
 *         -1 si es vol llegir sense existir el bloc
 */
int translate_inode_direct(int *block, emofs_inode *inode, int pos, int alloc) {
	*block = inode->direct_pointer[pos];
	if (*block == NULL_POINTER) {
		if (alloc) {
			*block = alloc_block();
			if (*block < 0) {
				return -2;
			}
			inode->direct_pointer[pos] = *block;
			inode->block_count++;
		} else{
			return -1;
		}
	}
	return 0;
}


/** Tradueix el bloc lògic al seu valor de bloc físic.
 * Cal recordar que si afegim un bloc de dades hem de modificar el contador
 * d'inodes però no corregim el tamany de fitxer.
 * @n_inode: nombre d'inode
 * @l_block: número de bloc lògic.
 * @block: número de bloc físic
 * @alloc: si 1 i no existeix el bloc físic reserva un bloc de dades.
 * @return: 0 si èxit. -1 si es volia llegir i no existia. -2 en cas de
 * problemes en la solicitud de blocs (escriptura).
 */
int translate_inode(int n_inode, int l_block, int *block, int alloc) {
	int level, i, n_block;
	emofs_inode inode;
	int i_block[INDIRECT_POINTERS_PER_BLOCK];
	emofs_data_loc loc;	
	int error;

	error = read_inode(n_inode, &inode);
	if (error < 0) {
		puts("No s'ha trobat l'inode");
		return error;
	}	
	localize_data(l_block, &loc);
	if (loc.n == 0) {
		error = translate_inode_direct(block, &inode, loc.l[0], alloc);
	} else {
		error = translate_inode_indirect(block, &inode, &loc, alloc);
	}
	write_inode(n_inode, &inode);
	return error;
}
