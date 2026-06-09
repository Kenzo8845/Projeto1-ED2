PROJ_NAME = ted

CC      = gcc
CFLAGS  = -ggdb -O0 -std=c99 -fstack-protector-all \
           -Werror=implicit-function-declaration \
           -Wall -Wextra -Wpedantic \
           -Iinclude \
           -I../include
LDFLAGS = -O0

RM = rm -f
EXE =
PATHSEP = /
RUN = ./

# Objetos do programa principal

OBJETOS = main.o \
          hash.o \
          quadra.o \
          habitante.o \
          svg.o \
          parser_geo.o \
          parser_pm.o \
          parser_qry.o

# Unity

UNITY_DIR = ../Unity
UNITY_OBJ = $(UNITY_DIR)/unity.o

$(UNITY_OBJ): $(UNITY_DIR)/unity.c  $(CC) -c $(CFLAGS) -I$(UNITY_DIR) -DUNITY_INCLUDE_DOUBLE $< -o $@

# Executável principal

ted: $(OBJETOS)
	$(CC) $(LDFLAGS) -o $(PROJ_NAME)$(EXE) $(OBJETOS)

# Regras de compilação dos módulos

main.o: main.c ../include/hash.h ../include/parser_geo.h \
        ../include/parser_pm.h ../include/parser_qry.h ../include/svg.h
	$(CC) -c $(CFLAGS) $< -o $@

hash.o: hash.c ../include/hash.h
	$(CC) -c $(CFLAGS) $< -o $@

quadra.o: quadra.c ../include/quadra.h
	$(CC) -c $(CFLAGS) $< -o $@

habitante.o: habitante.c ../include/habitante.h
	$(CC) -c $(CFLAGS) $< -o $@

svg.o: svg.c ../include/svg.h ../include/quadra.h ../include/hash.h
	$(CC) -c $(CFLAGS) $< -o $@

parser_geo.o: parser_geo.c ../include/parser_geo.h \
              ../include/hash.h ../include/quadra.h ../include/svg.h
	$(CC) -c $(CFLAGS) $< -o $@

parser_pm.o: parser_pm.c ../include/parser_pm.h \
             ../include/hash.h ../include/habitante.h
	$(CC) -c $(CFLAGS) $< -o $@

parser_qry.o: parser_qry.c ../include/parser_qry.h \
              ../include/hash.h ../include/quadra.h \
              ../include/habitante.h ../include/svg.h
	$(CC) -c $(CFLAGS) $< -o $@

# Flags de compilação dos testes

TST_FLAGS = $(CFLAGS) -I$(UNITY_DIR) -DUNITY_INCLUDE_DOUBLE

# Testes unitários

../test/test_hash$(EXE): ../test/test_hash.c hash.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_hash: ../test/test_hash$(EXE)
	$(RUN)..$(PATHSEP)test$(PATHSEP)test_hash$(EXE)

../test/test_quadra$(EXE): ../test/test_quadra.c quadra.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_quadra: ../test/test_quadra$(EXE)
	$(RUN)..$(PATHSEP)test$(PATHSEP)test_quadra$(EXE)

../test/test_habitante$(EXE): ../test/test_habitante.c habitante.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_habitante: ../test/test_habitante$(EXE)
	$(RUN)..$(PATHSEP)test$(PATHSEP)test_habitante$(EXE)

../test/test_svg$(EXE): ../test/test_svg.c svg.o quadra.o hash.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_svg: ../test/test_svg$(EXE)
	$(RUN)..$(PATHSEP)test$(PATHSEP)test_svg$(EXE)

../test/test_parser_geo$(EXE): ../test/test_parser_geo.c parser_geo.o \
                                hash.o quadra.o svg.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_parser_geo: ../test/test_parser_geo$(EXE)
	$(RUN)..$(PATHSEP)test$(PATHSEP)test_parser_geo$(EXE)

../test/test_parser_pm$(EXE): ../test/test_parser_pm.c parser_pm.o \
                               hash.o habitante.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_parser_pm: ../test/test_parser_pm$(EXE)
	$(RUN)..$(PATHSEP)test$(PATHSEP)test_parser_pm$(EXE)

../test/test_parser_qry$(EXE): ../test/test_parser_qry.c parser_qry.o \
                                hash.o quadra.o habitante.o svg.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_parser_qry: ../test/test_parser_qry$(EXE)
	$(RUN)..$(PATHSEP)test$(PATHSEP)test_parser_qry$(EXE)

# Roda todos os testes

tstall: test_hash test_quadra test_habitante test_svg \
        test_parser_geo test_parser_pm test_parser_qry

# Limpeza

clean:
	$(RM) *.o $(PROJ_NAME) $(UNITY_OBJ)
	$(RM) ../test/test_hash ../test/test_quadra ../test/test_habitante
	$(RM) ../test/test_svg ../test/test_parser_geo ../test/test_parser_pm ../test/test_parser_qry

.PHONY: ted tstall clean \
        test_hash test_quadra test_habitante \
        test_svg test_parser_geo test_parser_pm test_parser_qry