#include <stdio.h>
#include "super.h"
#include "bitmap.h"

/** Genera un mapa de zeros.
 * @return: estructura de mapa de bits inicialitzat a zero.
 */
emofs_bitmap map_of_zero() {
	int i = 0;
	emofs_bitmap mapa;
	for(i = 0; i < BLOCK_SIZE; i++) {
		mapa.valor[i] = 0;
	}
	return mapa;
}

/** Capgira tots els bits.
 * Substitució al mapa en paràmetre.
 * @buf: el mapa de bits
 */
void mflip(emofs_bitmap *buf) {
	int i = 0;
	for(i = 0; i < BLOCK_SIZE; i++) {
		buf->valor[i] = ~buf->valor[i];
	}
}

/* Determina el primer bloc lliure d'un mapa de bits de sistema de fitxers.
 * return: el nombre del bloc lliure. -1 si no s'en troben.
 */
int find_first_free_block(void) {
	int i, found, max;
	emofs_superblock sb;
	sbread(&sb);

	if(sb.free_block_count == 0) {
		printf("find_first_free_block: no hi ha blocs lliures\n");
		return -2;
	}

	max = MAP_SIZE * (sb.last_bitm_block - sb.first_bitm_block +1);
	found = 0;
	i = 0;
	while((i < max) && !found) {
		if(bm_read(i) == 0) {
			found = 1;	
		} else {
			i++;
		}
	}

	if (!found) {
		printf("find_first_free_block: no s'ha trobat bloc\n");
		return -1;
	}

	return i;	
}

/** Imprimeix el mapa.
 * @map: el mapa de bits
 */
void print_map(emofs_bitmap *map) {
	int i = 0;
	for(i = 0; i < BLOCK_SIZE; i++) {
		printf("%d", map->valor[i]);
	}
	printf("\n");
}

/** Llegeix un bit. Cal recordar que sols pot ser un 1 o 0, per això es torna
 * simplement amb un char.
 * Si és -1 és perquè hem intentat llegir un bit fora de rang.
 * @bit: el nombre de bit a llegir
 * @buf: el mapa de bits
 * @return: el valor del bit. -1 si hi ha error de rang.
 */
char mread(int bit, emofs_bitmap *buf) {
	unsigned char val = 0;
	int grup = 0;
	int posicio = 0;
	if ((bit >= MAP_SIZE) || (bit < 0)) {
		return -1;
	}
	grup = bit / 8;
	posicio = bit % 8;

	val = buf->valor[grup];
	val &= (1 << posicio);
	val >>= posicio;
	return val;
}

/** Escriu un bit. Cal recordar que sols pot ser un 1 o 0, per això es torna
 * simplement amb un char.
 * @bit: el nombre de bit a llegir
 * @val: el valor a desar.
 * @buf: el mapa de bits
 * @return: 0 si l'operació funciona correctament.
 */
char mwrite(int bit, unsigned char val, emofs_bitmap *buf) {
	unsigned char tmp = 0;
	unsigned int grup = 0;
	unsigned int posicio = 0;

	grup = bit / 8;
	posicio = bit % 8;
	tmp = ~(1 << posicio); 
	val &= 1;
	val <<= posicio;

	buf->valor[grup] &= tmp;
	buf->valor[grup] |= val;
	return 0;
}

/** Llegeix l'estat d'un bloc de dades representat al mapa de bits.
 * @int block: nombre de block que buscam
 * @return: 1 si està ocupat.
 *	    0 si està lliure.
 *	    -1 en cas d'error (fora de rang)
 */
char bm_read(int block) {
	emofs_bitmap bm;
	emofs_superblock sb;
	int n_bm, group, pos;

	sbread(&sb);

	/* Comprovam que no sigui fora de rang */
	if (block < 0 || block > sb.total_data_block_count) {
		puts("bm_read: accés fora de rang");
		return -1;
	}
	group = block / MAP_SIZE;
	pos = block % MAP_SIZE;
	bread(sb.first_bitm_block + group, &bm);
	return mread(pos, &bm);
}

/** Escriu l'estat d'un bloc de dades representat al mapa de bits.
 * @int block: nombre de block que buscam
 * @return: error.
 */
int bm_write(int block, unsigned char bit) {
	int n_block, pos;
	emofs_superblock sb;	
	emofs_bitmap bm;

	sbread(&sb);

	/* Comprovam que no sigui fora de rang */
	if (block < 0 || block > sb.total_data_block_count) {
		puts("bm_write: accés fora de rang");
		return -1;
	}
	/* Calculam en quin block del mapa de bits es troba */
	n_block = (block / MAP_SIZE) + sb.first_bitm_block;
	pos = block % MAP_SIZE;
	bread(n_block, &bm);
	mwrite(pos, bit, &bm);
	bwrite(n_block, &bm);
	return 0;
}
