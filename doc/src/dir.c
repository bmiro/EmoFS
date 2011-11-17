#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dir.h"
#include "inode.h"
#include "super.h"
#include "sem.h"
#include "common.h"

int mutex = 0;

/** D'una cadena representant una ruta de fitxer obte el cami entre
 * directoris.
 * @path: la ruta del fitxer.
 * @first: cadena que conte la primera part de la ruta.
 * @last: cadena que conte la resta de la ruta.
 * @return: 0 si troba.
 *         -1 si s'obte un error.
 */
int emofs_extract_path(const char *path, char *head, char *tail) {
/* Exemples de l'enunciat
    * camino = "/dir1/dir2/fichero"
      inicial = "dir1" (devuelve DIRECTORIO)
      final = "/dir2/fichero"

    * camino = "/dir/"
      inicial = "dir" (devuelve DIRECTORIO)
      final = "/"

    * camino = "/fichero"
      inicial = "fichero" (devuelve FICHERO)
      final = ""
 */
	int delim_found = 0;
	int i, j, k;

	if (path[0] != '/') {
		puts("Error extract_path: No hi ha barra al principi");
		return -1;
	}

	/* S'ha dignorar la primera barra per aixo i = 1 */
	j = 0;
	k = 0;
	for (i = 1; path[i] != '\0'; i++) {
		if ((path[i] == '/') && !delim_found) {
	   		delim_found = 1;
	   	}
		if (!delim_found) {
			head[j] = path[i];
			j++;
		} else {
			tail[k] = path[i];
			k++;
		}
	}
	/* Posam els finals d'string */
	head[j] = '\0';
	tail[k] = '\0';
	return delim_found;
}

/** Torna el directori on s'ha de crear el fitxer/carpeta i el nom
 * donant el path sencer: Pe: /dir1/dir2/fx torna: /dir1/dir2 i fx.
 * @path: path sencer a tallar
 * @partial_path: torna la primera part del path d'alla on penjara el 
 *                fitxer/directori
 * @new_dir_file: Nom del fitxer/directori a crear
 * @return: 2 si es un fitxer
 *	    1 si es directori
 *	    -1 en cas d'error
 */
int emofs_get_partial_path(const char *path, 
			   char *partial_path, char *new_dir_file) {

	int i, j, k;
	int to_return = -1;
	
        i = strlen(path)-2;
        while (path[i] != '/') {
                i--;
        }
        k = 0;
        if (path[strlen(path)-1] == '/'){
                to_return = DIRECTORY_INODE; /* Es una directori */
                for (j = i+1; j < strlen(path)-1; j++){
                        new_dir_file[k] = path[j];
                        k++;
                }
        } else {
                to_return = FILE_INODE; /* Es un fitxer */
                for (j = i+1; j < strlen(path); j++){
                        new_dir_file[k] = path[j];
                        k++;
                }
        }
        new_dir_file[k]  = '\0';

        for (j = 0; j < i+1; j++){
                partial_path[j] = path[j];
        }
        partial_path[j] = '\0';
	return to_return;
}


