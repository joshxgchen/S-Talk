all: maker 

maker:
	gcc -Wall -pthread s-talk.c list.o helper.c -o s-talk

clean: 
	rm s-talk
