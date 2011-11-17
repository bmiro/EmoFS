/* Mòdul de semàfors per control de concurrència. El funcionament típic
 * consisteix en que la funció obtingui el punter al semàfor amb sem_get,
 * demani accés amb un sem_wait i avisi que ha acabat amb un sem_signal. Quan
 * s'executa una simulació o hi ha la possibilitat de múltiples clients cal
 * inicialitzar els semàfor amb sem_init. En el cas de les eines d'usuari no és
 * necessari doncs són mono-fil i es suposa que no feim més d'una tasca a la
 * vegada. */

/** Crea un semàfor. 
 * @mutex: punter al semàfor
 */
void emofs_sem_init(int *mutex);

/** Destrueix un semàfor. 
 * @mutex: punter al semàfor
 */
void emofs_sem_del(int mutex);

/** Obté el punter d'un semàfor. 
 * @mutex: punter al semàfor
 */
void emofs_sem_get(int *mutex);

/** Feim una petició d'accés a la secció crítica
 * @mutex: punter al semàfor
 */
void emofs_sem_wait(int mutex);

/** Assenyalam que hem acabat les tasques de la secció crítica
 * @mutex: punter al semàfor
 */
void emofs_sem_signal(int mutex); 
