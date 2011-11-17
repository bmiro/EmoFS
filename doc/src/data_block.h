#include "block.h"

/** Llegeix un bloc de dades.
 * @n_block: el nombre del bloc de dades.
 * @block: buffer del bloc.
 * @return: 0 si exit.
*/
int data_read(int n_block, emofs_block *block);

/** Escriu un bloc de dades.
 * @n_block: el nombre del bloc de dades.
 * @block: buffer del bloc.
 * @return: 0 si exit.
*/
int data_write(int n_block, emofs_block *block);
