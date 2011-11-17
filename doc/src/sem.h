/* Modul de semafors per control de concurrencia. El funcionament tipic
 * consisteix en que la funcio obtingui el punter al semafor amb sem_get,
 * demani acces amb un sem_wait i avisi que ha acabat amb un sem_signal. Quan
 * s'executa una simulacio o hi ha la possibilitat de multiples clients cal
 * inicialitzar els semafor amb sem_init. En el cas de les eines d'usuari no es
 * necessari doncs son mono-fil i es suposa que no feim mes d'una tasca a la
 * vegada. */

/** Crea un semafor. 
 * @mutex: punter al semafor
 */
void emofs_sem_init(int *mutex);

/** Destrueix un semafor. 
 * @mutex: punter al semafor
 */
void emofs_sem_del(int mutex);

/** Obte el punter d'un semafor. 
 * @mutex: punter al semafor
 */
void emofs_sem_get(int *mutex);

/** Feim una peticio d'acces a la seccio critica
 * @mutex: punter al semafor
 */
void emofs_sem_wait(int mutex);

/** Assenyalam que hem acabat les tasques de la seccio critica
 * @mutex: punter al semafor
 */
void emofs_sem_signal(int mutex); 
