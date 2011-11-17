/** Modul del mapa de bits. Aquesta part de les metadades s'encarrega de
 * permetre un acces rapid a la localitzacio dels blocs de dades lliures. */

#include "common.h"

#define MAP_SIZE (BLOCK_SIZE*8)

typedef struct {
	unsigned char valor[BLOCK_SIZE];
} emofs_bitmap;

/** Genera un mapa de zeros.
 * @return: estructura de mapa de bits inicialitzat a zero.
 */
emofs_bitmap map_of_zero();

/** Capgira tots els bits.
 * Substitucio al mapa en parametre.
 * @buf: el mapa de bits
 */
void mflip(emofs_bitmap *buf);

/* Determina el primer bloc lliure d'un mapa de bits de sistema de fitxers.
 * return: el nombre del bloc lliure. -1 si no s'en troben.
 */
int find_first_free_block(void);

/** Imprimeix el mapa.
 * @map: el mapa de bits
 */
void print_map(emofs_bitmap *map);

/** Llegeix un bit. Cal recordar que sols pot ser un 1 o 0, per aixo es torna
 * simplement amb un char.  Si es -1 es perque hem intentat llegir un bit fora
 * de rang.
 * @bit: el nombre de bit a llegir
 * @buf: el mapa de bits
 * @return: el valor del bit. -1 si hi ha error de rang.
 */
char mread(int bit, emofs_bitmap *buf);

/** Escriu un bit. Cal recordar que sols pot ser un 1 o 0, per aixo es torna
 * simplement amb un char.
 * @bit: el nombre de bit a llegir
 * @val: el valor a desar.
 * @buf: el mapa de bits
 * @return: 0 si l'operacio funciona correctament.
 */
char mwrite(int bit, unsigned char val, emofs_bitmap *buf);

/** Llegeix l'estat d'un bloc de dades representat al mapa de bits.
 * @int block: nombre de block que buscam
 * @return: 1 si esta ocupat.
 */
char bm_read(int block);

/** Llegeix l'estat d'un bloc de dades representat al mapa de bits.
 * @int block: nombre de block que buscam
 * @return: 1 si esta ocupat.
 */
int bm_write(int block, unsigned char val);
