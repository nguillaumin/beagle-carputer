#include <stdlib.h>
#include <string.h>

#include "hiredis/hiredis.h"

#include "library.h"

const char *KEY_ALL_ALBUMS    = "all-albums";
const char *KEY_ALL_ARTISTS   = "all-artists";
const char *KEY_ALBUM_SONGS   = "album:%s:songs";
const char *KEY_ARTIST_ALBUMS = "artist:%s:albums";

static redisContext *r = NULL;

int library_open() {
	r = redisConnect("127.0.0.1", 6379);
	if (r == NULL) {
		return -1;
	} else if (r->err) {
		printf("Error connecting to Redis: %s (%d)\n", r->errstr, r->err);
		redisFree(r);
		return -1;
	}

	return 0;
}

int library_close(){
	redisFree(r);
	return 0;
}

int library_clear() {
	redisReply *reply = redisCommand(r, "FLUSHDB");
	freeReplyObject(reply);
	return 0;
}

int library_add_song(const song_t *song) {
	if (song->path && song->album && song->artist && song->track) {


		redisAppendCommand(r, "MULTI");
		redisAppendCommand(r, "SADD %s %s", KEY_ALL_ARTISTS, song->artist);
		redisAppendCommand(r, "SADD %s %s", KEY_ALL_ALBUMS, song->album);

		char album_song[strlen(KEY_ALBUM_SONGS)+strlen(song->album)];
		memset(album_song, 0, sizeof(album_song));
		sprintf(album_song, KEY_ALBUM_SONGS, song->album);
		redisAppendCommand(r, "ZADD %s %d %s", album_song, song->track, song->path);

		char artist_albums[strlen(KEY_ARTIST_ALBUMS)+strlen(song->artist)];
		memset(artist_albums, 0, sizeof(artist_albums));
		sprintf(artist_albums, KEY_ARTIST_ALBUMS, song->artist);
		redisAppendCommand(r, "SADD %s %s", artist_albums, song->album);

		redisAppendCommand(r, "EXEC");

		for (int i=0; i<6; i++) {
			redisReply *reply = NULL;
			redisGetReply(r, (void **) &reply);
			free(reply);
		}

	}

	return 0;
}

int library_random_album(char **album) {
	redisReply *reply = redisCommand(r, "SRANDMEMBER all-albums");
	if (reply != NULL) {
		*album = calloc(strlen(reply->str)+1, sizeof(char));
		strcpy(*album, reply->str);
		freeReplyObject(reply);

		return 0;
	}

	return -1;
}

int library_songs(const char *album, int min_track, int max_track, song_t **songs, int *num_songs) {

	char min_range[5] = "-inf";
	char max_range[5] = "+inf";
	if (min_track > 0) {
		sprintf(min_range, "%d", min_track);
	}
	if (max_track > 0) {
		sprintf(max_range, "%d", max_track);
	}

	char *key = calloc(strlen("album:")+strlen(album)+strlen(":songs")+1, sizeof(char));
	sprintf(key, "album:%s:songs", album);

	redisReply *reply = redisCommand(r, "ZRANGEBYSCORE %s %s %s WITHSCORES", key, min_range, max_range);
	fflush(stdout);

	if (reply != NULL) {
		*num_songs = reply->elements/2;
		*songs = calloc(sizeof(song_t), reply->elements/2);

		for(int i=0,j=0; j<reply->elements; i++, j+=2) {
			(*songs)[i].album = calloc(strlen(album)+1, sizeof(char));
			strcpy((*songs)[i].album, album);

			(*songs)[i].path = calloc(strlen(reply->element[j]->str)+1, sizeof(char));
			strcpy((*songs)[i].path, reply->element[j]->str);

			(*songs)[i].track = atoi(reply->element[j+1]->str);
		}

		freeReplyObject(reply);
	}

	free(key);
	return 0;
}
