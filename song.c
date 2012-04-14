#include <stdlib.h>

#include "song.h"

song_t *song_create() {
	song_t *song = calloc(1, sizeof(song_t));
	return song;
}

void song_free(song_t *song) {
	free(song->path);
	free(song->artist);
	free(song->album);
	free(song->title);
	free(song);
}
