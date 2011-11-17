#include <string.h>
#include <stdlib.h>

#include "file.h"
#include "inode.h"
#include "block.h"
#include "block_lib.h"

/* 
Calculam el bloc a que pertany el primer byte a llegir.
Llegim l'inode.
Tres casos:
	- acabar el primer bloc.
	- llegir completament els blocs intermitjos.
	- comencar el darrer bloc.
D'aquesta manera es bona idea calcular previament a quin cas
pertany.
*/

/** Funcio interna per calcular els blocs necessaris per la tasca.
 * Recorda que BLOCK_SIZE determina el nombre de bytes per bloc.
 * @offset: desplacament inicial.
 * @n_bytes: bytes a llegir.
 * @first: nombre de bloc del primer de tots.
 * @f_offset: desplacament dins el primer bloc.
 * @l_size: tamany del darrer bloc si n'hi ha mes d'un.
 * @return: 	1 sols primer
 *		2 primer i segon
 *		3+ un de comencament, final i alguns intermijos
 */
int calc_blocks_with_offset(int offset, int n_bytes, 
			    int *first, int *f_offset, int *l_size) {
	int how_many = 0;
	*first = offset / BLOCK_SIZE;
	*f_offset = offset % BLOCK_SIZE;
	*l_size = ((*f_offset) + n_bytes)%BLOCK_SIZE;
	how_many = (*f_offset + n_bytes)/BLOCK_SIZE;
	how_many += ((*f_offset + n_bytes) % BLOCK_SIZE) ? 1 : 0;
	return how_many;

}

/** Escriu un cert nombre de bits d'un fitxer.  Cal recordar que si indicam un
 * offset = 10 significa que hem de comencar per el byte nombre 10 del fitxer.
 * Es important actualitzar el tamany del fitxer.  a escriure des del byte
 * numero 10.
 * @inode: nombre d'inode on volem escriure
 * @buffer: buffer d'entrada. D'alla copiarem les dades.
 * @offset: determinam el primer byte del fitxer.
 * @n_bytes: nombre de bytes a escriure
 * @return: el nombre de bytes escrits.
 */
int write_file(int inode, const void *buffer, int offset, int n_bytes) {
	int error, i;
	/* quants de blocs tenim a llegir, nombre de bloc logic a treballar */
	int how_many = 0;
	int block = 0;
	/* Nombre de blocs llegits, necessari pels offsets del memcpy. */
	int n_blocks_written = 0;
	int n_bytes_written = 0;
	int n_block = 0; /* nombre del primer bloc */
	int f_offset = 0; /* offset del primer bloc */
	int l_size = 0; /* tamany al darrer bloc */
	int bytes_copy = 0; /* Bytes a escriure a la fase */
	int bytes_offset = 0; /* Bytes a desplacar-se */
	int grow_size = 0;
	emofs_block tmp_block;
	emofs_inode tmp_inode;
	/* recorda: offset, bytes a llegir, primer block, offset del primer,
	 * final del darrer */
	how_many = calc_blocks_with_offset(offset, n_bytes, \
					   &block, &f_offset, &l_size);
	if (how_many < 1) {
		puts("read_file: error al calcul de la posicio inicial");
		return n_bytes_written;
	}

	/* La idea es igual que a la lectura: fer les tres seccions on:
		Obtenim el bloc logic.
		Escrivim sobre ell.
		El tornam a disco.
	Calculam el canvi de tamany al final de tot. */
	error = translate_inode(inode, block, &n_block, 1);
	if(error == -2) {
		puts("write_file: no queda espai lliure");
		return n_bytes_written;
	}
	bread(n_block, &tmp_block);
	if (n_bytes < (BLOCK_SIZE - f_offset)) {
		bytes_copy = n_bytes;
	} else {
		bytes_copy = BLOCK_SIZE - f_offset;
	}
	memcpy(tmp_block.valor + f_offset, buffer, bytes_copy);
	bwrite(n_block, &tmp_block);
	n_blocks_written = 1;
	n_bytes_written += bytes_copy;
	how_many--;

	/* Importantissim: comencam a contar des de un ja que
	   que hem llegit el primer bloc. Aqui treballam amb els blocs
	   intermijos. */
	for (/*res*/; how_many -1 > 0; how_many--) {
		block++;
		error = translate_inode(inode, block, &n_block, 1);
		if (error == -2) {
			puts("write_file: no queda espai lliure");
			return n_bytes_written;
		}
		bread(n_block, &tmp_block);
		memcpy(tmp_block.valor,(char*)buffer + n_bytes_written, BLOCK_SIZE);
		bwrite(n_block, &tmp_block);
		n_blocks_written++;
		n_bytes_written += BLOCK_SIZE;
	}
	
	/* Comprovam que queda un altre bloc per escriure. */
	if (how_many == 1) {
		block++;
		error = translate_inode(inode, block, &n_block, 1);
		if(error == -2) {
			puts("write_file: no queda espai lliure");
			return n_bytes_written;
		}
		bread(n_block, &tmp_block);
		bytes_copy = l_size;
		memcpy(tmp_block.valor, (char *)buffer + n_bytes_written, bytes_copy);
		bwrite(n_block, &tmp_block);
		n_blocks_written++;
		n_bytes_written += bytes_copy;
	}

	/* Calcul del tamany de l'inode un cop fetes les modificacions.*/
	read_inode(inode, &tmp_inode);
	grow_size = tmp_inode.size - offset -  n_bytes_written;
	if (grow_size < 0) {
		tmp_inode.size -= grow_size;
		write_inode(inode, &tmp_inode);
	}
	if (n_bytes_written < n_bytes) {
		puts("write_file: no s'han escrit tots els bytes");
	}
	return n_bytes_written;

}

