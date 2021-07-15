dir ?= ../../../bin
SRC := ${dir}/src/
INCLUDE := ${dir}/src/
OBJ := ${dir}/obj/
BIN := ${dir}/bin/
CC := gcc
#CFLAGS= -Wall -I.. -I../../UnitTest -I.
CFLAGS := -I${INCLUDE} -I. #-DDEBUG#-Wall

TARGET = OneLeafNoInstruction OneLeaf TwoLeaves OneTeamOneLeaf OneTeamTwoLeaves TwoTeamsOneCycle ThreeTeamsOneCycleThreeLeaves

all = ${TARGET}

OneLeafNoInstruction : % : ${OBJ}%.o ${OBJ}%_program.o ${OBJ}main%.o
	${CC} $^ -o ${BIN}$@

OneLeaf : % : ${OBJ}%.o ${OBJ}%_program.o ${OBJ}main%.o
	${CC} $^ -o ${BIN}$@

TwoLeaves : % : ${OBJ}%.o ${OBJ}%_program.o ${OBJ}main%.o
	${CC} $^ -o ${BIN}$@

ThreeLeaves : % : ${OBJ}%.o ${OBJ}%_program.o ${OBJ}main%.o
	${CC} $^ -o ${BIN}$@

OneTeamOneLeaf : % : ${OBJ}%.o ${OBJ}%_program.o ${OBJ}main%.o
	${CC} $^ -o ${BIN}$@

OneTeamTwoLeaves : % : ${OBJ}%.o ${OBJ}%_program.o ${OBJ}main%.o
	${CC} $^ -o ${BIN}$@

TwoTeamsOneCycle : % : ${OBJ}%.o ${OBJ}%_program.o ${OBJ}main%.o
	${CC} $^ -o ${BIN}$@

ThreeTeamsOneCycleThreeLeaves : % : ${OBJ}%.o ${OBJ}%_program.o ${OBJ}main%.o
	${CC} $^ -o ${BIN}$@

TicTacToeBest_TPG : % : ${OBJ}%.o ${OBJ}%_program.o ${OBJ}main%.o
	${CC} $^ -o ${BIN}$@ -lm

StickGameBest_TPG : % : ${OBJ}%.o ${OBJ}%_program.o ${OBJ}main%.o
	${CC} $^ -o ${BIN}$@ -lm

${OBJ}main%.o : main%.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ}%.o : ${SRC}%.c
	${CC} -c ${CFLAGS} $^ -o $@



clean:
	rm -f ${TARGET} ${OBJ}*.o