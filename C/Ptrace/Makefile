TARGET=inject
SOURCE_FILES=${TARGET}.c
OBJS=${TARGET}.o

TEST_TARGET=test
TEST_SOURCE_FILES=${TEST_TARGET}.c
TEST_OBJS=${TEST_TARGET}.o

LIB_TARGET=hello
LIB_SOURCE_FILES=${LIB_TARGET}.c
LIB_OBJS=${LIB_TARGET}.o

CC=gcc
CINCLUDE=

CFLAGS=-g
CLIBS=
CLIBS_EX=-ldl
CLIBPATHS=

all:${TARGET} ${TEST_TARGET} ${LIB_TARGET}

${TARGET}: ${OBJS}
	${CC} ${CLIBPATHS} ${CLIBS} -o$@ $< ${CLIBS_EX}
	@if [ $$? ];then rm ${OBJS};echo "Make success"; else echo "Make fialed";fi

${OBJS}:
	${CC} ${CFLAGS} -c ${SOURCE_FILES}

${TEST_TARGET}: ${TEST_OBJS}
	${CC} -o$@ $<
	@if [ $$? ];then rm ${TEST_OBJS};echo "Make success"; else echo "Make fialed";fi

${TEST_OBJS}:
	${CC} ${CFLAGS} -c ${TEST_SOURCE_FILES}

${LIB_TARGET}:${LIB_OBJS}
	${CC} -shared -olib$@.so $<
	@if [ $$? ];then rm ${LIB_OBJS};echo "Make success"; else echo "Make fialed";fi

${LIB_OBJS}:
	${CC} ${CFLAGS} -fPIC -c ${LIB_SOURCE_FILES}

clean:
	@if [ -e *.o ];then echo "rm *.o";rm *.o;fi
	@if [ -e a.out ];then echo "rm a.out";rm a.out; fi
	@if [ -e ${TARGET} ];then echo "rm ${TARGET}";rm ${TARGET};fi
	@if [ -e ${TEST_TARGET} ];then echo "rm ${TEST_TARGET}";rm ${TEST_TARGET};fi
	@if [ -e lib${LIB_TARGET}.so ];then echo "rm lib${LIB_TARGET}.so";rm lib${LIB_TARGET}.so;fi

