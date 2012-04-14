#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <time.h>

#include <vorbis/vorbisfile.h>
#include <ao/ao.h>

#include "player.h"
#include "player_state.h"
#include "library.h"

int action = ACTION_NONE;
int input_handler_active = 1;

int main(int argc, char *argv[]) {
	pthread_t input_handler_thread;

	if(pthread_create( &input_handler_thread, NULL, input_handler, NULL)) {
		error(EXIT_FAILURE, errno, "Could not create input handler thread\n");
	}

	char score[5] = "-inf\0";

	char *album = NULL;
	int track = 0;
	double elapsed = 0;
	player_state_restore(&album, &track, &elapsed);

	if (library_open()) {
		printf("Could not open library\n");
		return -1;
	}

	if (album == NULL) {
		if (library_random_album(&album)) {
			printf("Unable to select an random album");
			return -1;
		}
		printf("Playing album '%s'\n", album);
	} else {
		sprintf(score, "%d", track);
		printf("Restoring player state: Album '%s', Track %d, Elapsed: %f\n", album, track, elapsed);
	}

	ao_initialize();

	ao_sample_format format;
	memset(&format, 0, sizeof(format));
	format.bits = 16;
	format.channels = 2;
	format.rate = 44100;
	format.byte_format = AO_FMT_BIG;

	ao_device *device = ao_open_live(ao_default_driver_id(), &format, NULL);

	if (device != NULL) {

		while(action != ACTION_QUIT) {
			action = ACTION_NONE;
			song_t *songs = NULL;
			int num_songs = 0;
			library_songs(album, track, -1, &songs, &num_songs);

			if (num_songs > 0) {
				printf("Playing %d songs\n", num_songs);
				fflush(stdout);

				for (int i=0; i<num_songs && (action == ACTION_NONE); i++) {
					printf("Playing track index %d - %d: %s\n", i, songs[i].track, songs[i].path);
					fflush(stdout);
					play(device, songs[i].path, album, songs[i].track, elapsed);
					elapsed = 0;	// Reset for next track

					if (action == ACTION_NEXT_SONG) {
						action = ACTION_NONE;
					} else if (action == ACTION_PREVIOUS_SONG) {
						action = ACTION_NONE;

						i = i-2;	// Account for the for loop incrementing it
						if (i < -1) {
							i = -1;
						}
					}
				}

				for (int i=0; i<num_songs; i++) {
					free(songs[i].album);
					free(songs[i].path);
				}

				track = 1;
				num_songs = 0;
				free(songs);
				free(album);

				if (library_random_album(&album)) {
					printf("Unable to select an random album");
					break;
				}
				printf("Playing album '%s'\n", album);

			} else {
				printf("Didn't find songs to play\n");
				break;
			}
		}

	} else {
		printf("Could not open audio device\n");
	}

	pthread_join(input_handler_thread, NULL);

	ao_close(device);
	ao_shutdown();
	library_close();

	return 0;


}

void play(ao_device *device, const char *path, const char *album, int track, double elapsed) {
	OggVorbis_File vf;
	int rc = ov_fopen(path, &vf);
	if (rc != 0) {
		printf("Could not open '%s': %d\n", path, rc);
		return;
	}

	if (elapsed) {
		rc = ov_time_seek(&vf, elapsed);
		if (rc) {
			printf("Could not seek: %d\n", rc);
		}
	}

	struct timespec sleep_interval = {0};
	struct timespec sleep_remaining = {0};
	sleep_interval.tv_nsec = 250 * 1000000L;

	double lasttime = 0;
	char pcmout[4096];
	int bitstream;
	long ret = 1;
	while(ret && ((action == ACTION_NONE) || (action == ACTION_PAUSE))) {
		while(action == ACTION_PAUSE) {
			if (nanosleep(&sleep_interval, &sleep_remaining)) {
				error(0, errno, "Could not sleep");
			}
		}
		ret = ov_read(&vf, pcmout, sizeof(pcmout), 0, 2, 1, &bitstream);
		// printf("Read %d\n", ret);
		if (ret < 0) {
			printf("Error in stream\n");
		} else if (ret > 0) {
			ao_play(device, pcmout, ret);
			elapsed = ov_time_tell(&vf);
			if (elapsed > (lasttime+2)) {
				player_state_store(album, track, elapsed);
				printf("Storing state, elapsed %f\n", elapsed);
				lasttime = elapsed;
			}
		}
	}

	ov_clear(&vf);
}

void *input_handler(void *ptr) {
	struct input_event ev;

	int fd = open(KB_DEVICE, O_RDONLY);
	if (fd != -1) {
		// int ret = ioctl(fd, EVIOCGBIT(0, EV_MAX));
		// printf("IOCTL ret: %d\n", ret);
		while(input_handler_active) {
			int rd = read(fd, &ev, sizeof(struct input_event));
			if (rd > 0) {
				switch (ev.type) {
				case EV_KEY:
					switch (ev.value) {
					case 0:	// Key up
						switch (ev.code) {
						case KEY_KP6:
							// Next album
							printf("Next album !\n");
							action = ACTION_NEXT_ALBUM;
							break;
						case KEY_KP0:
							action = ACTION_QUIT;
							input_handler_active = 0;
							break;
						case KEY_KP2:
							action = ACTION_NEXT_SONG;
							break;
						case KEY_KP8:
							action = ACTION_PREVIOUS_SONG;
							break;
						case KEY_KPENTER:
							if (action == ACTION_PAUSE) {
								action = ACTION_NONE;
							} else {
								action = ACTION_PAUSE;
							}
							break;
						}
						break;
					}

					break;
				}

			}
		}
		close(fd);
	} else {
		error(0, errno, "Can't open input device %s", KB_DEVICE);
	}

	return NULL;
}