/** Des de l'inode del directori i el cami parcial obte l'inode del directori
 * i de l'entrada. Recorda que a la primera crida recursiva sempre li haurem
 * d'indicar p_inode_dir l'inode del directori arrel.
 * Conve usar extract_path per obtenir els subcamins.
 * @path: ruta, parcial o completa, del fitxer que buscam.
 * @p_inode_dir: l'inode del directori.
 * @p_inode: l'inode del fitxer.
 * @p_entry: entrada del fitxer dins el directori.
 * @return: 0 si exit.
*/
int emofs_find_entry(const char *path, int *p_inode_dir,
		     int *p_inode, int *p_entry) {

	int found;
	int is_dir;
	char target [MAX_FILENAME_LEN];
	char tail [MAX_PATH_LEN];
	emofs_inode inode;
	emofs_dir_entry *dir_entry;
	int dir_entry_count;
	int i;
	emofs_superblock sb;

	/* Cas trivial del directori root */
	if(!strcmp(path, "/")) {
		sbread(&sb);
		*p_inode_dir = sb.root_inode;
		*p_inode = sb.root_inode;
		*p_entry = 0;
		return 0;
	}
	
	emofs_extract_path(path, target, tail);

	read_inode(*p_inode_dir, &inode);
	/* Cal recordar que no tenim desada enlloc la quantitat d'entrades que
	 * tenim dins el directori. Amb la divisio tamany_escrit/tamany_dades
	 * obtindrem quants d'elements tenim. */
	dir_entry_count = inode.size / sizeof(emofs_dir_entry);
	/* p_inode_dir -> inode del pare
	 * dir_entry -> l'entrada de directori: nom+inode
	 * dir_entry_count -> nombre de l'entrada dins el directori pare
	 */
	found = 0;
	for (i = 0; i < dir_entry_count; i++) {
		read_file(*p_inode_dir, (void *)&dir_entry, i*sizeof(emofs_dir_entry), \
			  sizeof(emofs_dir_entry));
		found = !strcmp(target, dir_entry->name);
		if (found) {
			break;
		}
	}
	if (!found) {
		return -1;
	}

	*p_entry = i; 
	*p_inode = dir_entry->inode;
	/* Si encara queda cami seguim amb les crides recursives */
	if (strcmp(tail, "")) {
		*p_inode_dir = dir_entry->inode;
		free(dir_entry);
		return emofs_find_entry(tail, p_inode_dir, p_inode, p_entry);
	}

	free(dir_entry);
	
	/* Si arribam aqui s'aturen les crides recursives. */
	return 0;
}

/** Crea un fitxer o directori i la seva entrada de directori. 
* Princpipalment empra les funcoons find_entry, alloc_inode, write_file
* @path: ruta del fitxer a crear
* @return: 0 si es crea correctament.
*	   -1 en cas d'error.
*/
int emofs_create(const char *path) {
	emofs_superblock sb;
	int i = 0;
	int p_inode_dir;
	int p_inode;
	int p_new_inode;
	int p_entry;
	emofs_inode inode;
	int type;
	char partial_path[MAX_PATH_LEN];
	char new_dir_file[MAX_FILENAME_LEN];
	int error = 0;
	int dir_entry_count;
	emofs_dir_entry dir_entry;
	
	if(!mutex){
		emofs_sem_get(&mutex);
	}
	emofs_sem_wait(mutex);

	sbread(&sb);
	if (emofs_file_exists(path)) {
		/* Existeix, per tant no el podem crear */
		puts("emofs_create: El fitxer o directori ja existeix");
		emofs_sem_signal(mutex);
		return -1;
	}
	
	type = emofs_get_partial_path(path, partial_path, new_dir_file);
	
	/* Comprovam que existeixi la ruta on crear el fitxer o directori */
	p_inode_dir = sb.root_inode;
	if (emofs_find_entry(partial_path, \
			     &p_inode_dir, &p_inode, &p_entry) == -1) {
		puts("emofs_create: El directori on es preten fer " \
		     "la creacio no existeix");
		printf("partial_path %s\n", partial_path);
		printf("p_inode_dir: %d, p_inode: %d, p_entry: %d\n", \
		       p_inode_dir, p_inode, p_entry);
		emofs_sem_signal(mutex);
		return error;
	}

	/* Obtenim el directori on volem crear l'entrada */
	p_inode_dir = sb.root_inode;
	emofs_find_entry(path, &p_inode_dir, &p_inode, &p_entry);

	dir_entry.inode = alloc_inode(type); /* Aqui reservam l'inode */
	if (dir_entry.inode == -1) {
		emofs_sem_signal(mutex);
		return -1;
	}
	for(i = 0; i < MAX_FILENAME_LEN; i++) {
		dir_entry.name[i] = ' ';
	}
	strcpy(dir_entry.name, new_dir_file);

	/* Llegim inode del directori del pare per saber el seu tamany i poder
	 * escriure al final. */
	read_inode(p_inode_dir, &inode);
	error = write_file(p_inode_dir, &dir_entry, inode.size, \
			   sizeof(emofs_dir_entry));
	if ( error < 0) {
		puts("emofs_create: Error d'escriptura");
		emofs_sem_signal(mutex);
		return error;
	}
	emofs_sem_signal(mutex);
	return error;
}

