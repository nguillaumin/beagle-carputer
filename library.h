#ifndef LIBRARY_H_
#define LIBRARY_H_

#include "song.h"

int library_open();
int library_close();

int library_clear();
int library_add_song(const song_t *song);

int library_random_album(char **album);
int library_songs(const char *album, int min_track, int max_track, song_t **songs, int *num_songs);

#endif
