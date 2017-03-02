all: caseConverter bellower frank caseInverter yeller checker

CC=gcc
CFLAGS=-I. -std=gnu99
DEPS = caseConverter.h, caseInverter.h

%.o: %.c $(DEPS)
	$(CC) -c -o  $@ $< $(CFLAGS)

caseConverter: caseConverter.c AddressUtility.c
	gcc -o caseConverter -std=gnu99 caseConverter.c AddressUtility.c -I.

bellower: bellower.c AddressUtility.c
	gcc -o bellower -std=gnu99 bellower.c AddressUtility.c -I.

frank: frank.c AddressUtility.c
	gcc -o frank -std=gnu99 frank.c AddressUtility.c -I.

caseInverter: caseInverter.c AddressUtility.c 
	gcc -o caseInverter -std=gnu99 caseInverter.c AddressUtility.c -I.

yeller: yeller.c AddressUtility.c
	gcc -o yeller -std=gnu99 yeller.c AddressUtility.c -I.

checker: checker.c AddressUtility.c
	gcc -o checker -std=gnu99 checker.c AddressUtility.c -I.

clean:
	rm -f checker yeller caseInverter frank bellower caseConverter

