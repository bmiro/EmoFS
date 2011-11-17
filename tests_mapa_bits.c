#include <stdio.h>
#include "bitmap.h"

int main(void) {
	int i, zeros, uns;
	emofs_bitmap mapa = map_of_zero();

	mflip(&mapa);
	 
	for(i = BLOCK_SIZE*8/2; i < BLOCK_SIZE*8; i++) {
		mwrite(i, 0, &mapa);
	}

	mflip(&mapa);

	for(i = 0; i < BLOCK_SIZE*8; i++) {
		if (mread(i, &mapa) == 0) {
			zeros++;
		} else {
			uns++;
		}
	}

	printf("hi ha 0: %d, 1: %d\n", zeros, uns);
	if (zeros == uns) {
		printf("Test passat correctament.\n");
	} else {
		printf("Error al test.\n");
	}
	
	return 0;
}

