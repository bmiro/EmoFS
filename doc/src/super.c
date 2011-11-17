#include <stdio.h>
#include "super.h"

/** Escriu al superblock.
 * @sb: Contingut del superblock
 * @return: error si n'hi ha.
 */
int sbwrite(emofs_superblock *sb) {
	return bwrite(SUPERBLOCK_BLOCK_N, sb);
}

/** Llegeix el superblock.
 * @sb: Contingut del superblock
 * @return: error si n'hi ha.
 */
int sbread(emofs_superblock *sb) {
	return bread(SUPERBLOCK_BLOCK_N, sb);
}

/** Imprimeix el contigut del superbloc.
 * @sb: el superblock.
 */
void print_sb(emofs_superblock *sb) {
	printf("first_bitm_block %d \n", sb->first_bitm_block);
	printf("sb.last_bitm_block %d \n", sb->last_bitm_block);
	printf("sb.first_inode_block %d \n", sb->first_inode_block);
	printf("sb.last_inode_block %d \n", sb->last_inode_block);
	printf("sb.first_data_block %d \n", sb->first_data_block);
	printf("sb.last_data_block %d \n", sb->last_data_block);
	printf("sb.root_inode %d \n", sb->root_inode);
	printf("sb.first_free_inode %d \n", sb->first_free_inode);
	printf("sb.free_block_count %d \n", sb->free_block_count);
	printf("sb.free_inode_count %d \n", sb->free_inode_count);
	printf("sb.total_block_count %d \n", sb->total_block_count);
	printf("sb.total_inode_count %d \n", sb->total_inode_count);
	printf("sb.total_data_block_count %d\n", sb->total_data_block_count);
}
