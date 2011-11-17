# compilador
C = gcc -pedantic -g -ggdb

# biblioteques
L = block.o bitmap.o block_lib.o super.o inode.o dir.o file.o sem.o

# nombre de blocs
F = 1000

all: libs utils bins sim

go: all
	./mi_mkfs $F
	nemiver	./mi_mkdir /a

kate-edit:
	kate *.c *.h Makefile

debug-mi_mkfs: mi_mkfs
	nemiver "mi_mkfs primer.emo 1000"

entrega:
	cd doc
	make complet
	cd ..
	cd ..
	tar czf aso aso_BartomeuMiro-PauRuŀlan.tar.gz

clean:
	rm *.o  \
	sim \
	mi_mkfs mi_ls mi_ln mi_stat mi_cat mi_rm mi_mkdir mi_touch \
	tests_mapa_bits tests_setmana1 tests_setmana2 2> /dev/null

##############################################################################
# Sintaxi

syntax:	libs utils

libs: $L

utils: ls.o ln.o stat.o cat.o rm.o mkdir.o touch.o mkfs.o append.o

bitmap.o:
	$C -c bitmap.c

block.o: 
	$C -c block.c

block_lib.o:
	$C -c block_lib.c

inode.o: 
	$C -c inode.c

super.o: 
	$C -c super.c

file.o:
	$C -c file.c

dir.o: 
	$C -c dir.c

sem.o: 
	$C -c sem.c


ls.o: 
	$C -c mi_ls.c
ln.o:
	$C -c mi_ln.c
stat.o:
	$C -c mi_stat.c
cat.o:
	$C -c mi_cat.c
rm.o:
	$C -c mi_rm.c
mkfs.o:
	$C -c mi_mkfs.c
mkdir.o:
	$C -c mi_mkdir.c
touch.o:
	$C -c mi_touch.c
append.o:
	$C -c mi_append.c
write.o:
	$C -c mi_write.c
mount.o:
	$C -c mi_mount.c
umount.o:
	$C -c mi_umount.c


##############################################################################
# Arxius de test

tests_setmana1: tests_setmana1.o
	$C -o tests_setmana1 tests_setmana1.o block.o
	./tests_setmana1

tests_setmana1.o: block.o block.o super.o
	$C -c tests_setmana1.c

tests_setmana2: tests_setmana2.o
	$C -o tests_setmana2 block.o super.o tests_setmana2.o
	./tests_setmana2 primer.emo

tests_setmana2.o: block.o super.o
	$C -c tests_setmana2.c

tests_setmana3.o: bitmap.o block_lib.o inode.o super.o
	$C -c tests_setmana3.c

tests_setmana3: tests_setmana3.o syntax
	$C -o tests_setmana3 tests_setmana3.o block.o bitmap.o block_lib.o inode.o super.o 
	./tests_setmana3 primer.emo 10 10

tests_mapa_bits: bitmap.o tests_mapa_bits.o 
	$C -o tests_mapa_bits tests_mapa_bits.o bitmap.o super.o block.o
	./tests_mapa_bits

tests_mapa_bits.o:
	$C -c tests_mapa_bits.c

###########################################################
# Utilitats d'usuari
ls: ls.o libs
	$C -o mi_ls mi_ls.o $L
ln: ln.o libs
	$C -o mi_ln mi_ln.o $L
stat: stat.o libs
	$C -o mi_stat mi_stat.o $L
cat: cat.o libs
	$C -o mi_cat mi_cat.o $L
rm: rm.o libs
	$C -o mi_rm mi_rm.o $L
mkdir: mkdir.o libs
	$C -o mi_mkdir mi_mkdir.o $L
touch: touch.o libs
	$C -o mi_touch mi_touch.o $L
mkfs: mi_mkfs.o libs
	$C -o mi_mkfs mi_mkfs.o $L
append: append.o libs
	$C -o mi_append mi_append.o $L
write: write.o libs
	$C -o mi_write mi_write.o $L
mount: mount.o libs
	$C -o mi_mount mi_mount.o $L
umount: umount.o libs
	$C -o mi_umount mi_umount.o $L

#L = block.o bitmap.o block_lib.o super.o inode.o dir.o file.o sem.o

bins: ls ln stat cat rm mkdir touch mkfs sim append write mount umount

##############################################################################
sim.o:
	$C -c sim.c
sim: sim.o libs
	$C -o sim sim.o $L
#export PATH=$PATH:.
