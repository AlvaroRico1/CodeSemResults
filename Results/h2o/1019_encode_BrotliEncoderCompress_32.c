BROTLI_BOOL BrotliEncoderCompress(
    int quality, int lgwin, BrotliEncoderMode mode, size_t input_size,
    const uint8_t* input_buffer, size_t* encoded_size,
    uint8_t* encoded_buffer) {
  BrotliEncoderState* s;
  size_t out_size = *encoded_size;
  const uint8_t* input_start = input_buffer;
  uint8_t* output_start = encoded_buffer;
  size_t max_out_size = BrotliEncoderMaxCompressedSize(input_size);
  if (out_size == 0) {
    /* Output buffer needs at least one byte. */
    return BROTLI_FALSE;
  }
  if (input_size == 0) {
    /* Handle the special case of empty input. */
    *encoded_size = 1;
    *encoded_buffer = 6;
    return BROTLI_TRUE;
  }
  if (quality == 10) {
    /* TODO: Implement this direct path for all quality levels. */
    const int lg_win = BROTLI_MIN(int, BROTLI_MAX_WINDOW_BITS,
                                       BROTLI_MAX(int, 16, lgwin));
    int ok = BrotliCompressBufferQuality10(lg_win, input_size, input_buffer,
                                           encoded_size, encoded_buffer);
    if (!ok || (max_out_size && *encoded_size > max_out_size)) {
      goto fallback;
    }
    return BROTLI_TRUE;
  }

  s = BrotliEncoderCreateInstance(0, 0, 0);
  if (!s) {
    return BROTLI_FALSE;
  } else {
    size_t available_in = input_size;
    const uint8_t* next_in = input_buffer;
    size_t available_out = *encoded_size;
    uint8_t* next_out = encoded_buffer;
    size_t total_out = 0;
    BROTLI_BOOL result = BROTLI_FALSE;
    BrotliEncoderSetParameter(s, BROTLI_PARAM_QUALITY, (uint32_t)quality);
    BrotliEncoderSetParameter(s, BROTLI_PARAM_LGWIN, (uint32_t)lgwin);
    BrotliEncoderSetParameter(s, BROTLI_PARAM_MODE, (uint32_t)mode);
    BrotliEncoderSetParameter(s, BROTLI_PARAM_SIZE_HINT, (uint32_t)input_size);
    result = BrotliEncoderCompressStream(s, BROTLI_OPERATION_FINISH,
        &available_in, &next_in, &available_out, &next_out, &total_out);
    if (!BrotliEncoderIsFinished(s)) result = 0;
    *encoded_size = total_out;
    BrotliEncoderDestroyInstance(s);
    if (!result || (max_out_size && *encoded_size > max_out_size)) {
      goto fallback;
    }
    return BROTLI_TRUE;
  }


// Source: encode.c
// Lines 1338-1392
