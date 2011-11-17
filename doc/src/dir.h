/* Modul dels directoris. Els directoris enmagatzemen la informacio relativa a
 * l'ordenacio i acces dels fitxers. Son la capa mes externa del sistema de
 * fitxers i per aixo tenen la nomenclatura «emofs_x». */

#include <time.h>

#include "common.h"
#include "file.h"

typedef struct {
	char name[MAX_FILENAME_LEN];
	__u32 inode; /* NULL_POINTER si es una entrada buida */
} emofs_dir_entry;

/** D'una cadena representant una ruta de fitxer obte el cami entre
 * directoris.
 * @path: la ruta del fitxer.
 * @first: cadena que conte la primera part de la ruta.
 * @last: cadena que conte la resta de la ruta.
 * @return: 0 si troba
 *         -1 si s'obte un error.
 */
int emofs_extract_path(const char *path, char *head, char *tail);

/** Torna el directori on s'ha de crear el fitxer/carpeta i el nom
 * donant el path sencer: Pe: /dir1/dir2/fx torna: /dir1/dir2 i fx.
 * @path: path sencer a tallar
 * @partial_path: torna la primera part del path d'alla on penjara el 
 *                fitxer/directori
 * @new_dir_file: Nom del fitxer/directori a crear
 * @return: 0 si es un fitxer
 *	    1 si es directori
 *	    -1 en cas d'error
 */
int emofs_get_partial_path(const char *path, \
			   char *partial_path, char *new_dir_file);

/** Des de l'inode del directori i el cami parcial obte l'inode del directori i
 * de l'entrada. 
 * Recorda que a la primera crida recursiva sempre li haurem d'indicar
 * p_inode_dir l'inode del directori arrel.
 * Conve usar extract_path per obtenir els subcamins.
 * @path: ruta, parcial o completa, del fitxer que buscam.
 * @p_inode_dir: l'inoide del directori.
 * @p_inode: l'inode del fitxer.
 * @p_entry: entrada del fitxer dins el directori.
 * @return: 0 si exit.
*/
int emofs_find_entry(const char *path, int *p_inode_dir,
		     int *p_inode, int *p_entry);

/** Comprova si un fitxer existeix.
 * @path: cami a mirar
 * @return 0 si existeix, -1 si no
 */
int emofs_file_exists(const char *path);

/** Crea un fitxer o directori i la seva entrada de directori. Te control de
* concurrencia.  Princpipalment empra les funcions find_entry, alloc_inode,
* write_file.
* @path: ruta del fitxer a crear
* @return: 0 si correctament.
	   -1 en cas d'error.
*/
int emofs_create(const char *path);

/** Crea l'enllac de una entrada de directori del src_path a l'inode espeificic
 * per la altre entrada de directori link_path. Actualitza la quatitat
 * d'enllacos d'entrades en el directori de l'inode. Te control de
 * concurrencia.
 * @src_path: ruta del fitxer a enllacar
 * @link_path: ruta de l'enllac
 * @return: 0 si correctament.
 *	    -1 en cas d'error.
 */
int emofs_link(const char *src_path, const char *link_path);

/** Borra l'entrada de directori especificada y en cas de que sigui l'ultim. Te
 * control de concurrencia.
 * enllac existent borra el propi fitxer/directori.
 * @path: ruta del fitxer/directori
 * @return:  0 si correctament
 *	     -1 en cas d'error.
 */
int emofs_unlink(const char *path);

/** Determina si el cami es d'un directori o fitxer.
 * @path: ruta del fitxer
 * @return: 1 si es fitxer
 *          0 si es directori
 *	    -1 si no existex
 */
int emofs_is_file(const char *path);

/** Canvi el la data de modificacio de l'inode.
 * @path: cami del fitxer
 * @date: data epoch a posar
 * @return: 0 si exit
 */
int emofs_update_time(const char *path, time_t date);

/** Posa el contingut del directori en un buffer de memoria.
 * el nom de cada entrava ve separat per ':'.
 * Es ell qui reserva la memoria de buf i la funcio que el
 * cridi qui l'ha d'alliberar en haver-ho emprat.
 * @path: Cami al directori
 * @buf: llistat de les entrades de directori separades per ':'
 * @return: numero de d'entrades llistades.
*/
int emofs_dir(const char *path, char **buf);

/** De la sortida de emofs_dir (contingut de directori separat per :)
 * lleva una entrada, i la copia dins filename.
 * @dir_content: llista d'entrades separades per : . En borra la primera
 * @filename: hi deixa el nom de l'entrada de directori
 * @return 0 si lleva entrada (exit)
 *          -1 en cas d'error
 */
int extract_dir_entry(char *dir_content, char *filename);

/** Obte la informacio d'un fitxer. 
 * @path: Cami al fitxer.
 * @stat: el punter de sortida de les dades.
 * @return: 0 si exit.
*/
int emofs_stat(const char *path, emofs_inode_stat *stat);

/** Llegeix un cert nombre de bits d'un fitxer.
 * @path: Cami al desti
 * @buf: punter al buffer de sortida, la funcio s'encarrega de reservar l'espai.
 * @offset: determinam el primer byte del fitxer.
 * @n_bytes: nombre de bytes a llegir
 * @return: el nombre de bytes llegits.
*/
int emofs_read(const char *path, void **buf, int offset, int n_bytes);

/** Escriu un cert nombre de bits d'un fitxer. Te control de concurrencia.
 * @path: Cami al desti
 * @buf: buffer de sortida.
 * @offset: determinam el primer byte del fitxer.
 * @n_bytes: nombre de bytes a escriure
 * @return: el nombre de bytes escriure.
*/
int emofs_write(const char *path, const void *buf, int offset, int n_bytes);
