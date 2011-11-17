/* Mòdul dels directoris. Els directoris enmagatzemen la informació relativa a
 * l'ordenació i accés dels fitxers. Són la capa més externa del sistema de
 * fitxers i per això tenen la nomenclatura «emofs_x». */

#include <time.h>

#include "common.h"
#include "file.h"

typedef struct {
	char name[MAX_FILENAME_LEN];
	__u32 inode; /* NULL_POINTER si es una entrada buida */
} emofs_dir_entry;

/** D'una cadena representant una ruta de fitxer obté el camí entre
 * directoris.
 * @path: la ruta del fitxer.
 * @first: cadena que conté la primera part de la ruta.
 * @last: cadena que conté la resta de la ruta.
 * @return: 0 si troba
 *         -1 si s'obté un error.
 */
int emofs_extract_path(const char *path, char *head, char *tail);

/** Torna el directori on s'ha de crear el fitxer/carpeta i el nom
 * donant el path sencer: Pe: /dir1/dir2/fx torna: /dir1/dir2 i fx.
 * @path: path sencer a tallar
 * @partial_path: torna la primera part del path d'alla on penjarà el 
 *                fitxer/directori
 * @new_dir_file: Nom del fitxer/directori a crear
 * @return: 0 si es un fitxer
 *	    1 si es directori
 *	    -1 en cas d'error
 */
int emofs_get_partial_path(const char *path, \
			   char *partial_path, char *new_dir_file);

/** Des de l'inode del directori i el camí parcial obté l'inode del directori i
 * de l'entrada. 
 * Recorda que a la primera crida recursiva sempre li haurem d'indicar
 * p_inode_dir l'inode del directori arrel.
 * Convé usar extract_path per obtenir els subcamins.
 * @path: ruta, parcial o completa, del fitxer que buscam.
 * @p_inode_dir: l'inoide del directori.
 * @p_inode: l'inode del fitxer.
 * @p_entry: entrada del fitxer dins el directori.
 * @return: 0 si èxit.
*/
int emofs_find_entry(const char *path, int *p_inode_dir,
		     int *p_inode, int *p_entry);

/** Comprova si un fitxer existeix.
 * @path: camí a mirar
 * @return 0 si existeix, -1 si no
 */
int emofs_file_exists(const char *path);

/** Crea un fitxer o directori i la seva entrada de directori. Té control de
* concurrència.  Princpipalment empra les funcions find_entry, alloc_inode,
* write_file.
* @path: ruta del fitxer a crear
* @return: 0 si correctament.
	   -1 en cas d'error.
*/
int emofs_create(const char *path);

/** Crea l'enllaç de una entrada de directori del src_path a l'inode espeificic
 * per la altre entrada de directori link_path. Actualitza la quatitat
 * d'enllaços d'entrades en el directori de l'inode. Té control de
 * concurrència.
 * @src_path: ruta del fitxer a enllaçar
 * @link_path: ruta de l'enllaç
 * @return: 0 si correctament.
 *	    -1 en cas d'error.
 */
int emofs_link(const char *src_path, const char *link_path);

/** Borra l'entrada de directori especificada y en cas de que sigui l'últim. Té
 * control de concurrència.
 * enllaç existent borra el propi fitxer/directori.
 * @path: ruta del fitxer/directori
 * @return:  0 si correctament
 *	     -1 en cas d'error.
 */
int emofs_unlink(const char *path);

/** Determina si el camí és d'un directori o fitxer.
 * @path: ruta del fitxer
 * @return: 1 si és fitxer
 *          0 si és directori
 *	    -1 si no existex
 */
int emofs_is_file(const char *path);

/** Canvi el la data de modificació de l'inode.
 * @path: camí del fitxer
 * @date: data epoch a posar
 * @return: 0 si èxit
 */
int emofs_update_time(const char *path, time_t date);

/** Posa el contingut del directori en un buffer de memòria.
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

/** Obté la informació d'un fitxer. 
 * @path: Cami al fitxer.
 * @stat: el punter de sortida de les dades.
 * @return: 0 si èxit.
*/
int emofs_stat(const char *path, emofs_inode_stat *stat);

/** Llegeix un cert nombre de bits d'un fitxer.
 * @path: Camí al destí
 * @buf: punter al buffer de sortida, la funcio s'encarrega de reservar l'espai.
 * @offset: determinam el primer byte del fitxer.
 * @n_bytes: nombre de bytes a llegir
 * @return: el nombre de bytes llegits.
*/
int emofs_read(const char *path, void **buf, int offset, int n_bytes);

/** Escriu un cert nombre de bits d'un fitxer. Té control de concurrència.
 * @path: Camí al destí
 * @buf: buffer de sortida.
 * @offset: determinam el primer byte del fitxer.
 * @n_bytes: nombre de bytes a escriure
 * @return: el nombre de bytes escriure.
*/
int emofs_write(const char *path, const void *buf, int offset, int n_bytes);
