CFLAGS := -Wall -std=c99 -g -D_POSIX_C_SOURCE=199309L
LDFLAGS := -lvorbisfile -lhiredis -lao -lpthread

all: build-library player

player: player.o player_state_redis.o library_redis.o song.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	
build-library: build-library.o library_redis.o metadata_parser_ogg.o song.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	
.c.o:
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f *.o
	rm -f build-library player
