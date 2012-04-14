#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <error.h>
#include <errno.h>

#include "build-library.h"
#include "song.h"
#include "library.h"
#include "metadata_parser.h"

static char root[256] = {0};

static int num_files = 0;
static int num_dirs = 0;

int main(int argc, char *argv[]) {

	if (argc < 2) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	DIR *root_dir = opendir(argv[1]);
	if (root_dir == NULL) {
		error(EXIT_FAILURE, errno, "Could not open root directory '%s'", argv[1]);
	}

	// Ensure trailing '/'
	strcpy(root, argv[1]);
	int len = strlen(root);
	if (root[len-1] != '/') {
		root[len] = '/';
		root[len+1] = '\0';
	}

	int rc = library_open();
	if (rc) {
		error(EXIT_FAILURE, rc, "Could not open library");
	}
	library_clear();

	walk(root);

	if ( closedir(root_dir) ) {
		error(0, errno, "Could not close root directory");
	}

	rc = library_close();
	if (rc) {
		error(0, rc, "Could not close library");
	}

	printf("Processed %d files from %d dirs\n", num_files, num_dirs);
	return EXIT_SUCCESS;

}

void walk(char *dir_name) {
	DIR *dir = opendir(dir_name);
	if (dir == NULL) {
		error(0, errno, "Could not open directory '%s'", dir_name);
		return;
	}

	char current_filename[256] = {0};
	char ext[16] = {0};

	struct dirent *dirent = NULL;
	while ( (dirent = readdir(dir)) != NULL) {
		// Skip current/parent folder and hidden ones
		if (dirent->d_name[0] == '.') {
			continue;
		}

		sprintf(current_filename, "%s%s", dir_name, dirent->d_name);

		struct stat file_stat;;
		if (stat(current_filename, &file_stat) ) {
			error(0, errno, "Could not stat file '%s'", current_filename);
			continue;
		}

		if (S_ISDIR(file_stat.st_mode)) {
			num_dirs++;
			// Recurse
			int len = strlen(current_filename);
			current_filename[len] = '/';
			current_filename[len+1] = '\0';
			walk(current_filename);
		} else if (S_ISREG(file_stat.st_mode)) {
			// Find extension
			int dot = strlen(dirent->d_name);
			for(; dirent->d_name[dot] != '.' && dot > 0; dot--);
			if (dot > 0) {
				strncpy(ext, dirent->d_name+dot+1, strlen(dirent->d_name)-dot+1);
				if (! strcmp(ext, "ogg")) {
					num_files++;
					song_t *song = song_create();
					fflush(stdout);

					parse_metadata(current_filename, song);
					printf("Adding %s:\n", current_filename);
					printf("\tTitle: %s\n", song->title);
					printf("\tAlbum: %s\n", song->album);
					printf("\tArtist: %s\n", song->artist);
					printf("\tTrack: %d\n", song->track);

					library_add_song(song);

					song_free(song);
				}
			} else {
				printf("Could not find extension for file '%s'\n", current_filename);
			}
		}
	}


	if ( closedir(dir) ) {
		error(0, errno, "Could not close directory '%s'", dir_name);
	}

	return;

}

void usage(char *program_name) {
	printf("%s </path/to/media>\n", program_name);
	printf("\tpath/to/media:    Path to the root folder of the media library\n");
}