/** Comprova si un fitxer existeix.
 * @path: cami a mirar
 * @return 0 si existeix, -1 si no
 */
int emofs_file_exists(const char *path) {
	int zero = 0;
	int tmp1, tmp2;
	return !emofs_find_entry(path, &zero, &tmp1, &tmp2);
}

/** Crea l'enllac de una entrada de directori del src_path a l'inode espeificic
 * per la altre entrada de directori link_path. Actualitza la quatitat
 * d'enllacos d'entrades en el directori de l'inode. Te control de
 * concurrencia.
 * @src_path: ruta del fitxer a enllacar
 * @link_path: ruta de l'enllac
 * @return: 0 si correctament.
 *	    -1 en cas d'error.
 */
int emofs_link(const char *src_path, const char *link_path) {
	int p_dir_src_inode = 0;
	int p_src_inode = 0;
	int p_dir_dst_inode = 0;
	int p_dst_inode = 0;
	int p_entry = 0;

	emofs_inode inode;
	emofs_dir_entry dir_entry;
		
	char link_name[MAX_FILENAME_LEN];
	char partial_link_path[MAX_PATH_LEN];

	if (!mutex){
		emofs_sem_get(&mutex);
	}
	emofs_sem_wait(mutex);
	
	emofs_get_partial_path(link_path, partial_link_path, link_name);
	/* Comprovam i obtenim informacio del primer path */
	p_dir_src_inode = 0;
	if (emofs_find_entry(src_path, \
			     &p_dir_src_inode, &p_src_inode, &p_entry) == -1) {
		puts("EmoFS_Link: El fitxer o directori font no existeix");
		emofs_sem_signal(mutex);
		return -1;
	}
	
	/* Comprovam que no estiqui ja creat el fitxer */
	p_dir_dst_inode = 0;
	if (emofs_find_entry(link_path, \
			     &p_dir_dst_inode, &p_dst_inode, &p_entry) == 0) {
		puts("EmoFS_Link: El fitxer ja existeix");
		emofs_sem_signal(mutex);
		return -1;
	}
	
	/* Incrementam el numero d'enllacos de l'inode font */
	read_inode(p_src_inode, &inode);
	inode.link_count++;
	write_inode(p_src_inode, &inode);
	
	/* Cream l'entrada de directori de l'enllac */
	dir_entry.inode = p_src_inode;
	strcpy(dir_entry.name, link_name);
	
	/* Anam a escriure l'entrada de directori al direcotri desti */
	read_inode(p_dir_dst_inode, &inode); /* Per saber el tamany de fitxer */
	if (write_file(p_dir_dst_inode, &dir_entry, \
		       inode.size, sizeof(emofs_dir_entry)) == -1) {
		puts("EmoFS_Link: Error d'escriptura");
		emofs_sem_signal(mutex);
		return -1;
	}
	emofs_sem_signal(mutex);
	return 0;	
}

/** Borra l'entrada de directori especificada y en cas de que sigui l'ultim
 * enllac existent borra el propi fitxer/directori. Te control de concurrencia.
 * @path: ruta del fitxer/directori
 * @return:  0 si correctament
 *	     -1 en cas d'error.
 */
