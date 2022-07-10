#
# Variables
#

#Project
EXEC_NAME = webserv
PROJ_NAME = sppA
CLSS_NAME = sysprog
SBMT_FILES = makefile src/* index.html

#Compiler
CC = gcc
C_FLAGS = -Wall
C_LIBS = 



#
# Recipes
#

#File recipies
%.o: %.c
	$(CC) -c $(C_FLAGS) $(C_LIBS) -o $@ $<

$(EXEC_NAME): $(patsubst %.c,%.o,$(wildcard src/*.c))
	$(CC) $(C_FLAGS) $(C_LIBS) -o $@ $^

#Phony recipes
.PHONY: clean
clean:
	rm $(EXEC_NAME) $(patsubst %.c,%.o,$(wildcard src/*.c))

.PHONY: submit
submit:
	@submit ${CLSS_NAME} ${PROJ_NAME} ${SBMT_FILES}

.PHONY: debug
debug: C_FLAGS := ${C_FLAGS} -ggdb
debug: $(EXEC_NAME)
#	gdb $(EXEC_NAME)
