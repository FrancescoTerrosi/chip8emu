#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <AL/al.h>
#include <AL/alc.h>

class AudioDevice
{
  public:
    AudioDevice();
    ~AudioDevice();
    void renderFrequency(float frequency_hz, unsigned int duration_ms, unsigned int sampleRate_hz);

  private:
    int al_check_error(const char* given_label);
    void init();
    void close();

    ALCdevice* openal_output_device;
    ALCcontext* openal_output_context;
    ALuint internal_buffer;
    ALuint streaming_source[1];
};

#endif // AUDIODEVICE_H
