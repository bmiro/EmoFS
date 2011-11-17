#include "sem.h"
/** Petit programa per inicialitzar els semafors. Cal cridar-lo abans d'usar
 * qualsevol eina. */
int main() {
	int mutex;
	emofs_sem_init(&mutex);
	return 0;
}
