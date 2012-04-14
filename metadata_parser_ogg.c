#include <stdlib.h>
#include <string.h>
#include <vorbis/vorbisfile.h>

#include "library.h"
#include "metadata_parser.h"

const char *ARTIST = "ARTIST=";
const char *ALBUM = "ALBUM=";
const char *TRACKNUMBER = "TRACKNUMBER=";
const char *TITLE = "TITLE=";

void parse_metadata(const char *path, song_t *song) {
	OggVorbis_File vf;

	int rc = ov_fopen(path, &vf);
	if (rc) {
		fprintf(stderr, "Could not open '%s'\n", path);
		return;
	}

	vorbis_comment *comment = ov_comment(&vf, -1);
	if (comment == NULL) {
		fprintf(stderr, "Could not read comments from '%s'\n", path);
		return;
	}

	song->path = calloc(strlen(path)+1, sizeof(char));
	strcpy(song->path, path);

	char *buf;
	for (int i=0; i < comment->comments; i++) {
		buf = calloc(comment->comment_lengths[i]+1, sizeof(char));
		strncpy(buf, comment->user_comments[i], comment->comment_lengths[i]);
		buf[comment->comment_lengths[i]] = '\0';

		int equals = 0;
		for (; buf[equals] != '=' && equals < strlen(buf); equals++);

		int len = 0;
		if (equals < strlen(buf)) {
			if (! strncmp(buf, ARTIST, strlen(ARTIST))) {
				len = strlen(buf)-strlen(ARTIST)+1;
				song->artist = calloc(len, sizeof(char));
				strncpy(song->artist, buf+equals+1, len);
			} else if (! strncmp(buf, ALBUM, strlen(ALBUM))) {
				len = strlen(buf)-strlen(ALBUM)+1;
				song->album = calloc(len, sizeof(char));
				strncpy(song->album, buf+equals+1, len);
			} else if (! strncmp(buf, TITLE, strlen(TITLE))) {
				len = strlen(buf)-strlen(TITLE)+1;
				song->title = calloc(len, sizeof(char));
				strncpy(song->title, buf+equals+1, len);
			} else if ( !strncmp(buf, TRACKNUMBER, strlen(TRACKNUMBER))) {
				len = strlen(buf)-strlen(TRACKNUMBER);
				song->track = atoi(buf+equals+1);
			}

		}

		free(buf);
	}

	ov_clear(&vf);
}
