
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h> 
#include <stdio.h>

#include "sem.h"

#define SEM_KEY_PATH "/"

/* Obté la clau comuna pel semàfor */
int get_key(void) {
	return ftok(SEM_KEY_PATH, 1);
}

/** Crea un semàfor. 
 * @mutex: punter al semàfor
 */
void emofs_sem_init(int *mutex){
	emofs_sem_get(mutex);
	if(*mutex < 0) {
		*mutex = semget(get_key(), 1, 0600 | IPC_CREAT);
		semctl(*mutex,0,SETVAL,1); 
	}
} 

/** Destrueix un semàfor. 
 * @mutex: punter al semàfor
 */
void emofs_sem_del(int mutex) {
	semctl(mutex, 0, IPC_RMID, 0);
}

/** Obté el punter d'un semàfor. 
 * @mutex: punter al semàfor
 */
void emofs_sem_get(int *mutex){
	*mutex = semget(get_key(), 1, 0600);
}

/** Feim una petició d'accés a la secció crítica
 * @mutex: punter al semàfor
 */
void emofs_sem_wait(int mutex){
	struct sembuf op_P [] = {0, -1, 0};
	semop(mutex, op_P, 1);
}

/** Assenyalam que hem acabat les tasques de la secció crítica
 * @mutex: punter al semàfor
 */
void emofs_sem_signal(int mutex){
	struct sembuf op_V [] = {0, 1, 0};
	semop(mutex, op_V, 1);
}
