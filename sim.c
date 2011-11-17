/** Simulador de la pràctica del sistema de fitxers. El programa consisteix en
 * crear una imatge de sf, crear un seguit de fils i anar escrivint adins el
 * sistema.
 */

#include <signal.h> 
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "dir.h"
#include "block.h"

/* Quants de fils escriptors llançarem */
#define PROCESS_NUMBER 100
/* Nombre d'escriptures que farà un fil escriptor */
#define CHILDREN_WRITTING 5

/* El temps a esperar entre fill i fill, en nanosegons */
#define NEXT_CHILDREN_WAIT 1000000
/* El temps d'espera d'un fill a fer la següent escriptura */
#define CHILDREN_WAIT_TIME 100000 
#define MAX 2000 

/* Necessitarem contar el nombre de fills que ja han acabat */
static int ENDED_CHILDREN = 0;
static char SIM_PATH[256];
static int mutex = 0;

/* Clàssic foser o enterrador. Compta el nombre de fills morts per saber quan
 * podem aturar el programa.
 */
void reaper() {
	while(wait3(NULL, WNOHANG, NULL) > 0) {
		ENDED_CHILDREN++;
	}
}

/* Crea el directori dins del sistema de fitxers segons el moment actual. És de
 * la forma "simul_aaaammddhhmmss"
 */ 
int init_fs() {
	time_t t_time;
	struct tm *t;
	time(&t_time);
	t = localtime(&t_time);
	sprintf(SIM_PATH, "/simul_%d%d%d%d%d%d/",
		1900 + t->tm_year, 1 + t->tm_mon, t->tm_mday,
		t->tm_hour, t->tm_min, t->tm_sec);
	emofs_create(SIM_PATH);
	
	/**/
	/* puts("ruta a crear"); */
	/* puts(SIM_PATH); */
	/**/

	return 0;
}

/* Feina dels fils escriptors. Primer creen un directori de la forma
"proceso_n", on n és el seu PID. Allà farà un prueba.dat. Cada 0,1s han de fer
una escriptura "hh:mm:ss Escritura número i a partir de la posición j". i és el
nombre d'escriptura i j la posició, la qual és aleatòria. */
int sim_work() {
	int i = 0;
	char local_path[256];
	char buff[256];
	time_t t_time;
	struct tm *t;
	int offset;
	printf("Inici worker %d\n", getpid());

	strcpy(local_path, SIM_PATH);
	sprintf(buff, "process_%d/", getpid());
	strcat(local_path, buff);
	emofs_create(local_path);
	/* Un cop creat el directori podem determinar el nom final */
	strcat(local_path, "prueba.dat");
	emofs_create(local_path);
	/**/
	/* puts("sim: worker: fitxer local"); */
	/* puts(local_path); */
	/**/
	srand(time(NULL));
	for(i = 0; i < CHILDREN_WRITTING; i++){
		offset = rand() % MAX;
		time(&t_time);
		t = localtime(&t_time);
		sprintf(buff, "%d:%d:%d escriptura nombre %d a la posicio %d\n",
			t->tm_hour, t->tm_min, t->tm_sec, i, offset);
		emofs_write(local_path, buff, offset, strlen(buff));
	}
	/**/
	printf("sim: worker: final client amb pid %d\n", getpid());
	/**/
	return 0;
}


void mi_ls(char *path) {
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
	/* El nombre de paràmetres és tots els arguments del programa manco el
	 * propi programa. */

       sprintf(msg, "Type \t Size \t\t Epoc \t\t Name \n");
       fwrite(msg, strlen(msg), 1, stdout);		

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
       }
}

void mi_cat(char *path) {
	emofs_inode_stat info;
	int i;
	char *buffer;
	emofs_stat(path, &info);
	for(i = 0; i < info.size; i++) {
		emofs_read(path, (void *)&buffer, i, 1);
		fwrite(buffer, 1, 1, stdout);
	}
	free(buffer);
}

/* Imprimeix el llistat de tots els fitxers que s'han generat amb els seus
 * tamanys. Mostra el contigut d'un fitxer a l'atzar. */
int show_work() {
	int i, j, max;
	char path[256];
	char tmp[256];
	emofs_dir_entry *dir_entry;

	for(j=strlen(SIM_PATH)-1; SIM_PATH[j] == '/' && j > 0; j--) {
		SIM_PATH[j] = '\0';
	}

	mi_ls(SIM_PATH);


	/* Després de fer el emofs_dir s'obté una llista de fitxers separats
	 * per dos punts. Basta agafar el primer i imprimir el contigut */
	emofs_read(SIM_PATH,(void *)&dir_entry, 0, sizeof(emofs_dir_entry));
	sprintf(path, "%s/%s/prueba.dat", SIM_PATH, dir_entry->name);
	free(dir_entry);

	/* */
	puts("");
	puts("sim: anam a mostrar un fitxer dels creats");
	puts(path);
	puts("-------------------------------------");
	/* */
	mi_cat(path);
	/* */
	puts("-------------------------------------");
	/* */

	return 0;
}

int main() {
	int i;
	/* mi_mount */
	emofs_sem_init(&mutex);
	bmount();

	init_fs();
	signal(SIGCHLD, reaper);
	for(i = 0; i < PROCESS_NUMBER; i++) {
		if (fork() == 0) {
			sim_work();
			exit(0);
		} else {
			usleep(NEXT_CHILDREN_WAIT);
		}
	}
	while (ENDED_CHILDREN < PROCESS_NUMBER) {
		pause();
	}
	show_work();
	puts("Simulació acabada");

	/* mi_umount */
	bumount();

	emofs_sem_get(&mutex);
	emofs_sem_del(mutex);

	return 0;
}
