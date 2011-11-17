#include "sem.h"
/** Petit programa per destruir els semàfors. No s'ha de cridar fins haver
 * acabat totes les feines. */
int main() {
	int mutex;
	emofs_sem_get(&mutex);
	emofs_sem_del(mutex);
	return 0;
}
