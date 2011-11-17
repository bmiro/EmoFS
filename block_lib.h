/** Biblioteca de gestió de blocs. Encapsula tasques com crear un mapa de bits,
 * el vector d'inodes o reservar un inode. */

#include "common.h"

#include "super.h"

/* Decisions de disseny.
 * - El primer bit del mapa de bits correspon al primer bloc, el del superbloc.
 * - L'ordre sempre és sb - bm - ai
 * - El valor "null" pels inodes correspon al zero.
 */

/** Determina el nombre de blocs necessaris per situar el mapa de bits.
 * @n_block: nombre de blocs (de dades)
 * @return: nombre de blocs pel mapa de bits
 */
int bitmap_size(int n_block);

/** Determina el nombre de blocs necessaris per contenir el vector d'inodes.
 * @n_inode: nombre d'inodes que desitjam
 * @return: nombre de blocs lògics necessaris
 */
int inode_array_size(int n_inode);

/** Inicialitza el superblock.
 * D'aquí es farà el procés i calculs necessaris usant bitmap_size, 
 * inode_array_size
 * @n_block: nombre total de blocs del sistema de fitxers
 */
int init_superblock(int n_block);

/** Inicialitza el mapa de bits.
 * Llegeix informacio del superblock, per tant ha d'estar inicialitzat.
 * @return: 0 si èxit.
 */
int init_bitmap();

/** Inicialitza l'array d'inodes.
 * Ha de fer la crida per 
 * @first_block: la posició lògica del primer inode
 * @n_inode: nombre d'inodes a crear pel sistema de fitxers
 */
int init_inode_array();

/** Reserva un bloc. S'encarrega de busca-lo, reservar-lo, deixar-lo marcat al
 * mapa de bits i tornar-lo per poder treballar sobre ell. Modifica el nombre
 * total de blocs del superbloc. Torna -1 si hi no havia blocs lliures.
 * @type: Tipus de inode (directori o fitxer)
 * @return: el número del bloc reservat
 */
int alloc_block();

/** Allibera un bloc. S'encarrega de tornar a posar a zero el seu bit del mapa
 * de bits.  Torna -1 si el bloc no estava ocupat. Modifica el nombre total de
 * blocs del superbloc.
 * @n_block: número del bloc que volem alliberar.
 * @return: error d'execució.
 */
int free_block(int n_block);

/** Reserva un inode. Modifica el nombre d'inodes lliures i totals del
 * superbloc.  Cal inicialitzar totes les dades de l'inode: tipus, tamany,
 * mtime, recomptes i posar a zero els punters directes i indirectes. Modificar
 * la llista enllaçada d'inodes lliures.
 * @return: número d'inode, -1 si no en queden o es produeix un error.
 */
int alloc_inode(int type);

/** Allibera un inode. Fa el recorregut de tots els inodes indirectes. Allibera
 * també tots els blocs de dades.  Modifica el nombre d'inodes lliures i totals
 * del superbloc. Després afegeix l'inode actual a la llista d'inodes lliures.
 * @n_inode: número de l'inode a lliberar.
 * @return: -1 si el número d'inode no era correcte.
 */
int free_inode(int n_inode);