int emofs_unlink(const char *path) {
	int p_inode = 0;
	int p_dir_inode = 0;
	int p_entry = 0;

	emofs_inode inode;
	emofs_dir_entry *last_dir_entry;

	if(!mutex){
		emofs_sem_get(&mutex);
	}
	emofs_sem_wait(mutex);
	if (emofs_find_entry(path, &p_dir_inode, &p_inode, &p_entry) == -1) {
		puts("EmoFS_UnLink: El fitxer o directori font no existeix");
		emofs_sem_signal(mutex);
		return -1;
	}

	read_inode(p_dir_inode, &inode);
	/* Borram entrada de directori */
	if (inode.size > sizeof(emofs_dir_entry)) {
		/* Hi ha mes entrades posam l'ultima al lloc de la que volem 
		 * borrar. */
		read_file(p_dir_inode, (void *)&last_dir_entry, \
			  inode.size-sizeof(emofs_dir_entry), \
			  sizeof(emofs_dir_entry));
		write_file(p_dir_inode, last_dir_entry, \
			   p_entry*sizeof(emofs_dir_entry), \
			   sizeof(emofs_dir_entry));
	}

	free(last_dir_entry);

	/* Truncam per l'ultima entrada, si sols hi ha la que volem borrar be
	 * sino llevam l'ultima que ara estara copiada al lloc de la que voliem
	 * borrar. */
	truncate_file(p_dir_inode, inode.size-sizeof(emofs_dir_entry));
	read_inode(p_inode, &inode);
	if (inode.link_count == 0) {
		/* Hem de alliberar blocs de dades i inode */
		truncate_file(p_inode, 0);
		free_inode(p_inode);
	} else {
		inode.link_count--;
		write_inode(p_inode, &inode);
	}
	emofs_sem_signal(mutex);
	return 0;
}

/** Determina si el cami es d'un directori o fitxer.
 * @path: ruta del fitxer
 * @return: 0 si es directori
 *          1 si es fitxer
 */
int emofs_is_file(const char *path) {
	emofs_inode_stat info;

	emofs_stat(path, &info);
	return info.type == FILE_INODE;
	
}

/** Canvi el la data de modificacio de l'inode.
 * @path: cami del fitxer
 * @date: data epoch a posar
 * @return: 0 si exit
 */
int emofs_update_time(const char *path, time_t date) {
	int inode = 0;
	int inode_dir = 0;
	int entry = 0;
	emofs_find_entry(path, &inode_dir, &inode, &entry);
	timestamp_file(inode, date);
	return 0;
}

/** Posa el contingut del directori en un buffer de memoria.
 * el nom de cada entrava ve separat per ':'.
 * Es ell qui reserva la memoria de buf i l'ha d'alliberar
 * la funcio que l'empri en haver acabat.
 * @path: Cami al directori
 * @buf: llistat de les entrades de directori separades per ':'
 * @return: numero de d'entrades llistades, -1 error, -2 era un fitxer.
*/
int emofs_dir(const char *path, char **buf) {
	int p_inode;
	emofs_inode inode;
	emofs_superblock sb;
	int p_entry; /* No s'empra pero es necessari per cridar find_entry */
	int p_inode_dir;
	int dir_entry_count;
	emofs_dir_entry *dir_entry;
	int error;
	int i, j, k;
	int used_entry_count = 0;
	int buffer_size = 0;

	sbread(&sb);
	p_inode_dir = sb.root_inode;
	error = emofs_find_entry(path, &p_inode_dir, &p_inode, &p_entry);

	if (error < 0) {
		return error;
	}
	
	read_inode(p_inode, &inode);
	dir_entry_count = inode.size/sizeof(emofs_dir_entry);

	*buf = malloc(sizeof(char));

	k = 0;
	for (i = 0; i < dir_entry_count; i++) {
		read_file(p_inode, (void *) &dir_entry, \
			i*sizeof(emofs_dir_entry), sizeof(emofs_dir_entry));
		if (dir_entry->inode != NULL_POINTER) {
			buffer_size += strlen(dir_entry->name)+1; /*Mes les ':'*/
			/*Memoria dinamica, el que faltava per fer*/
			*buf = realloc(*buf, buffer_size*sizeof(char));
			if (*buf == NULL) {
				puts("Error redimensionant tamany de buffer.");
			}
			for(j = 0; dir_entry->name[j] != '\0'; j++) {
				(*buf)[k] = dir_entry->name[j];
				k++;
			}
			(*buf)[k] = ':'; /* separadors de nom de fitxer */
			k++;
			used_entry_count++;
		}
	}

	free(dir_entry);

	if (used_entry_count > 0) {
		/* Posam el final d'string substituint el separador */
		(*buf)[k-1] = '\0';
	} else {
		*buf = malloc(sizeof(char));
		(*buf)[k] = '\0'; /* k sera 0 */
	}

	return used_entry_count;
}

