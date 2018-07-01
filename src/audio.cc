#include "audio.hh"
#include "journal.hh"

#ifdef OPENAL_BACKEND

#include "openal_backend.inl"

#elif SDL_MIXER_BACKEND

#include "sdl_mixer_backend.inl"

#endif // SDL_MIXER_BACKEND


