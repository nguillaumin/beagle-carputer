#ifndef SONG_H_
#define SONG_H_

typedef struct song {
	char *path;
	char *artist;
	char *album;
	char *title;
	int track;
} song_t;

song_t *song_create();
void song_free(song_t *song);

#endif