/** De la sortida de emofs_dir (contingut de directori separat per ':')
 * lleva una entrada i la copia dins filename.
 * @dir_content: llista d'entrades separades per ':'; En borra la primera
 * @filename: hi deixa el nom de l'entrada de directori
 * @return 0 si lleva entrada (exit)
 *          -1 en cas d'error
 */
int emofs_extract_dir_entry(char *dir_content, char *filename) {
	int i = 0;
	int j = 0;
	char tmp;
	
	/* Copiam la darrera entrada. El truc ve de que desarem el contingut de
	 * dir_content dins filename llegint al reves i despres el
	 * girarem. */
	/* abcde:fgh:ijk -> filename = kji */
	i = strlen(dir_content)-1;
	j = 0;
	for(; dir_content[i] != ':' && i >= 0; i--) {
		if (j >= MAX_FILENAME_LEN) {
			puts("emofs_extract_dir_entry: nom massa llarg");
			return -1;
		}
		filename[j] = dir_content[i];
		dir_content[i] = '\0';
		j++;
	}
	/* Com que la condicio es que haguem trobat un «:» sabem que a
	 * dir_content en queda com a minim un. El llevam de la sortida.
	 */     
	if (i > 0) {
		dir_content[i] = '\0';
	}
	/* Posam el nom en l'ordre correcte doncs tenim filename a
	 * l'inreves. Posam tambe un caracter null al final. */
	j--;
	for (i = 0; i < (j/2)+1; i++) {
		tmp = filename[i];
		filename[i] = filename[j-i];
		filename[j-i] = tmp;
	}
	j++;
	filename[j] = '\0';
	return 0;
}


/** Obte la informacio d'un fitxer o directori 
 * @path: Cami al fitxer/directori.
 * @stat: el punter de sortida de les dades.
 * @return: 0 si exit.
*/
int emofs_stat(const char *path, emofs_inode_stat *stat) {
	int inode = 0;
	emofs_superblock sb;
	int p_entry = 0; /* No s'empra pero es necessari per cridar find_entry */
	int p_inode_dir = 0;
	int error = 0;

	sbread(&sb);
	p_inode_dir = sb.root_inode;
	error = emofs_find_entry(path, &p_inode_dir, &inode, &p_entry);
	if(error < 0) {
		puts("emofs_stat: no s'ha trobat entrada");
		return error;
	}
	if(inode < 0) {
		puts("emofs_stat: inode negatiu");
		return -1;
	}
	error = stat_file(inode, stat);
	return error;
}

/** Llegeix un cert nombre de bits d'un fitxer.
 * @path: Cami al desti
 * @buf: punter al buffer de sortida, la funcio s'encarrega de reservar l'espai.
 * @offset: determinam el primer byte del fitxer.
 * @n_bytes: nombre de bytes a llegir
 * @return: el nombre de bytes llegits.
*/
int emofs_read(const char *path, void **buf, int offset, int n_bytes) {
	int inode;
	emofs_superblock sb;
	int p_entry; /* No s'empra pero es necessari per cridar find_entry */
	int p_inode_dir;

	sbread(&sb);
	p_inode_dir = sb.root_inode;
	emofs_find_entry(path, &p_inode_dir, &inode, &p_entry);
	return read_file(inode, buf, offset, n_bytes);
}

/** Escriu un cert nombre de bits d'un fitxer. Te control de concurrencia.
 * @path: Cami al desti
 * @buf: buffer de entrada.
 * @offset: determinam el primer byte del fitxer.
 * @n_bytes: nombre de bytes a escriure
 * @return: el nombre de bytes escriure.
*/
int emofs_write(const char *path, const void *buf, int offset, int n_bytes) {
	int inode, error;
	emofs_superblock sb;
        /* No s'empra pero es necessari per cridar find_entry */
	int p_entry; 
	int p_inode_dir;

	if(!mutex){
		emofs_sem_get(&mutex);
	}
	emofs_sem_wait(mutex);
	sbread(&sb);
	p_inode_dir = sb.root_inode;
	emofs_find_entry(path, &p_inode_dir, &inode, &p_entry);
	error = write_file(inode, buf, offset, n_bytes);
	emofs_sem_signal(mutex);
	return error;
}
