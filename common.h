/* Mòdul de definició de paràmetres comuns de tot el sistema de fitxers. */

/* Definició dels paràmetres comuns al llarg de la pràctica. */
#define __u32 unsigned int
#define __u16 unsigned short

/* Un bloc té 4KB, com a minim pot ser de 512 Bytes (segons l'enunciat) */
#define BLOCK_SIZE 4096

/* Bloc on desam el superblock. */
#define SUPERBLOCK_BLOCK_N 0

#define NULL_POINTER (-1)

/* 60 perque 60B+4B d'un int fan 64B. D'aquesta manera podem fer caure bé els
 * dins els blocs. */
#define MAX_FILENAME_LEN 60

#define MAX_PATH_LEN 255

#define EMOFS_IMAGE_FILE "emo.fs"

/* Definicions d'accés als inodes. */
#define FREE_INODE 0
#define DIRECTORY_INODE 1
#define FILE_INODE 2

#define BUFFER_SIZE 255

