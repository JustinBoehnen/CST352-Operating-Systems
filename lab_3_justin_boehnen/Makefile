OPTS=-dbg

all: os user

clean:
	rm -f *.o
	rm -f *.ast
	rm -f *.dbg
	rm -f *.sl
	rm -f *.slb


usermode.sl: usermode.h usermode.c
	stacklc ${OPTS} usermode.c

user.sl: user.c usermode.h
	stacklc ${OPTS} user.c

os.sl: os.c
	stacklc ${OPTS} os.c

user: user.sl usermode.sl
	slasm user.sl usermode.sl

os: os.sl
	slasm os.sl
