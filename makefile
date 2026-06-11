CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

SRC = main.c adresse.c station.c switch_reseau.c reseau.c trame.c
OBJ = $(SRC:.c=.o)

EXEC = sae23

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

clean:
	rm -f $(OBJ) $(EXEC)