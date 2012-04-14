#ifndef PLAYER_STATE_H_
#define PLAYER_STATE_H_

int player_state_store(const char *album, int track, double song_time_elapsed);
int player_state_restore(char **album, int *track, double *song_time_elapsed);
int player_state_clear();

#endif
