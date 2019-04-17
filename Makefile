CC = gcc
COMPILER_FLAGS = -g -std=gnu99
LINKER_FLAGS = -g -lpthread -lm
BINARYOSS = oss
BINARYUSER = user
OBJCOMMON = utils.o data.o pcb.o queue.o log.o
OBJSOSS = oss.o
OBJSUSER = user.o
HEADERS = config.h utils.h data.h log.h.

all: $(BINARYOSS) $(BINARYUSER)

$(BINARYOSS): $(OBJSOSS) $(OBJCOMMON)
	$(CC) -o $(BINARYOSS) $(OBJSOSS) $(OBJCOMMON) $(LINKER_FLAGS)

$(BINARYUSER): $(OBJSUSER) $(OBJCOMMON)
	$(CC) -o $(BINARYUSER) $(OBJSUSER) $(OBJCOMMON) $(LINKER_FLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(COMPILER_FLAGS) -c $<

clean:
	/bin/rm $(OBJSOSS) $(OBJSUSER) $(OBJCOMMON) $(BINARYOSS) $(BINARYUSER)

dist:
	zip -r oss.zip *.c *.h Makefile README .git
