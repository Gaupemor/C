all: ruterdrift

ruterdrift: oppgave1.c
	gcc -g -o ruterdrift oppgave1.c

run: ruterdrift
	@echo
	./ruterdrift 10_routers_10_edges kommandoer_10_routers.txt
	@echo
	./ruterdrift 50_routers_150_edges kommandoer_50_rutere.txt

test: ruterdrift
	@echo
	valgrind ./ruterdrift 10_routers_10_edges kommandoer_10_routers.txt
	@echo
	valgrind ./ruterdrift 50_routers_150_edges kommandoer_50_rutere.txt

clean:
	rm -f ruterdrift
	rm -f a.out
