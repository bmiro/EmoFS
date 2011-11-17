/** Modul de creacio i gestio dels inodes. Els inodes son part de les metadades
 * del sistema de fitxers i contenen informacio relativa a cada fitxer o
 * directori. Indiquen la data de modificacio, el seu tamany o la quantitat
 * d'enllacos que tenen. */

#include "common.h"

#define INODES_PER_BLOCK (BLOCK_SIZE/sizeof(emofs_inode))
#define INDIRECT_POINTERS_PER_BLOCK (BLOCK_SIZE/sizeof(int))
#define LEVEL_1_POINTERS (INDIRECT_POINTERS_PER_BLOCK)
#define LEVEL_2_POINTERS (LEVEL_1_POINTERS*LEVEL_1_POINTERS)
#define LEVEL_3_POINTERS (LEVEL_2_POINTERS*LEVEL_1_POINTERS)
#define OFFSET_L0_POINTERS (0)
#define OFFSET_L1_POINTERS (OFFSET_L0_POINTERS + DIRECT_POINTER_COUNT)
#define OFFSET_L2_POINTERS (OFFSET_L1_POINTERS + LEVEL_1_POINTERS)
#define OFFSET_L3_POINTERS (OFFSET_L2_POINTERS + LEVEL_2_POINTERS)

/* Tamany objectiu de l'inode. En bytes. */
#define INODE_TARGET_SIZE 64

/* Dades descriptores de l'inode. */
#define INODE_ITEMS 5
#define INDIRECT_POINTER_COUNT 3

/* Nombre de chars ocupats fins al moment */
#define INODE_MUST_SIZE ((INODE_ITEMS+INDIRECT_POINTER_COUNT)*4)

/* 8 per aque ens quadrin els blocks de 64bytes */
#define DIRECT_POINTER_COUNT ((INODE_TARGET_SIZE-INODE_MUST_SIZE)/4)

/* Como minimo, para INODO:
 *    Tipo (libre, directorio o fichero)
 *    Tamano en bytes logicos
 *    Fecha de modificacion
 *    Cantidad de bloques fisicos asignados
 *    Cantidad de enlaces fisicos
 *    Varios punteros a bloques directos (segun el tamano que queramos que tenga
 *        el inodo: se recomienda un tamano de 64 bytes)
 *    3 punteros a bloques indirectos */
typedef struct {
	/* 0 lliure, 1 directori, 2 fitxer, 3 buit */
	__u32 type;
	/* Si es lliure size indica la posicio del següent bloc lliure.
	 * -1 indica final de llista.*/
	__u32 size;
	__u32 mtime;
	__u32 block_count;
	__u32 link_count;
	__u32 direct_pointer[DIRECT_POINTER_COUNT];
	/* 0 es el de nivell 1, 1 el de nivell 2 i 2 el de nivell 3 */
	__u32 indirect_pointer[INDIRECT_POINTER_COUNT];
} emofs_inode;

/* Localitzacio d'una dada adins un inode. 
 * n -> nivell de punter indirecte
 * pex: 3 -> [0, [3, 0, 0]]
 * pex: 10 -> [1, [2, 0, 0]]
 */
typedef struct {
	int n; /* de 0 a 3 */
	int l[3];
} emofs_data_loc;

/** Situa dins l'estructura de localitzacio un numero de bloc de dades.
 * @n_block: nombre sobre el qual fer el calcul.
 * @loc: l'estructura de posicionament.
 * @return: 0 si exit.
 */
int localize_data(int n_block, emofs_data_loc *loc);

/** Escriu un inode.
 * @n_inode: el nombre de l'inode a escriure.
 * @buf: l'inode d'entrada.
 * @return: -1 si l'inode objectiu no existeix.
 */
int write_inode(int n_inode, emofs_inode *buf);

/** Escriu un inode.
 * @n_inode: el nombre de l'inode a escriure.
 * @buf: l'inode d'entrada.
 * @date: hora de modificacio de l'inode.
 * @return: -1 si l'inode objectiu no existeix.
 */
int write_inode_time(int n_inode, emofs_inode *buf, time_t date);

/** Llegeix un inode.
 * @n_inode: el nombre de l'inode a llegir.
 * @buf: buffer on es desa el contingut de l'inode.
 * @return: -1 si el nombre d'inode no existia.
 */
int read_inode(int n_inode, emofs_inode *buf);

/** Tradueix el bloc logic al seu valor de bloc fisic.
 * Cal recordar que si afegim un bloc de dades hem de modificar el contador
 * d'inodes i corregir el tamany de fitxer.
 * @n_inode: nombre d'inode
 * @l_block: numero de bloc logic.
 * @alloc: si 1 i no existeix el bloc fisic reserva un bloc de dades.
 * @return: 0 si exit. -1 si es volia llegir i no existia.
 */
int translate_inode(int n_inode, int l_block, int *block, int alloc);
