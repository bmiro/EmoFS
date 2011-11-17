/* Modul dels fitxers. Els fitxers son una abstraccio per treballar sobre els
 * inodes. Alhora permeten manipular les dades de l'usuari amb un cami de
 * fitxer (/home/usuari/.signature) enlloc de amb el nombre d'inode. */

#include "common.h"

#include <time.h>

/* L'estructura emofs_stat es exacta a la de l'inode excepte que no desa punters.
 * Consultar inode.h per mes informacio.
 */
typedef struct {
	__u32 type;
	__u32 size;
	__u32 mtime;
	__u32 block_count;
	__u32 link_count;
} emofs_inode_stat;

/** Escriu un cert nombre de bytes d'un fitxer.  Cal recordar que si indicam un
 * offset = 10 significa que hem de comencar. Es important actualitzar el
 * tamany del fitxer a escriure des del byte numero 10.
 * @inode: nombre d'inode on volem escriure
 * @buffer: buffer d'entrada. D'alla copiarem les dades.
 * @offset: determinam el primer byte del fitxer.
 * @n_bytes: nombre de bytes a escriure
 * @return: el nombre de bytes escrits.
 */
int write_file(int inode, const void *buffer, int offset, int n_bytes);

/** Llegeix un cert nombre de bytes d'un fitxer.
 * Cal recordar que si indicam un offset = 10 significa que hem de comencar.
 * Es important actualitzar el tamany del fitxer.
 * a escriure des del byte numero 10.
 * @inode: nombre d'inode on volem escriure
 * @buffer: Punter buffer de sortida, aquesta funcio reservara l'espai
 * @offset: determinam el primer byte del fitxer.
 * @n_bytes: nombre de bytes a escriure
 * @return: el nombre de bytes escrits.
 */
int read_file(int inode, void **buf, int offset, int n_bytes);

/** Trunca un fitxer a partir del byte n.
 * Si n_bytes = 0 alliberam tots els blocs.
 * @inode: el nombre d'inode.
 * @n_byte: el byte final del fitxer.
 * @return: 0 si exit.
 */
int truncate_file(int inode, int n_byte);

/** Obte la informacio d'un inode. 
 * @inode: nombre del fitxer.
 * @stat: el punter de sortida de les dades.
 * @return: 0 si exit.
 */
int stat_file(int inode, emofs_inode_stat *stat);

/** Canvi el la data de modificacio de l'inode.
 * @inode: nombre del fitxer
 * @date: data epoch a posar
 * @return: 0 si exit
 */
int timestamp_file(int inode, time_t date);
