#include "data_block.h"

#include "super.h"

int data_read(int n_block, emofs_block *block) {
	emofs_superblock sb;

	sbread(&sb);
	return bread(n_block + sb.first_data_block, block);
}

int data_write(int n_block, emofs_block *block) {
	emofs_superblock sb;
	
	sbread(&sb);
	return bwrite(n_block + sb.first_data_block, block);
}
