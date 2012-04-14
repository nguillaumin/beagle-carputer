# Car-embedded media-player for the BeagleBoard platform

This is the code source of the media player itself, which has two parts:

* `build-library` will walk a file tree of OGG files and load their metadata into Redis.
* `player` that will pop random albums from Redis an play them in a loop.

Please check the [wiki](https://github.com/nguillaumin/beagle-carputer/wiki) for further details and how to build the full system.

Please forgive my clunky C skills, I've not practised them since university.
