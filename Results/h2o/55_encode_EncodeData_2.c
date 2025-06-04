static BROTLI_BOOL EncodeData(
    BrotliEncoderState* s, const BROTLI_BOOL is_last,
    const BROTLI_BOOL force_flush, size_t* out_size, uint8_t** output) {
  const uint64_t delta = UnprocessedInputSize(s);
  const uint32_t bytes = (uint32_t)delta;
  const uint32_t wrapped_last_processed_pos =
      WrapPosition(s->last_processed_pos_);
  uint8_t* data;
  uint32_t mask;
  MemoryManager* m = &s->memory_manager_;
  const BrotliDictionary* dictionary = BrotliGetDictionary();

  if (!EnsureInitialized(s)) return BROTLI_FALSE;
  data = s->ringbuffer_.buffer_;
  mask = s->ringbuffer_.mask_;

  /* Adding more blocks after "last" block is forbidden. */
  if (s->is_last_block_emitted_) return BROTLI_FALSE;
  if (is_last) s->is_last_block_emitted_ = BROTLI_TRUE;

  if (delta > InputBlockSize(s)) {
    return BROTLI_FALSE;
  }
  if (s->params.quality == FAST_TWO_PASS_COMPRESSION_QUALITY &&
      !s->command_buf_) {
    s->command_buf_ =
        BROTLI_ALLOC(m, uint32_t, kCompressFragmentTwoPassBlockSize);
    s->literal_buf_ =
        BROTLI_ALLOC(m, uint8_t, kCompressFragmentTwoPassBlockSize);
    if (BROTLI_IS_OOM(m)) return BROTLI_FALSE;
  }

  if (s->params.quality == FAST_ONE_PASS_COMPRESSION_QUALITY ||
      s->params.quality == FAST_TWO_PASS_COMPRESSION_QUALITY) {
    uint8_t* storage;
    size_t storage_ix = s->last_byte_bits_;
    size_t table_size;
    int* table;

    if (delta == 0 && !is_last) {
      /* We have no new input data and we don't have to finish the stream, so
         nothing to do. */
      *out_size = 0;
      return BROTLI_TRUE;
    }
    storage = GetBrotliStorage(s, 2 * bytes + 502);
    if (BROTLI_IS_OOM(m)) return BROTLI_FALSE;
    storage[0] = s->last_byte_;
    table = GetHashTable(s, s->params.quality, bytes, &table_size);
    if (BROTLI_IS_OOM(m)) return BROTLI_FALSE;
    if (s->params.quality == FAST_ONE_PASS_COMPRESSION_QUALITY) {
      BrotliCompressFragmentFast(
          m, &data[wrapped_last_processed_pos & mask],
          bytes, is_last,
          table, table_size,
          s->cmd_depths_, s->cmd_bits_,
          &s->cmd_code_numbits_, s->cmd_code_,
          &storage_ix, storage);
      if (BROTLI_IS_OOM(m)) return BROTLI_FALSE;
    } else {
      BrotliCompressFragmentTwoPass(
          m, &data[wrapped_last_processed_pos & mask],
          bytes, is_last,
          s->command_buf_, s->literal_buf_,
          table, table_size,
          &storage_ix, storage);
      if (BROTLI_IS_OOM(m)) return BROTLI_FALSE;
    }
    s->last_byte_ = storage[storage_ix >> 3];
    s->last_byte_bits_ = storage_ix & 7u;
    UpdateLastProcessedPos(s);
    *output = &storage[0];
    *out_size = storage_ix >> 3;
    return BROTLI_TRUE;
  }


// Source: encode.c
// Lines 852-926
