#include "wav_writer.h"

static bool write_u16_le(FILE *file, uint16_t value)
{
    uint8_t b[2] = {(uint8_t)(value & 0xFFU), (uint8_t)((value >> 8) & 0xFFU)};
    return fwrite(b, sizeof(b), 1, file) == 1;
}

static bool write_u32_le(FILE *file, uint32_t value)
{
    uint8_t b[4] = {
        (uint8_t)(value & 0xFFU),
        (uint8_t)((value >> 8) & 0xFFU),
        (uint8_t)((value >> 16) & 0xFFU),
        (uint8_t)((value >> 24) & 0xFFU),
    };
    return fwrite(b, sizeof(b), 1, file) == 1;
}

static bool wav_write_header(FILE *file, uint16_t channels, uint32_t sample_rate,
                             uint16_t bits_per_sample, uint32_t data_size)
{
    const uint16_t block_align = (uint16_t)(channels * (bits_per_sample / 8U));
    const uint32_t byte_rate = sample_rate * (uint32_t)block_align;
    const uint32_t riff_size = 36U + data_size;

    if (fwrite("RIFF", 4, 1, file) != 1) return false;
    if (!write_u32_le(file, riff_size)) return false;
    if (fwrite("WAVE", 4, 1, file) != 1) return false;

    if (fwrite("fmt ", 4, 1, file) != 1) return false;
    if (!write_u32_le(file, 16U)) return false;
    if (!write_u16_le(file, 1U)) return false;
    if (!write_u16_le(file, channels)) return false;
    if (!write_u32_le(file, sample_rate)) return false;
    if (!write_u32_le(file, byte_rate)) return false;
    if (!write_u16_le(file, block_align)) return false;
    if (!write_u16_le(file, bits_per_sample)) return false;

    if (fwrite("data", 4, 1, file) != 1) return false;
    if (!write_u32_le(file, data_size)) return false;

    return true;
}

bool wav_writer_open(WavWriter *writer, const char *path, uint16_t channels,
                     uint32_t sample_rate, uint16_t bits_per_sample)
{
    if (!writer || !path || channels == 0 || sample_rate == 0 ||
        bits_per_sample != 16U) {
        return false;
    }

    memset(writer, 0, sizeof(*writer));
    writer->file = fopen(path, "wb");
    if (!writer->file) {
        perror("fopen wav");
        return false;
    }

    writer->channels = channels;
    writer->sample_rate = sample_rate;
    writer->bits_per_sample = bits_per_sample;
    writer->data_bytes_written = 0;

    if (!wav_write_header(writer->file, channels, sample_rate, bits_per_sample, 0U)) {
        fclose(writer->file);
        writer->file = NULL;
        return false;
    }

    return true;
}

bool wav_writer_write_interleaved_i16(WavWriter *writer, const int16_t *samples,
                                      size_t frame_count)
{
    if (!writer || !writer->file || !samples || frame_count == 0) return false;

    const size_t values = frame_count * writer->channels;
    const size_t written = fwrite(samples, sizeof(int16_t), values, writer->file);
    if (written != values) return false;

    writer->data_bytes_written += (uint32_t)(values * sizeof(int16_t));
    return true;
}

bool wav_writer_close(WavWriter *writer)
{
    if (!writer || !writer->file) return true;

    if (fseek(writer->file, 0L, SEEK_SET) != 0) {
        fclose(writer->file);
        writer->file = NULL;
        return false;
    }

    const bool ok = wav_write_header(writer->file, writer->channels, writer->sample_rate,
                                     writer->bits_per_sample,
                                     writer->data_bytes_written);
    const int close_status = fclose(writer->file);
    writer->file = NULL;

    return ok && (close_status == 0);
}
