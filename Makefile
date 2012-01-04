CC=i586-mingw32msvc-gcc
CFLAGS=-ansi -pedantic -Wall -Werror -O2 -I./inc/ -D_DEBUG
LDFLAGS=-Wl,--enable-stdcall-fixup
LIBS=-luuid -lws2_32
OBJS=src/wolapi.o src/Chat.o src/IChat.o src/NetUtil.o src/INetUtil.o src/IConnectionPointContainer.o src/IWOLConnectionPoint.o src/irc.o src/apgar.o

all: wolapi.dll

src/%.o: src/%.c inc/%.h inc/wolapi.h
	$(CC) $(CFLAGS) -c -o $@ $<

wolapi.dll: $(OBJS)
	$(CC) $(LDFLAGS) -shared -o wolapi.dll $(OBJS) res/wolapi.def $(LIBS)

clean:
	rm -rf wolapi.dll src/*.o
