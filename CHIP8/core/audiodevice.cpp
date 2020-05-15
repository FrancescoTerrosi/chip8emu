#include "audiodevice.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

AudioDevice::AudioDevice()
{

}

AudioDevice::~AudioDevice()
{

}

int AudioDevice::al_check_error(const char * given_label)
{
    ALenum al_error;
    al_error = alGetError();

    if(AL_NO_ERROR != al_error)
    {
        printf("ERROR - %s  (%s)\n", alGetString(al_error), given_label);
        return al_error;
    }
    return 0;
}

void AudioDevice::init()
{
    const char * defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

    openal_output_device  = alcOpenDevice(defname);
    openal_output_context = alcCreateContext(openal_output_device, NULL);
    alcMakeContextCurrent(openal_output_context);

    alGenBuffers(1, & internal_buffer);
    al_check_error("failed call to alGenBuffers");

}

void AudioDevice::close()
{
    ALenum errorCode = 0;

    alSourceStopv(1, & streaming_source[0]);
    alSourcei(streaming_source[0], AL_BUFFER, 0);

    alDeleteSources(1, &streaming_source[0]);
    alDeleteBuffers(16, &streaming_source[0]);
    errorCode = alGetError();
    alcMakeContextCurrent(NULL);
    errorCode = alGetError();
    alcDestroyContext(openal_output_context);
    alcCloseDevice(openal_output_device);
}

void AudioDevice::renderFrequency(float frequency_hz, unsigned int duration_ms, unsigned int sampleRate_hz)
{
    init();

    size_t buflen = (size_t)((duration_ms/1000.0) * sampleRate_hz);
    short* samples = (short*) malloc(sizeof(short) * buflen);
    for(int i = 0; i < buflen; i++)
    {
        samples[i] = 32760 * sin( (2.f * M_PI * frequency_hz)/sampleRate_hz * i );
    }
    alBufferData(internal_buffer, AL_FORMAT_MONO16, samples, buflen, sampleRate_hz);
    al_check_error("populating alBufferData");
    free(samples);
    alGenSources(1, &streaming_source[0]);
    alSourcei(streaming_source[0], AL_BUFFER, internal_buffer);
    alSourcePlay(streaming_source[0]);

    ALenum current_playing_state;
    alGetSourcei(streaming_source[0], AL_SOURCE_STATE, &current_playing_state);
    al_check_error("alGetSourcei AL_SOURCE_STATE");
    while (AL_PLAYING == current_playing_state)
    {
        usleep(1);

        alGetSourcei(streaming_source[0], AL_SOURCE_STATE, &current_playing_state);
        al_check_error("alGetSourcei AL_SOURCE_STATE");
    }

    close();
}
