#include <time.h>
#include <stdio.h>

#include "bitmap.h"
#include "block.h"
#include "inode.h"
#include "super.h"

int main(int argc, char **argv) {
	int i, nombre_blocs;
	emofs_superblock sb;
	int inici_mb, inici_ai, inici_bd;
	int final_mb, final_ai, final_bd;

	if (argc != 2) {
        puts("Nombre de parametres incorrecte.");
        puts("Us: tests_setmana2 <nom>");
		return -1;
	}

	bmount(argv[1]);

	printf("Tests setmana 2:\n");
	printf("-Llegim el superblock en el seu ESTAT inicial\n");
	sbread(&sb);
	print_sb(&sb);

	printf("Mostram els valors dels defines:\n");

	printf("\t * super.h:\n");
	printf("\t\t SUPERBLOCK_ITEMS = %d\n", SUPERBLOCK_ITEMS);
	printf("\t\t SUPERBLOCK_SIZE = %d\n", SUPERBLOCK_SIZE);
	printf("\t\t PADDING_BYTES = %d\n", PADDING_BYTES);

	printf("\t * inode.h:\n");
	printf("\t\t INODES_PER_BLOCK = %d\n", INODES_PER_BLOCK);
	printf("\t\t INDIRECT_POINTERS_PER_BLOCK = %d\n", \
		 INDIRECT_POINTERS_PER_BLOCK);   
	printf("\t\t INODE_TARGET_SIZE = %d\n", INODE_TARGET_SIZE);
	printf("\t\t INODE_ITEMS = %d\n", INODE_ITEMS);
	printf("\t\t INDIRECT_POINTER_COUNT = %d\n", INDIRECT_POINTER_COUNT);
	printf("\t\t INODE_MUST_SIZE = %d\n", INODE_MUST_SIZE);
	printf("\t\t DIRECT_POINTER_COUNT = %d\n", DIRECT_POINTER_COUNT);

	printf("\t * bitmap.h:\n");
	printf("\t\t MAP_SIZE = %d\n", MAP_SIZE);
	
	bumount();
}
