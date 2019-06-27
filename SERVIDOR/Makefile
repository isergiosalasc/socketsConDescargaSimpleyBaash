
#VARIABLES (compilador, flags y objects)
CC = gcc
CFLAGS = -Werror -Wall -pedantic
OBJ = servidor.c 
#main.o fuente1.o fuente2.o



all: servidor

#PUEDE IMPRIMIRSE MENSAJES AL USUARIO MEDIANTE "echo"
main: $(OBJ)
	@echo "################# COMPILANDO PROYECTO ###################"
	$(CC) -o servidor 
	



#DECLARARLA PHONY TIENE DOS VENTAJAS: Evita conflictos y mejor performance
clean:
	find . -type f | xargs touch
	rm -rf *.o servidor

