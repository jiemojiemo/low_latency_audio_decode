
//
// Created by William.Hua on 2021/2/5.
//

#include "portaudio.h"
#include "dr_mp3.h"
#include "dr_wav.h"
#include <thread>
#include <iostream>
#include <vector>
using namespace std;
using namespace std::chrono_literals;

#define NUM_SECONDS   (5)
#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (64)

#define TABLE_SIZE   (200)
typedef struct
{
    int num_callback{0};
    double callback_cost_time{0};
    drmp3 decoder;
    vector<float> frames;
    int current_frame = {0};
    int total_frames = {0};
    int num_channel = {0};
}paTestData;

static int patestCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    auto* user_data = (paTestData*)(userData);
    float *out = (float*)outputBuffer;
    unsigned long i;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;

    user_data->num_callback++;

    // read audio from memory
    auto start_time = std::chrono::steady_clock::now();

    auto num_output = std::min(int(framesPerBuffer), user_data->total_frames - user_data->current_frame);
    auto offset = user_data->current_frame * user_data->num_channel;
    std::copy_n(user_data->frames.data() + offset, num_output * user_data->num_channel, out);
    user_data->current_frame += num_output;

    auto end_time = std::chrono::steady_clock::now();
    auto cost_time = std::chrono::duration<double, micro>(end_time - start_time).count();
    user_data->callback_cost_time += cost_time;

    if(num_output == 0){
        return paComplete;
    }
    return paContinue;
}

int main(int argc, char* argv[])
{
    if(argc < 2){
        printf("Usage: main input_audio.mp3\n");
        return -1;
    }

    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;
    paTestData data;
    int i;

    printf("PortAudio Test: output sine wave. SR = %d, BufSize = %d\n", SAMPLE_RATE, FRAMES_PER_BUFFER);

    // open mp3 file
    if(!drmp3_init_file(&data.decoder, argv[1], NULL)){
        printf("Failed to open file\n");
        return -1;
    }


    // decode whole file into memory
    data.num_channel = data.decoder.channels;
    data.total_frames = drmp3_get_pcm_frame_count(&data.decoder);
    data.frames.resize(data.total_frames * data.num_channel);
    drmp3_read_pcm_frames_f32(&data.decoder, data.total_frames, data.frames.data());

    err = Pa_Initialize();
    if( err != paNoError ) goto error;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default output device.\n");
        goto error;
    }
    outputParameters.channelCount = data.decoder.channels;       /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
        &stream,
        NULL, /* no input */
        &outputParameters,
        data.decoder.sampleRate,
        FRAMES_PER_BUFFER,
        paClipOff,      /* we won't output out of range samples so don't bother clipping them */
        patestCallback,
        &data );
    if( err != paNoError ) goto error;

    err = Pa_SetStreamFinishedCallback( stream, NULL );
    if( err != paNoError ) goto error;

    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;

    printf("Play for %d seconds.\n", NUM_SECONDS );
    Pa_Sleep( NUM_SECONDS * 1000 );

    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;

    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;

    Pa_Terminate();
    printf("Test finished.\n");

    drmp3_uninit(&data.decoder);

    printf("Decode avg cost time: %lf micro", data.callback_cost_time / data.num_callback);

    return err;
    error:
    Pa_Terminate();
    fprintf( stderr, "An error occurred while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}