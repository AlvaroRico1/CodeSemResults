yaml_string_write_handler(void *data, unsigned char *buffer, size_t size)
{
  yaml_emitter_t *emitter = (yaml_emitter_t *)data;

    if (emitter->output.string.size - *emitter->output.string.size_written
            < size) {
        memcpy(emitter->output.string.buffer
                + *emitter->output.string.size_written,
                buffer,
                emitter->output.string.size
                - *emitter->output.string.size_written);
        *emitter->output.string.size_written = emitter->output.string.size;
        return 0;
    }

    memcpy(emitter->output.string.buffer
            + *emitter->output.string.size_written, buffer, size);
    *emitter->output.string.size_written += size;
    return 1;
}


// Source: api.c
// Lines 420-439
