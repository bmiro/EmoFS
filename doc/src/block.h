/** Modul de gestio de blocs. Els blocs son la unitat minima del sistema de
 * fitxers i consisteixen en un trocet de memoria la qual pot usar-se com
 * inode, mapa de bits, bloc de dades o superbloc. */

#include "common.h"

/* Estructura de dades que representa un bloc de BLOCK_SIZE*bytes. */
typedef struct {
	unsigned char valor[BLOCK_SIZE];
} emofs_block;

/** Genera un bloc ple de zeros.
 * @return: estructura de bloc de zeros
 */
emofs_block block_of_zero();

/** Genera un bloc ple d'uns.
 * @return: estructura de bloc d'uns
 */
emofs_block block_of_one();

/** Munta una imatge de sistema de fitxers.
 * Crea el fitxer si no existeix.
 * @return: el descriptor de fitxer
 */
int bmount(void);

/** Desmunta la imatge en us.
 * No te parametres perque el fitxer en us es desa a una variable
 * estatica interna.
 * @return: el codi d'error del close
 */
int bumount(void);

/** Escriu el buffer al bloc assenyalat.
 * @bloque: nombre de bloc a on escriure
 * @buf: buffer d'entrada
 * @return: sempre zero
 */
int bwrite(int bloque, const void *buf);


/** Llegeix el bloc assenyalat i fica el contingut dins el buffer.
 * @bloque: nombre de bloc d'on llegir
 * @buf: buffer de sortida
 * @return: sempre zero
 */
int bread(int bloque, void *buf);

