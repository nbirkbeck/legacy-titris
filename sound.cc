#include "sound.h"
#include <SDL2/SDL_audio.h>
#include <iostream>
#include <unordered_map>

SDL_AudioDeviceID audio_dev;

void LazyInit();

struct Sound {
  Uint8* audio_buf = 0;
  Uint32 audio_len = 0;

  void Play() {
    if (!audio_buf) {
      LazyInit();
    }
    SDL_PauseAudioDevice(audio_dev, 0);

    SDL_ClearQueuedAudio(audio_dev);
    SDL_QueueAudio(audio_dev, audio_buf, audio_len);
  }
};

Sound hit_sound;
Sound line_sound;
Sound tetris_sound;
std::unordered_map<std::string, Sound*> sounds = {
                                                  {"sounds/hit.wav", &hit_sound},
                                                  {"sounds/tetris.wav", &tetris_sound},
                                                  {"sounds/line.wav", &line_sound},
};

void PlaySound(const char* filename) {
  if (sounds.count(filename)) {
    sounds[filename]->Play();
  }
}

void LazyInit() {
  SDL_AudioSpec desired_audio_spec = {0};
  desired_audio_spec.freq = 44100;
  desired_audio_spec.channels = 2;
  desired_audio_spec.samples = 2048;
  desired_audio_spec.format = AUDIO_U8;
  desired_audio_spec.callback = 0;

  SDL_AudioSpec audio_spec;
  audio_dev =
      SDL_OpenAudioDevice(nullptr, 0, &desired_audio_spec, &audio_spec, 0);

  
  SDL_LoadWAV("sounds/hit.wav", &audio_spec, &hit_sound.audio_buf,
              &hit_sound.audio_len);
  SDL_LoadWAV("sounds/tetris.wav", &audio_spec, &tetris_sound.audio_buf,
              &tetris_sound.audio_len);
  SDL_LoadWAV("sounds/line.wav", &audio_spec, &line_sound.audio_buf,
              &line_sound.audio_len);
  std::cout << "Done init audio\n";
}
