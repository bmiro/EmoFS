/** Modul del superbloc. Al superbloc desam informacio com quins tamanys tenen
 * el mapa de bits, els inodes i les dades; quin es el primer inode lliure o
 * quants de blocs de dades queden. */

#include "common.h"

/* Importantissim contar be el nombre d'items. */
#define SUPERBLOCK_ITEMS 13
#define SUPERBLOCK_SIZE (SUPERBLOCK_ITEMS*4)
#define PADDING_BYTES (BLOCK_SIZE-SUPERBLOCK_SIZE)

/* Como minimo, para SUPERBLOQUE:
 *    Numero del primer bloque del mapa de bits
 *    Numero del ultimo bloque del mapa de bits
 *    Numero del primer bloque del array de inodos
 *    Numero del ultimo bloque del array de inodos
 *    Numero del primer bloque de datos
 *    Numero del ultimo bloque de datos
 *    Numero del inodo del directorio raiz
 *    Numero del primer inodo libre
 *    Cantidad de bloques libres
 *    Cantidad de inodos libres
 *    Cantidad total de bloques
 *    Cantidad total de inodos */
typedef struct {
	__u32 first_bitm_block;
	__u32 last_bitm_block;
	__u32 first_inode_block;
	__u32 last_inode_block;
	__u32 first_data_block;
	__u32 last_data_block;
	__u32 root_inode;
	__u32 first_free_inode;
	__u32 free_block_count;
	__u32 free_inode_count;
	__u32 total_block_count;
	__u32 total_inode_count;
	__u32 total_data_block_count;
	unsigned char padding[PADDING_BYTES];
} emofs_superblock;

/** Escriu al superblock
 * @sb: Contingut del superblock
 * @return: -1 per error.
 */
int sbread(emofs_superblock *sb);

/** Escriu el superblock.
 * @sb: Contingut que ha de tenir el superblock
 * @return: -1 per error
 */
int sbwrite(emofs_superblock *sb);

/** Imprimeix el contigut del superbloc.
 * @sb: el superblock.
 */
void print_sb(emofs_superblock *sb);