/** Llegeix un cert nombre de bits d'un fitxer.
 * @inode: nombre d'inode on volem escriure
 * @buffer: Punter buffer de sortida aquesta funcio reservara l'espai.
 * @offset: determinam el primer byte del fitxer.
 * @n_bytes: nombre de bytes a llegir
 * @return: el nombre de bytes llegits.
 */
int read_file(int inode, void **buf, int offset, int n_bytes) {
	int j, error;
	int b_index = 0;
	int n_block = 0;
	/* Quants de blocs hem de llegir, nombre del primer bloc, desplacament
	sobre el primer bloc */
	int how_many;
	int block = 0;
	int f_offset = 0;
	int l_size = 0;
	int n_bytes_readen = 0;
	emofs_block tmp_block;

	how_many = calc_blocks_with_offset(offset, n_bytes, \
					   &block, &f_offset, &l_size);
	n_bytes_readen = 0;
	if (how_many < 1) {
		puts("read_file: error al calcul de la posicio inicial");
		return n_bytes_readen;
	}


	error = translate_inode(inode, block, &n_block, 0);

	if (error < 0) {
		return 0;
	}
	bread(n_block, &tmp_block);
	if (n_bytes + f_offset < BLOCK_SIZE) {
		b_index = n_bytes;
		n_bytes_readen = n_bytes;
	} else {
		b_index = BLOCK_SIZE - f_offset;
		n_bytes_readen = b_index;
	}
	/* l'unic coment en que n_bytes_readen no es major a b_index */
	*buf = malloc(n_bytes_readen*sizeof(char));
	memcpy(*buf, (tmp_block.valor + f_offset), n_bytes_readen);
	how_many--;

	if (how_many == 0) {
		/* Sols havia un bloc per llegir. */
		return n_bytes_readen;
	}

	/* Recorda que la variable «block» conte el nombre del primer bloc de
	   dades. Anam a llegir blocs sencers, per aixo descartam el darrer.*/
	for (/*res*/; (how_many -1) > 0 ; how_many--) {
		block++;
		translate_inode(inode, block, &n_block, 0);
		bread(n_block, &tmp_block);
		n_bytes_readen += BLOCK_SIZE;
		*buf = realloc(*buf, n_bytes_readen*sizeof(char));
		memcpy((char *) *buf + b_index, tmp_block.valor, BLOCK_SIZE);
		b_index += BLOCK_SIZE;
	}
	/* Ara sols queda el darrer bloc. */
	block++;

	translate_inode(inode, block, &n_block, 0);
	bread(n_block, &tmp_block);
	n_bytes_readen += ((l_size) ? l_size : BLOCK_SIZE); /* Hem llegit el darrer trocet de bloc */
	*buf = realloc(*buf, n_bytes_readen*sizeof(char));
	memcpy((char *)*buf + b_index, tmp_block.valor, l_size);

	if (n_bytes_readen != n_bytes) {
		puts("read_file: error amb el nombre de bytes llegits");
	}
	return n_bytes_readen;	
}

