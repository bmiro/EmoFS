#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common.h"
#include "dir.h"

/*
 * Llista les entrades de directori. (es comporta com un ls -l de unix)
 * mi_ls <cami>
 * @return: 0 exit, -1 nombre de parametres incorrecte
 */
int main(int argc, char **argv) {
	char path[MAX_PATH_LEN];
	int i, j;
	char msg[MAX_PATH_LEN];
	char full_path[MAX_PATH_LEN];
	char partial_path[MAX_PATH_LEN];
	char filename[MAX_FILENAME_LEN];
	emofs_inode_stat stat;
	int entries_count;
	char type [2];
	int len;
	char *dir_content; /* Reserva la memoria la funcio que el torna */
	/* El nombre de parametres es tots els arguments del programa manco el
	 * propi programa. */
	int how_many_params = argc - 1;
	if (argc < 2) {
		puts("Nombre de parametres incorrecte.");
		puts("Us: mi_ls <cami>");
		return -1;
	}

	memset(msg, '\0', MAX_PATH_LEN);	
	memset(path, '\0', MAX_PATH_LEN);
	memset(full_path, '\0', MAX_PATH_LEN);
	memset(partial_path, '\0', MAX_PATH_LEN);
	memset(filename, '\0', MAX_FILENAME_LEN);

	bmount();
	
	for (i=0; i < how_many_params; i++){
		strcpy(path, argv[1+i]);
		for(j=strlen(path)-1; path[j] == '/' && j > 0; j--) {
			path[j] = '\0';
		}
	
		if(!emofs_file_exists(path)) {
			sprintf(msg, "%s no existeix \n", path);
			fwrite(msg, strlen(msg), 1, stdout);
			continue;
		}
	
		sprintf(msg, "Type \t Size \t\t Epoc \t\t Name \n");
		fwrite(msg, strlen(msg), 1, stdout);		

		if (how_many_params > 1) {
			sprintf(msg, "%s:\n", path);
			fwrite(msg, strlen(msg), 1, stdout);
		}
	

		if (emofs_is_file(path)) {
			/* ls d'un fitxer mostra sols el path del fitxer */
			emofs_stat(path, &stat);
			emofs_get_partial_path(path, partial_path, filename);
			sprintf(msg, "f \t %d \t\t %d \t %s \n",
				stat.size, stat.mtime, filename);
			fwrite(msg, strlen(msg), 1, stdout);
		} else {
			entries_count = emofs_dir(path, &dir_content);
			for (j=0; j < entries_count; j++) {
				emofs_extract_dir_entry(dir_content, filename);
				/*Afegim path a l'element per poder fer el
				 * emofs_stat */
				if (path[strlen(path)-1] != '/')  {
					sprintf(full_path, "%s/%s", path, \
						filename);
				} else {
					sprintf(full_path, "%s%s", path, \
						filename);
				}
				emofs_stat(full_path, &stat);
				sprintf(msg, "%c \t %d \t\t %d \t %s \n", \
					((stat.type==FILE_INODE) ? 'f' : 'd'), \
					stat.size, stat.mtime, filename);
				len = strlen(msg);
				fwrite(msg, len, 1, stdout);
			}
			free(dir_content);
		}
		
		if (how_many_params > 1) {
			sprintf(msg, "\n");
			fwrite(msg, strlen(msg), 1, stdout);
		}
	}
	
	bumount();
	return 0;
}
