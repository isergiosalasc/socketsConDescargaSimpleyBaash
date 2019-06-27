
# #VARIABLES (compilador, flags y objects)
CC = gcc
CFLAGS = -Werror -Wall -pedantic
OBJ = cliente.c
#main.o fuente1.o fuente2.o



all: cliente

#PUEDE IMPRIMIRSE MENSAJES AL USUARIO MEDIANTE "echo"
main: $(OBJ)
	#@echo "################# COMPILANDO PROYECTO ###################"
	$(CC) -o cliente
	



#DECLARARLA PHONY TIENE DOS VENTAJAS: Evita conflictos y mejor performance
clean:
	find . -type f | xargs touch
	rm -rf *.o cliente