void truncate_assist(int block, int level, \
		     emofs_inode *inode, int *n_truncated_blocks);

/** Trunca un fitxer a partir del byte n.
 * Si n_bytes = 0 alliberam tots els blocs.
 * @inode: el nombre d'inode.
 * @n_byte: el byte final del fitxer.
 * @return: 0 si exit.
 */
int truncate_file(int inode, int n_byte) {
	int how_many, block, f_offset, l_size;
	int n_block, n_bytes, n_inode;
	int i;
	emofs_inode tmp_inode;
	int blocks_to_truncate;

	read_inode(inode, &tmp_inode);

	if (tmp_inode.size <= n_byte) {
		/* Si fitxer es mes petit o del tamany a truncar
		 * no s'ha de fer res. */
		return 0;
	}

	blocks_to_truncate = (tmp_inode.size-n_byte)/BLOCK_SIZE;
	for(i = INDIRECT_POINTER_COUNT-1; i >= 0; i--) {
		if(tmp_inode.indirect_pointer[i] != NULL_POINTER) {
			truncate_assist(tmp_inode.indirect_pointer[i], i,  \
					&tmp_inode, &blocks_to_truncate);
			if (blocks_to_truncate > 0) {
				free_block(tmp_inode.indirect_pointer[i]);
				tmp_inode.indirect_pointer[i] = NULL_POINTER;
				tmp_inode.block_count--;
				blocks_to_truncate--;
			}
		}
	}
	for(i = DIRECT_POINTER_COUNT-1; i >= 0; i--) {
		/* El blocks_to_truncate > 0 no esta a la condicio
		 * del bucle per donar suport als fitxers esparsos. */
		if (blocks_to_truncate > 0 && \
		    tmp_inode.direct_pointer[i] != NULL_POINTER) {
			free_block(tmp_inode.direct_pointer[i]);
			tmp_inode.direct_pointer[i] = NULL_POINTER;
			tmp_inode.block_count--;
			blocks_to_truncate--;
		}
	}
	
	tmp_inode.size = n_byte;
	write_inode(inode, &tmp_inode);
	return 0;
}

void truncate_assist(int block, int level, 
		     emofs_inode *inode, int *n_truncated_blocks) {
	int i;
	int tmp[INDIRECT_POINTERS_PER_BLOCK];
	if (level <= 0) {
		free_block(block);
		(*n_truncated_blocks)--;
	} else {
		bread(block, tmp);
		for(i = 0; i < INDIRECT_POINTERS_PER_BLOCK; i++) {
			if(tmp[i] != NULL_POINTER) {
				truncate_assist(tmp[i], level-1, \
						inode, n_truncated_blocks);
				tmp[i] = NULL_POINTER;
				inode->block_count--;
			}
		}
		bwrite(block, tmp);
	}
}

/** Obte la informacio d'un inode. 
 * @inode: nombre del fitxer.
 * @stat: el punter de sortida de les dades.
 * @return: 0 si exit.
 */
int stat_file(int inode, emofs_inode_stat *stat) {
	emofs_inode tmp;
	int error;
	
	error = read_inode(inode, &tmp);
	if (error > -1) {
		memcpy(stat, &tmp, sizeof(emofs_inode_stat));
	}
	return error;
}

/** Canvi el la data de modificacio de l'inode.
 * @inode: nombre del fitxer
 * @date: data epoch a posar
 * @return: 0 si exit
 */
int timestamp_file(int inode, time_t date) {
	emofs_inode i;
	read_inode(inode, &i);
	write_inode_time(inode, &i, date);
	return 0;
}
