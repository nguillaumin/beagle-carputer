#include <stdlib.h>
#include <string.h>

#include "hiredis/hiredis.h"
#include "player_state.h"

const char *KEY_STATE = "player-state";
const char *HKEY_ALBUM = "album";
const char *HKEY_TRACK = "track";
const char *HKEY_SONG_TIME_ELAPSED = "song-time-elapsed";

int player_state_store(const char *album, int track, double song_time_elapsed) {
	redisContext *r = redisConnect("127.0.0.1", 6379);
	if (r == NULL) {
		return -1;
	} else if (r->err) {
		redisFree(r);
		return -1;
	}

	redisReply *reply = NULL;
	reply = redisCommand(r, "HSET %s %s %s", KEY_STATE, HKEY_ALBUM, album);
	freeReplyObject(reply);

	reply = redisCommand(r, "HSET %s %s %d", KEY_STATE, HKEY_TRACK, track);
	freeReplyObject(reply);

	reply = redisCommand(r, "HSET %s %s %f", KEY_STATE, HKEY_SONG_TIME_ELAPSED, song_time_elapsed);
	freeReplyObject(reply);

	return 0;
}

int player_state_clear() {
	redisContext *r = redisConnect("127.0.0.1", 6379);
	if (r == NULL) {
		return -1;
	} else if (r->err) {
		redisFree(r);
		return -1;
	}

	redisReply *reply = redisCommand(r, "DEL %s", KEY_STATE);
	freeReplyObject(reply);
	redisFree(r);

	return 0;
}

int player_state_restore(char **album, int *track, double *song_time_elapsed) {
	redisContext *r = redisConnect("127.0.0.1", 6379);
	if (r == NULL) {
		return -1;
	} else if (r->err) {
		redisFree(r);
		return -1;
	}

	int rc = 0;

	redisReply *reply = redisCommand(r, "HGETALL %s", KEY_STATE);
	if (reply) {
		if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 6) {
			for(int i=0; i<reply->elements; i++) {
				char *p = reply->element[i]->str;
				if (! strcmp(HKEY_ALBUM, p)) {
					*album = calloc(strlen(reply->element[i+1]->str)+1, sizeof(char));
					strcpy(*album, reply->element[++i]->str);
				} else if (! strcmp(HKEY_TRACK, p)) {
					*track = atoi(reply->element[++i]->str);
				} else if (! strcmp(HKEY_SONG_TIME_ELAPSED, p)) {
					*song_time_elapsed = atof(reply->element[++i]->str);
				}
			}
		}
		rc = -1;
		freeReplyObject(reply);
		redisFree(r);

	} else {
		rc = -1;
	}

	return rc;
}
