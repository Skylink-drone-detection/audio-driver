#ifndef WAV_WRITER_H
#define WAV_WRITER_H

#include "libraries.h"

typedef struct {
    FILE *file;
    uint16_t channels;
    uint32_t sample_rate;
    uint16_t bits_per_sample;
    uint32_t data_bytes_written;
} WavWriter;

bool wav_writer_open(WavWriter *writer, const char *path, uint16_t channels,
                     uint32_t sample_rate, uint16_t bits_per_sample);

bool wav_writer_write_interleaved_i16(WavWriter *writer, const int16_t *samples,
                                      size_t frame_count);

bool wav_writer_close(WavWriter *writer);

#endif
