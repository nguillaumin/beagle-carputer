#ifndef PLAYER_H_
#define PLAYER_H_

#ifndef KB_DEVICE
#define KB_DEVICE "/dev/input/event2"
#endif

#define ACTION_NONE 0
#define ACTION_NEXT_ALBUM 1
#define ACTION_QUIT 2
#define ACTION_NEXT_SONG 3
#define ACTION_PREVIOUS_SONG 4
#define ACTION_PAUSE 5

void play(ao_device *device, const char *path, const char *album, int track, double elapsed);
void *input_handler(void *ptr);

#endif
