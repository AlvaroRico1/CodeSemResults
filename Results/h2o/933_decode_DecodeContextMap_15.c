static BrotliDecoderErrorCode DecodeContextMap(uint32_t context_map_size,
                                               uint32_t* num_htrees,
                                               uint8_t** context_map_arg,
                                               BrotliDecoderState* s) {
  BrotliBitReader* br = &s->br;
  BrotliDecoderErrorCode result = BROTLI_DECODER_SUCCESS;

  switch ((int)s->substate_context_map) {
    case BROTLI_STATE_CONTEXT_MAP_NONE:
      result = DecodeVarLenUint8(s, br, num_htrees);
      if (result != BROTLI_DECODER_SUCCESS) {
        return result;
      }
      (*num_htrees)++;
      s->context_index = 0;
      BROTLI_LOG_UINT(context_map_size);
      BROTLI_LOG_UINT(*num_htrees);
      *context_map_arg = (uint8_t*)BROTLI_ALLOC(s, (size_t)context_map_size);
      if (*context_map_arg == 0) {
        return BROTLI_FAILURE(BROTLI_DECODER_ERROR_ALLOC_CONTEXT_MAP);
      }
      if (*num_htrees <= 1) {
        memset(*context_map_arg, 0, (size_t)context_map_size);
        return BROTLI_DECODER_SUCCESS;
      }
      s->substate_context_map = BROTLI_STATE_CONTEXT_MAP_READ_PREFIX;
      /* No break, continue to next state. */
    case BROTLI_STATE_CONTEXT_MAP_READ_PREFIX: {
      uint32_t bits;
      /* In next stage ReadHuffmanCode uses at least 4 bits, so it is safe
         to peek 4 bits ahead. */
      if (!BrotliSafeGetBits(br, 5, &bits)) {
        return BROTLI_DECODER_NEEDS_MORE_INPUT;
      }
      if ((bits & 1) != 0) { /* Use RLE for zeros. */
        s->max_run_length_prefix = (bits >> 1) + 1;
        BrotliDropBits(br, 5);
      } else {
        s->max_run_length_prefix = 0;
        BrotliDropBits(br, 1);
      }
      BROTLI_LOG_UINT(s->max_run_length_prefix);
      s->substate_context_map = BROTLI_STATE_CONTEXT_MAP_HUFFMAN;
      /* No break, continue to next state. */
    }
    case BROTLI_STATE_CONTEXT_MAP_HUFFMAN:
      result = ReadHuffmanCode(*num_htrees + s->max_run_length_prefix,
                               s->context_map_table, NULL, s);
      if (result != BROTLI_DECODER_SUCCESS) return result;
      s->code = 0xFFFF;
      s->substate_context_map = BROTLI_STATE_CONTEXT_MAP_DECODE;
      /* No break, continue to next state. */
    case BROTLI_STATE_CONTEXT_MAP_DECODE: {
      uint32_t context_index = s->context_index;
      uint32_t max_run_length_prefix = s->max_run_length_prefix;
      uint8_t* context_map = *context_map_arg;
      uint32_t code = s->code;
      BROTLI_BOOL skip_preamble = (code != 0xFFFF);
      while (context_index < context_map_size || skip_preamble) {
        if (!skip_preamble) {
          if (!SafeReadSymbol(s->context_map_table, br, &code)) {
            s->code = 0xFFFF;
            s->context_index = context_index;
            return BROTLI_DECODER_NEEDS_MORE_INPUT;
          }
          BROTLI_LOG_UINT(code);

          if (code == 0) {
            context_map[context_index++] = 0;
            continue;
          }
          if (code > max_run_length_prefix) {
            context_map[context_index++] =
                (uint8_t)(code - max_run_length_prefix);
            continue;
          }
        } else {
          skip_preamble = BROTLI_FALSE;
        }
        /* RLE sub-stage. */
        {
          uint32_t reps;
          if (!BrotliSafeReadBits(br, code, &reps)) {
            s->code = code;
            s->context_index = context_index;
            return BROTLI_DECODER_NEEDS_MORE_INPUT;
          }
          reps += 1U << code;
          BROTLI_LOG_UINT(reps);
          if (context_index + reps > context_map_size) {
            return
                BROTLI_FAILURE(BROTLI_DECODER_ERROR_FORMAT_CONTEXT_MAP_REPEAT);
          }
          do {
            context_map[context_index++] = 0;
          } while (--reps);
        }
      }
      /* No break, continue to next state. */
    }
    case BROTLI_STATE_CONTEXT_MAP_TRANSFORM: {
      uint32_t bits;
      if (!BrotliSafeReadBits(br, 1, &bits)) {
        s->substate_context_map = BROTLI_STATE_CONTEXT_MAP_TRANSFORM;
        return BROTLI_DECODER_NEEDS_MORE_INPUT;
      }
      if (bits != 0) {
        InverseMoveToFrontTransform(*context_map_arg, context_map_size, s);
      }
      s->substate_context_map = BROTLI_STATE_CONTEXT_MAP_NONE;
      return BROTLI_DECODER_SUCCESS;
    }
    default:
      return
          BROTLI_FAILURE(BROTLI_DECODER_ERROR_UNREACHABLE);
  }
}

/* Decodes a command or literal and updates block type ring-buffer.
   Reads 3..54 bits. */
static BROTLI_INLINE BROTLI_BOOL DecodeBlockTypeAndLength(
    int safe, BrotliDecoderState* s, int tree_type) {
  uint32_t max_block_type = s->num_block_types[tree_type];
  const HuffmanCode* type_tree = &s->block_type_trees[
      tree_type * BROTLI_HUFFMAN_MAX_SIZE_258];
  const HuffmanCode* len_tree = &s->block_len_trees[
      tree_type * BROTLI_HUFFMAN_MAX_SIZE_26];
  BrotliBitReader* br = &s->br;
  uint32_t* ringbuffer = &s->block_type_rb[tree_type * 2];
  uint32_t block_type;

  /* Read 0..15 + 3..39 bits */
  if (!safe) {
    block_type = ReadSymbol(type_tree, br);
    s->block_length[tree_type] = ReadBlockLength(len_tree, br);
  } else {
    BrotliBitReaderState memento;
    BrotliBitReaderSaveState(br, &memento);
    if (!SafeReadSymbol(type_tree, br, &block_type)) return BROTLI_FALSE;
    if (!SafeReadBlockLength(s, &s->block_length[tree_type], len_tree, br)) {
      s->substate_read_block_length = BROTLI_STATE_READ_BLOCK_LENGTH_NONE;
      BrotliBitReaderRestoreState(br, &memento);
      return BROTLI_FALSE;
    }
  }

  if (block_type == 1) {
    block_type = ringbuffer[1] + 1;
  } else if (block_type == 0) {
    block_type = ringbuffer[0];
  } else {
    block_type -= 2;
  }
  if (block_type >= max_block_type) {
    block_type -= max_block_type;
  }
  ringbuffer[0] = ringbuffer[1];
  ringbuffer[1] = block_type;
  return BROTLI_TRUE;
}

static BROTLI_INLINE void DetectTrivialLiteralBlockTypes(
    BrotliDecoderState* s) {
  size_t i;
  for (i = 0; i < 8; ++i) s->trivial_literal_contexts[i] = 0;
  for (i = 0; i < s->num_block_types[0]; i++) {
    size_t offset = i << BROTLI_LITERAL_CONTEXT_BITS;
    size_t error = 0;
    size_t sample = s->context_map[offset];
    size_t j;
    for (j = 0; j < (1u << BROTLI_LITERAL_CONTEXT_BITS);) {
      BROTLI_REPEAT(4, error |= s->context_map[offset + j++] ^ sample;)
    }
    if (error == 0) {
      s->trivial_literal_contexts[i >> 5] |= 1u << (i & 31);
    }
  }
}

static BROTLI_INLINE void PrepareLiteralDecoding(BrotliDecoderState* s) {
  uint8_t context_mode;
  size_t trivial;
  uint32_t block_type = s->block_type_rb[1];
  uint32_t context_offset = block_type << BROTLI_LITERAL_CONTEXT_BITS;
  s->context_map_slice = s->context_map + context_offset;
  trivial = s->trivial_literal_contexts[block_type >> 5];
  s->trivial_literal_context = (trivial >> (block_type & 31)) & 1;
  s->literal_htree = s->literal_hgroup.htrees[s->context_map_slice[0]];
  context_mode = s->context_modes[block_type];
  s->context_lookup1 = &kContextLookup[kContextLookupOffsets[context_mode]];
  s->context_lookup2 = &kContextLookup[kContextLookupOffsets[context_mode + 1]];
}

/* Decodes the block type and updates the state for literal context.
   Reads 3..54 bits. */
static BROTLI_INLINE BROTLI_BOOL DecodeLiteralBlockSwitchInternal(
    int safe, BrotliDecoderState* s) {
  if (!DecodeBlockTypeAndLength(safe, s, 0)) {
    return BROTLI_FALSE;
  }
  PrepareLiteralDecoding(s);
  return BROTLI_TRUE;
}

static void BROTLI_NOINLINE DecodeLiteralBlockSwitch(BrotliDecoderState* s) {
  DecodeLiteralBlockSwitchInternal(0, s);
}

static BROTLI_BOOL BROTLI_NOINLINE SafeDecodeLiteralBlockSwitch(
    BrotliDecoderState* s) {
  return DecodeLiteralBlockSwitchInternal(1, s);
}

/* Block switch for insert/copy length.
   Reads 3..54 bits. */
static BROTLI_INLINE BROTLI_BOOL DecodeCommandBlockSwitchInternal(
    int safe, BrotliDecoderState* s) {
  if (!DecodeBlockTypeAndLength(safe, s, 1)) {
    return BROTLI_FALSE;
  }
  s->htree_command = s->insert_copy_hgroup.htrees[s->block_type_rb[3]];
  return BROTLI_TRUE;
}

static void BROTLI_NOINLINE DecodeCommandBlockSwitch(BrotliDecoderState* s) {
  DecodeCommandBlockSwitchInternal(0, s);
}
static BROTLI_BOOL BROTLI_NOINLINE SafeDecodeCommandBlockSwitch(
    BrotliDecoderState* s) {
  return DecodeCommandBlockSwitchInternal(1, s);
}

/* Block switch for distance codes.
   Reads 3..54 bits. */
static BROTLI_INLINE BROTLI_BOOL DecodeDistanceBlockSwitchInternal(
    int safe, BrotliDecoderState* s) {
  if (!DecodeBlockTypeAndLength(safe, s, 2)) {
    return BROTLI_FALSE;
  }
  s->dist_context_map_slice = s->dist_context_map +
      (s->block_type_rb[5] << BROTLI_DISTANCE_CONTEXT_BITS);
  s->dist_htree_index = s->dist_context_map_slice[s->distance_context];
  return BROTLI_TRUE;
}

static void BROTLI_NOINLINE DecodeDistanceBlockSwitch(BrotliDecoderState* s) {
  DecodeDistanceBlockSwitchInternal(0, s);
}

static BROTLI_BOOL BROTLI_NOINLINE SafeDecodeDistanceBlockSwitch(
    BrotliDecoderState* s) {
  return DecodeDistanceBlockSwitchInternal(1, s);
}

static size_t UnwrittenBytes(const BrotliDecoderState* s, BROTLI_BOOL wrap) {
  size_t pos = wrap && s->pos > s->ringbuffer_size ?
      (size_t)s->ringbuffer_size : (size_t)(s->pos);
  size_t partial_pos_rb = (s->rb_roundtrips * (size_t)s->ringbuffer_size) + pos;
  return partial_pos_rb - s->partial_pos_out;
}

/* Dumps output.
   Returns BROTLI_DECODER_NEEDS_MORE_OUTPUT only if there is more output to push
   and either ring-buffer is as big as window size, or |force| is true.
 */
static BrotliDecoderErrorCode BROTLI_NOINLINE WriteRingBuffer(
    BrotliDecoderState* s, size_t* available_out, uint8_t** next_out,
    size_t* total_out, BROTLI_BOOL force) {
  uint8_t* start =
      s->ringbuffer + (s->partial_pos_out & (size_t)s->ringbuffer_mask);
  size_t to_write = UnwrittenBytes(s, BROTLI_TRUE);
  size_t num_written = *available_out;
  if (num_written > to_write) {
    num_written = to_write;
  }
  if (s->meta_block_remaining_len < 0) {
    return BROTLI_FAILURE(BROTLI_DECODER_ERROR_FORMAT_BLOCK_LENGTH_1);
  }
  if (next_out && !*next_out) {
    *next_out = start;
  } else {
    if (next_out) {
      memcpy(*next_out, start, num_written);
      *next_out += num_written;
    }
  }
  *available_out -= num_written;
  BROTLI_LOG_UINT(to_write);
  BROTLI_LOG_UINT(num_written);
  s->partial_pos_out += num_written;
  if (total_out) {
    *total_out = s->partial_pos_out;
  }
  if (num_written < to_write) {
    if (s->ringbuffer_size == (1 << s->window_bits) || force) {
      return BROTLI_DECODER_NEEDS_MORE_OUTPUT;
    } else {
      return BROTLI_DECODER_SUCCESS;
    }
  }
  /* Wrap ring buffer only if it has reached its maximal size. */
  if (s->ringbuffer_size == (1 << s->window_bits) &&
      s->pos >= s->ringbuffer_size) {
    s->pos -= s->ringbuffer_size;
    s->rb_roundtrips++;
    s->should_wrap_ringbuffer = (size_t)s->pos != 0 ? 1 : 0;
  }
  return BROTLI_DECODER_SUCCESS;
}

static void BROTLI_NOINLINE WrapRingBuffer(BrotliDecoderState* s) {
  if (s->should_wrap_ringbuffer) {
    memcpy(s->ringbuffer, s->ringbuffer_end, (size_t)s->pos);
    s->should_wrap_ringbuffer = 0;
  }
}

/* Allocates ring-buffer.

   s->ringbuffer_size MUST be updated by BrotliCalculateRingBufferSize before
   this function is called.

   Last two bytes of ring-buffer are initialized to 0, so context calculation
   could be done uniformly for the first two and all other positions.
*/
static BROTLI_BOOL BROTLI_NOINLINE BrotliEnsureRingBuffer(
    BrotliDecoderState* s) {
  uint8_t* old_ringbuffer = s->ringbuffer;
  if (s->ringbuffer_size == s->new_ringbuffer_size) {
    return BROTLI_TRUE;
  }

  s->ringbuffer = (uint8_t*)BROTLI_ALLOC(s, (size_t)(s->new_ringbuffer_size) +
      kRingBufferWriteAheadSlack);
  if (s->ringbuffer == 0) {
    /* Restore previous value. */
    s->ringbuffer = old_ringbuffer;
    return BROTLI_FALSE;
  }
  s->ringbuffer[s->new_ringbuffer_size - 2] = 0;
  s->ringbuffer[s->new_ringbuffer_size - 1] = 0;

  if (!!old_ringbuffer) {
    memcpy(s->ringbuffer, old_ringbuffer, (size_t)s->pos);
    BROTLI_FREE(s, old_ringbuffer);
  }

  s->ringbuffer_size = s->new_ringbuffer_size;
  s->ringbuffer_mask = s->new_ringbuffer_size - 1;
  s->ringbuffer_end = s->ringbuffer + s->ringbuffer_size;

  return BROTLI_TRUE;
}

static BrotliDecoderErrorCode BROTLI_NOINLINE CopyUncompressedBlockToOutput(
    size_t* available_out, uint8_t** next_out, size_t* total_out,
    BrotliDecoderState* s) {
  /* TODO: avoid allocation for single uncompressed block. */
  if (!BrotliEnsureRingBuffer(s)) {
    return BROTLI_FAILURE(BROTLI_DECODER_ERROR_ALLOC_RING_BUFFER_1);
  }

  /* State machine */
  for (;;) {
    switch (s->substate_uncompressed) {
      case BROTLI_STATE_UNCOMPRESSED_NONE: {
        int nbytes = (int)BrotliGetRemainingBytes(&s->br);
        if (nbytes > s->meta_block_remaining_len) {
          nbytes = s->meta_block_remaining_len;
        }
        if (s->pos + nbytes > s->ringbuffer_size) {
          nbytes = s->ringbuffer_size - s->pos;
        }
        /* Copy remaining bytes from s->br.buf_ to ring-buffer. */
        BrotliCopyBytes(&s->ringbuffer[s->pos], &s->br, (size_t)nbytes);
        s->pos += nbytes;
        s->meta_block_remaining_len -= nbytes;
        if (s->pos < 1 << s->window_bits) {
          if (s->meta_block_remaining_len == 0) {
            return BROTLI_DECODER_SUCCESS;
          }
          return BROTLI_DECODER_NEEDS_MORE_INPUT;
        }
        s->substate_uncompressed = BROTLI_STATE_UNCOMPRESSED_WRITE;
        /* No break, continue to next state */
      }
      case BROTLI_STATE_UNCOMPRESSED_WRITE: {
        BrotliDecoderErrorCode result;
        result = WriteRingBuffer(
            s, available_out, next_out, total_out, BROTLI_FALSE);
        if (result != BROTLI_DECODER_SUCCESS) {
          return result;
        }
        if (s->ringbuffer_size == 1 << s->window_bits) {
          s->max_distance = s->max_backward_distance;
        }
        s->substate_uncompressed = BROTLI_STATE_UNCOMPRESSED_NONE;
        break;
      }
    }
  }
  BROTLI_DCHECK(0);  /* Unreachable */
}

/* Calculates the smallest feasible ring buffer.

   If we know the data size is small, do not allocate more ring buffer
   size than needed to reduce memory usage.

   When this method is called, metablock size and flags MUST be decoded.
*/
static void BROTLI_NOINLINE BrotliCalculateRingBufferSize(
    BrotliDecoderState* s) {
  int window_size = 1 << s->window_bits;
  int new_ringbuffer_size = window_size;
  /* We need at least 2 bytes of ring buffer size to get the last two
     bytes for context from there */
  int min_size = s->ringbuffer_size ? s->ringbuffer_size : 1024;
  int output_size;

  /* If maximum is already reached, no further extension is retired. */
  if (s->ringbuffer_size == window_size) {
    return;
  }

  /* Metadata blocks does not touch ring buffer. */
  if (s->is_metadata) {
    return;
  }

  if (!s->ringbuffer) {
    output_size = 0;
  } else {
    output_size = s->pos;
  }
  output_size += s->meta_block_remaining_len;
  min_size = min_size < output_size ? output_size : min_size;

  if (!!s->canny_ringbuffer_allocation) {
    /* Reduce ring buffer size to save memory when server is unscrupulous.
       In worst case memory usage might be 1.5x bigger for a short period of
       ring buffer reallocation.*/
    while ((new_ringbuffer_size >> 1) >= min_size) {
      new_ringbuffer_size >>= 1;
    }
  }

  s->new_ringbuffer_size = new_ringbuffer_size;
}

/* Reads 1..256 2-bit context modes. */
static BrotliDecoderErrorCode ReadContextModes(BrotliDecoderState* s) {
  BrotliBitReader* br = &s->br;
  int i = s->loop_counter;

  while (i < (int)s->num_block_types[0]) {
    uint32_t bits;
    if (!BrotliSafeReadBits(br, 2, &bits)) {
      s->loop_counter = i;
      return BROTLI_DECODER_NEEDS_MORE_INPUT;
    }
    s->context_modes[i] = (uint8_t)(bits << 1);
    BROTLI_LOG_ARRAY_INDEX(s->context_modes, i);
    i++;
  }
  return BROTLI_DECODER_SUCCESS;
}

static BROTLI_INLINE void TakeDistanceFromRingBuffer(BrotliDecoderState* s) {
  if (s->distance_code == 0) {
    --s->dist_rb_idx;
    s->distance_code = s->dist_rb[s->dist_rb_idx & 3];
    /* Compensate double distance-ring-buffer roll for dictionary items. */
    s->distance_context = 1;
  } else {
    int distance_code = s->distance_code << 1;
    /* kDistanceShortCodeIndexOffset has 2-bit values from LSB: */
    /* 3, 2, 1, 0, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 */
    const uint32_t kDistanceShortCodeIndexOffset = 0xaaafff1b;
    /* kDistanceShortCodeValueOffset has 2-bit values from LSB: */
    /*-0, 0,-0, 0,-1, 1,-2, 2,-3, 3,-1, 1,-2, 2,-3, 3 */
    const uint32_t kDistanceShortCodeValueOffset = 0xfa5fa500;
    int v = (s->dist_rb_idx +
        (int)(kDistanceShortCodeIndexOffset >> distance_code)) & 0x3;
    s->distance_code = s->dist_rb[v];
    v = (int)(kDistanceShortCodeValueOffset >> distance_code) & 0x3;
    if ((distance_code & 0x3) != 0) {
      s->distance_code += v;
    } else {
      s->distance_code -= v;
      if (s->distance_code <= 0) {
        /* A huge distance will cause a BROTLI_FAILURE() soon. */
        /* This is a little faster than failing here. */
        s->distance_code = 0x0fffffff;
      }
    }
  }
}

static BROTLI_INLINE BROTLI_BOOL SafeReadBits(
    BrotliBitReader* const br, uint32_t n_bits, uint32_t* val) {
  if (n_bits != 0) {
    return BrotliSafeReadBits(br, n_bits, val);
  } else {
    *val = 0;
    return BROTLI_TRUE;
  }
}

/* Precondition: s->distance_code < 0 */
static BROTLI_INLINE BROTLI_BOOL ReadDistanceInternal(
    int safe, BrotliDecoderState* s, BrotliBitReader* br) {
  int distval;
  BrotliBitReaderState memento;
  HuffmanCode* distance_tree = s->distance_hgroup.htrees[s->dist_htree_index];
  if (!safe) {
    s->distance_code = (int)ReadSymbol(distance_tree, br);
  } else {
    uint32_t code;
    BrotliBitReaderSaveState(br, &memento);
    if (!SafeReadSymbol(distance_tree, br, &code)) {
      return BROTLI_FALSE;
    }
    s->distance_code = (int)code;
  }
  /* Convert the distance code to the actual distance by possibly */
  /* looking up past distances from the s->ringbuffer. */
  s->distance_context = 0;
  if ((s->distance_code & ~0xf) == 0) {
    TakeDistanceFromRingBuffer(s);
    --s->block_length[2];
    return BROTLI_TRUE;
  }
  distval = s->distance_code - (int)s->num_direct_distance_codes;
  if (distval >= 0) {
    uint32_t nbits;
    int postfix;
    int offset;
    if (!safe && (s->distance_postfix_bits == 0)) {
      nbits = ((uint32_t)distval >> 1) + 1;
      offset = ((2 + (distval & 1)) << nbits) - 4;
      s->distance_code = (int)s->num_direct_distance_codes + offset +
                         (int)BrotliReadBits(br, nbits);
    } else {
      /* This branch also works well when s->distance_postfix_bits == 0 */
      uint32_t bits;
      postfix = distval & s->distance_postfix_mask;
      distval >>= s->distance_postfix_bits;
      nbits = ((uint32_t)distval >> 1) + 1;
      if (safe) {
        if (!SafeReadBits(br, nbits, &bits)) {
          s->distance_code = -1; /* Restore precondition. */
          BrotliBitReaderRestoreState(br, &memento);
          return BROTLI_FALSE;
        }
      } else {
        bits = BrotliReadBits(br, nbits);
      }
      offset = ((2 + (distval & 1)) << nbits) - 4;
      s->distance_code = (int)s->num_direct_distance_codes +
          ((offset + (int)bits) << s->distance_postfix_bits) + postfix;
    }
  }
  s->distance_code = s->distance_code - BROTLI_NUM_DISTANCE_SHORT_CODES + 1;
  --s->block_length[2];
  return BROTLI_TRUE;
}

static BROTLI_INLINE void ReadDistance(
    BrotliDecoderState* s, BrotliBitReader* br) {
  ReadDistanceInternal(0, s, br);
}

static BROTLI_INLINE BROTLI_BOOL SafeReadDistance(
    BrotliDecoderState* s, BrotliBitReader* br) {
  return ReadDistanceInternal(1, s, br);
}

static BROTLI_INLINE BROTLI_BOOL ReadCommandInternal(
    int safe, BrotliDecoderState* s, BrotliBitReader* br, int* insert_length) {
  uint32_t cmd_code;
  uint32_t insert_len_extra = 0;
  uint32_t copy_length;
  CmdLutElement v;
  BrotliBitReaderState memento;
  if (!safe) {
    cmd_code = ReadSymbol(s->htree_command, br);
  } else {
    BrotliBitReaderSaveState(br, &memento);
    if (!SafeReadSymbol(s->htree_command, br, &cmd_code)) {
      return BROTLI_FALSE;
    }
  }
  v = kCmdLut[cmd_code];
  s->distance_code = v.distance_code;
  s->distance_context = v.context;
  s->dist_htree_index = s->dist_context_map_slice[s->distance_context];
  *insert_length = v.insert_len_offset;
  if (!safe) {
    if (BROTLI_PREDICT_FALSE(v.insert_len_extra_bits != 0)) {
      insert_len_extra = BrotliReadBits(br, v.insert_len_extra_bits);
    }
    copy_length = BrotliReadBits(br, v.copy_len_extra_bits);
  } else {
    if (!SafeReadBits(br, v.insert_len_extra_bits, &insert_len_extra) ||
        !SafeReadBits(br, v.copy_len_extra_bits, &copy_length)) {
      BrotliBitReaderRestoreState(br, &memento);
      return BROTLI_FALSE;
    }
  }
  s->copy_length = (int)copy_length + v.copy_len_offset;
  --s->block_length[1];
  *insert_length += (int)insert_len_extra;
  return BROTLI_TRUE;
}

static BROTLI_INLINE void ReadCommand(
    BrotliDecoderState* s, BrotliBitReader* br, int* insert_length) {
  ReadCommandInternal(0, s, br, insert_length);
}

static BROTLI_INLINE BROTLI_BOOL SafeReadCommand(
    BrotliDecoderState* s, BrotliBitReader* br, int* insert_length) {
  return ReadCommandInternal(1, s, br, insert_length);
}

static BROTLI_INLINE BROTLI_BOOL CheckInputAmount(
    int safe, BrotliBitReader* const br, size_t num) {
  if (safe) {
    return BROTLI_TRUE;
  }
  return BrotliCheckInputAmount(br, num);
}

#define BROTLI_SAFE(METHOD)                       \
  {                                               \
    if (safe) {                                   \
      if (!Safe##METHOD) {                        \
        result = BROTLI_DECODER_NEEDS_MORE_INPUT; \
        goto saveStateAndReturn;                  \
      }                                           \
    } else {                                      \
      METHOD;                                     \
    }                                             \
  }

static BROTLI_INLINE BrotliDecoderErrorCode ProcessCommandsInternal(
    int safe, BrotliDecoderState* s) {
  int pos = s->pos;
  int i = s->loop_counter;
  BrotliDecoderErrorCode result = BROTLI_DECODER_SUCCESS;
  BrotliBitReader* br = &s->br;

  if (!CheckInputAmount(safe, br, 28)) {
    result = BROTLI_DECODER_NEEDS_MORE_INPUT;
    goto saveStateAndReturn;
  }
  if (!safe) {
    BROTLI_UNUSED(BrotliWarmupBitReader(br));
  }

  /* Jump into state machine. */
  if (s->state == BROTLI_STATE_COMMAND_BEGIN) {
    goto CommandBegin;
  } else if (s->state == BROTLI_STATE_COMMAND_INNER) {
    goto CommandInner;
  } else if (s->state == BROTLI_STATE_COMMAND_POST_DECODE_LITERALS) {
    goto CommandPostDecodeLiterals;
  } else if (s->state == BROTLI_STATE_COMMAND_POST_WRAP_COPY) {
    goto CommandPostWrapCopy;
  } else {
    return BROTLI_FAILURE(BROTLI_DECODER_ERROR_UNREACHABLE);
  }

CommandBegin:
  if (safe) {
    s->state = BROTLI_STATE_COMMAND_BEGIN;
  }
  if (!CheckInputAmount(safe, br, 28)) { /* 156 bits + 7 bytes */
    s->state = BROTLI_STATE_COMMAND_BEGIN;
    result = BROTLI_DECODER_NEEDS_MORE_INPUT;
    goto saveStateAndReturn;
  }
  if (BROTLI_PREDICT_FALSE(s->block_length[1] == 0)) {
    BROTLI_SAFE(DecodeCommandBlockSwitch(s));
    goto CommandBegin;
  }
  /* Read the insert/copy length in the command */
  BROTLI_SAFE(ReadCommand(s, br, &i));
  BROTLI_LOG(("[ProcessCommandsInternal] pos = %d insert = %d copy = %d\n",
              pos, i, s->copy_length));
  if (i == 0) {
    goto CommandPostDecodeLiterals;
  }
  s->meta_block_remaining_len -= i;

CommandInner:
  if (safe) {
    s->state = BROTLI_STATE_COMMAND_INNER;
  }
  /* Read the literals in the command */
  if (s->trivial_literal_context) {
    uint32_t bits;
    uint32_t value;
    PreloadSymbol(safe, s->literal_htree, br, &bits, &value);
    do {
      if (!CheckInputAmount(safe, br, 28)) { /* 162 bits + 7 bytes */
        s->state = BROTLI_STATE_COMMAND_INNER;
        result = BROTLI_DECODER_NEEDS_MORE_INPUT;
        goto saveStateAndReturn;
      }
      if (BROTLI_PREDICT_FALSE(s->block_length[0] == 0)) {
        BROTLI_SAFE(DecodeLiteralBlockSwitch(s));
        PreloadSymbol(safe, s->literal_htree, br, &bits, &value);
        if (!s->trivial_literal_context) goto CommandInner;
      }
      if (!safe) {
        s->ringbuffer[pos] =
            (uint8_t)ReadPreloadedSymbol(s->literal_htree, br, &bits, &value);
      } else {
        uint32_t literal;
        if (!SafeReadSymbol(s->literal_htree, br, &literal)) {
          result = BROTLI_DECODER_NEEDS_MORE_INPUT;
          goto saveStateAndReturn;
        }
        s->ringbuffer[pos] = (uint8_t)literal;
      }
      --s->block_length[0];
      BROTLI_LOG_ARRAY_INDEX(s->ringbuffer, pos);
      ++pos;
      if (BROTLI_PREDICT_FALSE(pos == s->ringbuffer_size)) {
        s->state = BROTLI_STATE_COMMAND_INNER_WRITE;
        --i;
        goto saveStateAndReturn;
      }
    } while (--i != 0);
  } else {
    uint8_t p1 = s->ringbuffer[(pos - 1) & s->ringbuffer_mask];
    uint8_t p2 = s->ringbuffer[(pos - 2) & s->ringbuffer_mask];
    do {
      const HuffmanCode* hc;
      uint8_t context;
      if (!CheckInputAmount(safe, br, 28)) { /* 162 bits + 7 bytes */
        s->state = BROTLI_STATE_COMMAND_INNER;
        result = BROTLI_DECODER_NEEDS_MORE_INPUT;
        goto saveStateAndReturn;
      }
      if (BROTLI_PREDICT_FALSE(s->block_length[0] == 0)) {
        BROTLI_SAFE(DecodeLiteralBlockSwitch(s));
        if (s->trivial_literal_context) goto CommandInner;
      }
      context = s->context_lookup1[p1] | s->context_lookup2[p2];
      BROTLI_LOG_UINT(context);
      hc = s->literal_hgroup.htrees[s->context_map_slice[context]];
      p2 = p1;
      if (!safe) {
        p1 = (uint8_t)ReadSymbol(hc, br);
      } else {
        uint32_t literal;
        if (!SafeReadSymbol(hc, br, &literal)) {
          result = BROTLI_DECODER_NEEDS_MORE_INPUT;
          goto saveStateAndReturn;
        }
        p1 = (uint8_t)literal;
      }
      s->ringbuffer[pos] = p1;
      --s->block_length[0];
      BROTLI_LOG_UINT(s->context_map_slice[context]);
      BROTLI_LOG_ARRAY_INDEX(s->ringbuffer, pos & s->ringbuffer_mask);
      ++pos;
      if (BROTLI_PREDICT_FALSE(pos == s->ringbuffer_size)) {
        s->state = BROTLI_STATE_COMMAND_INNER_WRITE;
        --i;
        goto saveStateAndReturn;
      }
    } while (--i != 0);
  }
  BROTLI_LOG_UINT(s->meta_block_remaining_len);
  if (BROTLI_PREDICT_FALSE(s->meta_block_remaining_len <= 0)) {
    s->state = BROTLI_STATE_METABLOCK_DONE;
    goto saveStateAndReturn;
  }

CommandPostDecodeLiterals:
  if (safe) {
    s->state = BROTLI_STATE_COMMAND_POST_DECODE_LITERALS;
  }
  if (s->distance_code >= 0) {
    /* Implicit distance case. */
    s->distance_context = s->distance_code ? 0 : 1;
    --s->dist_rb_idx;
    s->distance_code = s->dist_rb[s->dist_rb_idx & 3];
  } else {
    /* Read distance code in the command, unless it was implicitly zero. */
    if (BROTLI_PREDICT_FALSE(s->block_length[2] == 0)) {
      BROTLI_SAFE(DecodeDistanceBlockSwitch(s));
    }
    BROTLI_SAFE(ReadDistance(s, br));
  }
  BROTLI_LOG(("[ProcessCommandsInternal] pos = %d distance = %d\n",
              pos, s->distance_code));
  if (s->max_distance != s->max_backward_distance) {
    s->max_distance =
        (pos < s->max_backward_distance) ? pos : s->max_backward_distance;
  }
  i = s->copy_length;
  /* Apply copy of LZ77 back-reference, or static dictionary reference if
  the distance is larger than the max LZ77 distance */
  if (s->distance_code > s->max_distance) {
    int address = s->distance_code - s->max_distance - 1;
    if (i >= BROTLI_MIN_DICTIONARY_WORD_LENGTH &&
        i <= BROTLI_MAX_DICTIONARY_WORD_LENGTH) {
      int offset = (int)s->dictionary->offsets_by_length[i];
      uint32_t shift = s->dictionary->size_bits_by_length[i];
      int mask = (int)BitMask(shift);
      int word_idx = address & mask;
      int transform_idx = address >> shift;
      /* Compensate double distance-ring-buffer roll. */
      s->dist_rb_idx += s->distance_context;
      offset += word_idx * i;
      if (BROTLI_PREDICT_FALSE(!s->dictionary->data)) {
        return BROTLI_FAILURE(BROTLI_DECODER_ERROR_DICTIONARY_NOT_SET);
      }
      if (transform_idx < kNumTransforms) {
        const uint8_t* word = &s->dictionary->data[offset];
        int len = i;
        if (transform_idx == 0) {
          memcpy(&s->ringbuffer[pos], word, (size_t)len);
          BROTLI_LOG(("[ProcessCommandsInternal] dictionary word: [%.*s]\n",
                      len, word));
        } else {
          len = TransformDictionaryWord(
              &s->ringbuffer[pos], word, len, transform_idx);
          BROTLI_LOG(("[ProcessCommandsInternal] dictionary word: [%.*s],"
                      " transform_idx = %d, transformed: [%.*s]\n",
                      i, word, transform_idx, len, &s->ringbuffer[pos]));
        }
        pos += len;
        s->meta_block_remaining_len -= len;
        if (pos >= s->ringbuffer_size) {
          /*s->partial_pos_rb += (size_t)s->ringbuffer_size;*/
          s->state = BROTLI_STATE_COMMAND_POST_WRITE_1;
          goto saveStateAndReturn;
        }
      } else {
        BROTLI_LOG(("Invalid backward reference. pos: %d distance: %d "
            "len: %d bytes left: %d\n",
            pos, s->distance_code, i, s->meta_block_remaining_len));
        return BROTLI_FAILURE(BROTLI_DECODER_ERROR_FORMAT_TRANSFORM);
      }
    } else {
      BROTLI_LOG(("Invalid backward reference. pos: %d distance: %d "
          "len: %d bytes left: %d\n",
          pos, s->distance_code, i, s->meta_block_remaining_len));
      return BROTLI_FAILURE(BROTLI_DECODER_ERROR_FORMAT_DICTIONARY);
    }
  } else {
    int src_start = (pos - s->distance_code) & s->ringbuffer_mask;
    uint8_t* copy_dst = &s->ringbuffer[pos];
    uint8_t* copy_src = &s->ringbuffer[src_start];
    int dst_end = pos + i;
    int src_end = src_start + i;
    /* update the recent distances cache */
    s->dist_rb[s->dist_rb_idx & 3] = s->distance_code;
    ++s->dist_rb_idx;
    s->meta_block_remaining_len -= i;
    /* There are 32+ bytes of slack in the ring-buffer allocation.
       Also, we have 16 short codes, that make these 16 bytes irrelevant
       in the ring-buffer. Let's copy over them as a first guess.
     */
    memmove16(copy_dst, copy_src);
    if (src_end > pos && dst_end > src_start) {
      /* Regions intersect. */
      goto CommandPostWrapCopy;
    }
    if (dst_end >= s->ringbuffer_size || src_end >= s->ringbuffer_size) {
      /* At least one region wraps. */
      goto CommandPostWrapCopy;
    }
    pos += i;
    if (i > 16) {
      if (i > 32) {
        memcpy(copy_dst + 16, copy_src + 16, (size_t)(i - 16));
      } else {
        /* This branch covers about 45% cases.
           Fixed size short copy allows more compiler optimizations. */
        memmove16(copy_dst + 16, copy_src + 16);
      }
    }
  }
  BROTLI_LOG_UINT(s->meta_block_remaining_len);
  if (s->meta_block_remaining_len <= 0) {
    /* Next metablock, if any */
    s->state = BROTLI_STATE_METABLOCK_DONE;
    goto saveStateAndReturn;
  } else {
    goto CommandBegin;
  }
CommandPostWrapCopy:
  {
    int wrap_guard = s->ringbuffer_size - pos;
    while (--i >= 0) {
      s->ringbuffer[pos] =
          s->ringbuffer[(pos - s->distance_code) & s->ringbuffer_mask];
      ++pos;
      if (BROTLI_PREDICT_FALSE(--wrap_guard == 0)) {
        s->state = BROTLI_STATE_COMMAND_POST_WRITE_2;
        goto saveStateAndReturn;
      }
    }
  }
  if (s->meta_block_remaining_len <= 0) {
    /* Next metablock, if any */
    s->state = BROTLI_STATE_METABLOCK_DONE;
    goto saveStateAndReturn;
  } else {
    goto CommandBegin;
  }

saveStateAndReturn:
  s->pos = pos;
  s->loop_counter = i;
  return result;
}

#undef BROTLI_SAFE

static BROTLI_NOINLINE BrotliDecoderErrorCode ProcessCommands(
    BrotliDecoderState* s) {
  return ProcessCommandsInternal(0, s);
}

static BROTLI_NOINLINE BrotliDecoderErrorCode SafeProcessCommands(
    BrotliDecoderState* s) {
  return ProcessCommandsInternal(1, s);
}

BrotliDecoderResult BrotliDecoderDecompress(
    size_t encoded_size, const uint8_t* encoded_buffer, size_t* decoded_size,
    uint8_t* decoded_buffer) {
  BrotliDecoderState s;
  BrotliDecoderResult result;
  size_t total_out = 0;
  size_t available_in = encoded_size;
  const uint8_t* next_in = encoded_buffer;
  size_t available_out = *decoded_size;
  uint8_t* next_out = decoded_buffer;
  BrotliDecoderStateInit(&s);
  result = BrotliDecoderDecompressStream(
      &s, &available_in, &next_in, &available_out, &next_out, &total_out);
  *decoded_size = total_out;
  BrotliDecoderStateCleanup(&s);
  if (result != BROTLI_DECODER_RESULT_SUCCESS) {
    result = BROTLI_DECODER_RESULT_ERROR;
  }
  return result;
}

/* Invariant: input stream is never overconsumed:
    * invalid input implies that the whole stream is invalid -> any amount of
      input could be read and discarded
    * when result is "needs more input", then at least one more byte is REQUIRED
      to complete decoding; all input data MUST be consumed by decoder, so
      client could swap the input buffer
    * when result is "needs more output" decoder MUST ensure that it doesn't
      hold more than 7 bits in bit reader; this saves client from swapping input
      buffer ahead of time
    * when result is "success" decoder MUST return all unused data back to input
      buffer; this is possible because the invariant is hold on enter
*/
BrotliDecoderResult BrotliDecoderDecompressStream(
    BrotliDecoderState* s, size_t* available_in, const uint8_t** next_in,
    size_t* available_out, uint8_t** next_out, size_t* total_out) {
  BrotliDecoderErrorCode result = BROTLI_DECODER_SUCCESS;
  BrotliBitReader* br = &s->br;
  /* Ensure that *total_out is set, even if no data will ever be pushed out. */
  if (total_out) {
    *total_out = s->partial_pos_out;
  }
  /* Do not try to process further in a case of unrecoverable error. */
  if ((int)s->error_code < 0) {
    return BROTLI_DECODER_RESULT_ERROR;
  }
  if (*available_out && (!next_out || !*next_out)) {
    return SaveErrorCode(
        s, BROTLI_FAILURE(BROTLI_DECODER_ERROR_INVALID_ARGUMENTS));
  }
  if (!*available_out) next_out = 0;
  if (s->buffer_length == 0) { /* Just connect bit reader to input stream. */
    br->avail_in = *available_in;
    br->next_in = *next_in;
  } else {
    /* At least one byte of input is required. More than one byte of input may
       be required to complete the transaction -> reading more data must be
       done in a loop -> do it in a main loop. */
    result = BROTLI_DECODER_NEEDS_MORE_INPUT;
    br->next_in = &s->buffer.u8[0];
  }
  /* State machine */
  for (;;) {
    if (result != BROTLI_DECODER_SUCCESS) { /* Error, needs more input/output */
      if (result == BROTLI_DECODER_NEEDS_MORE_INPUT) {
        if (s->ringbuffer != 0) { /* Pro-actively push output. */
          BrotliDecoderErrorCode intermediate_result = WriteRingBuffer(s,
              available_out, next_out, total_out, BROTLI_TRUE);
          /* WriteRingBuffer checks s->meta_block_remaining_len validity. */
          if ((int)intermediate_result < 0) {
            result = intermediate_result;
            break;
          }
        }
        if (s->buffer_length != 0) { /* Used with internal buffer. */
          if (br->avail_in == 0) { /* Successfully finished read transaction. */
            /* Accumulator contains less than 8 bits, because internal buffer
               is expanded byte-by-byte until it is enough to complete read. */
            s->buffer_length = 0;
            /* Switch to input stream and restart. */
            result = BROTLI_DECODER_SUCCESS;
            br->avail_in = *available_in;
            br->next_in = *next_in;
            continue;
          } else if (*available_in != 0) {
            /* Not enough data in buffer, but can take one more byte from
               input stream. */
            result = BROTLI_DECODER_SUCCESS;
            s->buffer.u8[s->buffer_length] = **next_in;
            s->buffer_length++;
            br->avail_in = s->buffer_length;
            (*next_in)++;
            (*available_in)--;
            /* Retry with more data in buffer. */
            continue;
          }
          /* Can't finish reading and no more input.*/
          break;
        } else { /* Input stream doesn't contain enough input. */
          /* Copy tail to internal buffer and return. */
          *next_in = br->next_in;
          *available_in = br->avail_in;
          while (*available_in) {
            s->buffer.u8[s->buffer_length] = **next_in;
            s->buffer_length++;
            (*next_in)++;
            (*available_in)--;
          }
          break;
        }
        /* Unreachable. */
      }

      /* Fail or needs more output. */

      if (s->buffer_length != 0) {
        /* Just consumed the buffered input and produced some output. Otherwise
           it would result in "needs more input". Reset internal buffer.*/
        s->buffer_length = 0;
      } else {
        /* Using input stream in last iteration. When decoder switches to input
           stream it has less than 8 bits in accumulator, so it is safe to
           return unused accumulator bits there. */
        BrotliBitReaderUnload(br);
        *available_in = br->avail_in;
        *next_in = br->next_in;
      }
      break;
    }
    switch (s->state) {
      case BROTLI_STATE_UNINITED:
        /* Prepare to the first read. */
        if (!BrotliWarmupBitReader(br)) {
          result = BROTLI_DECODER_NEEDS_MORE_INPUT;
          break;
        }
        /* Decode window size. */
        s->window_bits = DecodeWindowBits(br); /* Reads 1..7 bits. */
        BROTLI_LOG_UINT(s->window_bits);
        if (s->window_bits == 9) {
          /* Value 9 is reserved for future use. */
          result = BROTLI_FAILURE(BROTLI_DECODER_ERROR_FORMAT_WINDOW_BITS);
          break;
        }
        /* Maximum distance, see section 9.1. of the spec. */
        s->max_backward_distance = (1 << s->window_bits) - BROTLI_WINDOW_GAP;

        /* Allocate memory for both block_type_trees and block_len_trees. */
        s->block_type_trees = (HuffmanCode*)BROTLI_ALLOC(s,
            sizeof(HuffmanCode) * 3 *
                (BROTLI_HUFFMAN_MAX_SIZE_258 + BROTLI_HUFFMAN_MAX_SIZE_26));
        if (s->block_type_trees == 0) {
          result = BROTLI_FAILURE(BROTLI_DECODER_ERROR_ALLOC_BLOCK_TYPE_TREES);
          break;
        }
        s->block_len_trees =
            s->block_type_trees + 3 * BROTLI_HUFFMAN_MAX_SIZE_258;

        s->state = BROTLI_STATE_METABLOCK_BEGIN;
        /* No break, continue to next state */
      case BROTLI_STATE_METABLOCK_BEGIN:
        BrotliDecoderStateMetablockBegin(s);
        BROTLI_LOG_UINT(s->pos);
        s->state = BROTLI_STATE_METABLOCK_HEADER;
        /* No break, continue to next state */
      case BROTLI_STATE_METABLOCK_HEADER:
        result = DecodeMetaBlockLength(s, br); /* Reads 2 - 31 bits. */
        if (result != BROTLI_DECODER_SUCCESS) {
          break;
        }
        BROTLI_LOG_UINT(s->is_last_metablock);
        BROTLI_LOG_UINT(s->meta_block_remaining_len);
        BROTLI_LOG_UINT(s->is_metadata);
        BROTLI_LOG_UINT(s->is_uncompressed);
        if (s->is_metadata || s->is_uncompressed) {
          if (!BrotliJumpToByteBoundary(br)) {
            result = BROTLI_FAILURE(BROTLI_DECODER_ERROR_FORMAT_PADDING_1);
            break;
          }
        }
        if (s->is_metadata) {
          s->state = BROTLI_STATE_METADATA;
          break;
        }
        if (s->meta_block_remaining_len == 0) {
          s->state = BROTLI_STATE_METABLOCK_DONE;
          break;
        }
        BrotliCalculateRingBufferSize(s);
        if (s->is_uncompressed) {
          s->state = BROTLI_STATE_UNCOMPRESSED;
          break;
        }
        s->loop_counter = 0;
        s->state = BROTLI_STATE_HUFFMAN_CODE_0;
        break;
      case BROTLI_STATE_UNCOMPRESSED: {
        result = CopyUncompressedBlockToOutput(
            available_out, next_out, total_out, s);
        if (result != BROTLI_DECODER_SUCCESS) {
          break;
        }
        s->state = BROTLI_STATE_METABLOCK_DONE;
        break;
      }
      case BROTLI_STATE_METADATA:
        for (; s->meta_block_remaining_len > 0; --s->meta_block_remaining_len) {
          uint32_t bits;
          /* Read one byte and ignore it. */
          if (!BrotliSafeReadBits(br, 8, &bits)) {
            result = BROTLI_DECODER_NEEDS_MORE_INPUT;
            break;
          }
        }
        if (result == BROTLI_DECODER_SUCCESS) {
          s->state = BROTLI_STATE_METABLOCK_DONE;
        }
        break;
      case BROTLI_STATE_HUFFMAN_CODE_0:
        if (s->loop_counter >= 3) {
          s->state = BROTLI_STATE_METABLOCK_HEADER_2;
          break;
        }
        /* Reads 1..11 bits. */
        result = DecodeVarLenUint8(s, br, &s->num_block_types[s->loop_counter]);
        if (result != BROTLI_DECODER_SUCCESS) {
          break;
        }
        s->num_block_types[s->loop_counter]++;
        BROTLI_LOG_UINT(s->num_block_types[s->loop_counter]);
        if (s->num_block_types[s->loop_counter] < 2) {
          s->loop_counter++;
          break;
        }
        s->state = BROTLI_STATE_HUFFMAN_CODE_1;
        /* No break, continue to next state */
      case BROTLI_STATE_HUFFMAN_CODE_1: {
        int tree_offset = s->loop_counter * BROTLI_HUFFMAN_MAX_SIZE_258;
        result = ReadHuffmanCode(s->num_block_types[s->loop_counter] + 2,
            &s->block_type_trees[tree_offset], NULL, s);
        if (result != BROTLI_DECODER_SUCCESS) break;
        s->state = BROTLI_STATE_HUFFMAN_CODE_2;
        /* No break, continue to next state */
      }
      case BROTLI_STATE_HUFFMAN_CODE_2: {
        int tree_offset = s->loop_counter * BROTLI_HUFFMAN_MAX_SIZE_26;
        result = ReadHuffmanCode(BROTLI_NUM_BLOCK_LEN_SYMBOLS,
            &s->block_len_trees[tree_offset], NULL, s);
        if (result != BROTLI_DECODER_SUCCESS) break;
        s->state = BROTLI_STATE_HUFFMAN_CODE_3;
        /* No break, continue to next state */
      }
      case BROTLI_STATE_HUFFMAN_CODE_3: {
        int tree_offset = s->loop_counter * BROTLI_HUFFMAN_MAX_SIZE_26;
        if (!SafeReadBlockLength(s, &s->block_length[s->loop_counter],
            &s->block_len_trees[tree_offset], br)) {
          result = BROTLI_DECODER_NEEDS_MORE_INPUT;
          break;
        }
        BROTLI_LOG_UINT(s->block_length[s->loop_counter]);
        s->loop_counter++;
        s->state = BROTLI_STATE_HUFFMAN_CODE_0;
        break;
      }
      case BROTLI_STATE_METABLOCK_HEADER_2: {
        uint32_t bits;
        if (!BrotliSafeReadBits(br, 6, &bits)) {
          result = BROTLI_DECODER_NEEDS_MORE_INPUT;
          break;
        }
        s->distance_postfix_bits = bits & BitMask(2);
        bits >>= 2;
        s->num_direct_distance_codes = BROTLI_NUM_DISTANCE_SHORT_CODES +
            (bits << s->distance_postfix_bits);
        BROTLI_LOG_UINT(s->num_direct_distance_codes);
        BROTLI_LOG_UINT(s->distance_postfix_bits);
        s->distance_postfix_mask = (int)BitMask(s->distance_postfix_bits);
        s->context_modes =
            (uint8_t*)BROTLI_ALLOC(s, (size_t)s->num_block_types[0]);
        if (s->context_modes == 0) {
          result = BROTLI_FAILURE(BROTLI_DECODER_ERROR_ALLOC_CONTEXT_MODES);
          break;
        }
        s->loop_counter = 0;
        s->state = BROTLI_STATE_CONTEXT_MODES;
        /* No break, continue to next state */
      }
      case BROTLI_STATE_CONTEXT_MODES:
        result = ReadContextModes(s);
        if (result != BROTLI_DECODER_SUCCESS) {
          break;
        }
        s->state = BROTLI_STATE_CONTEXT_MAP_1;
        /* No break, continue to next state */
      case BROTLI_STATE_CONTEXT_MAP_1:
        result = DecodeContextMap(
            s->num_block_types[0] << BROTLI_LITERAL_CONTEXT_BITS,
            &s->num_literal_htrees, &s->context_map, s);
        if (result != BROTLI_DECODER_SUCCESS) {
          break;
        }
        DetectTrivialLiteralBlockTypes(s);
        s->state = BROTLI_STATE_CONTEXT_MAP_2;
        /* No break, continue to next state */
      case BROTLI_STATE_CONTEXT_MAP_2:
        {
          uint32_t num_distance_codes = s->num_direct_distance_codes +
              ((2 * BROTLI_MAX_DISTANCE_BITS) << s->distance_postfix_bits);
          BROTLI_BOOL allocation_success = BROTLI_TRUE;
          result = DecodeContextMap(
              s->num_block_types[2] << BROTLI_DISTANCE_CONTEXT_BITS,
              &s->num_dist_htrees, &s->dist_context_map, s);
          if (result != BROTLI_DECODER_SUCCESS) {
            break;
          }
          allocation_success &= BrotliDecoderHuffmanTreeGroupInit(
              s, &s->literal_hgroup, BROTLI_NUM_LITERAL_SYMBOLS,
              s->num_literal_htrees);
          allocation_success &= BrotliDecoderHuffmanTreeGroupInit(
              s, &s->insert_copy_hgroup, BROTLI_NUM_COMMAND_SYMBOLS,
              s->num_block_types[1]);
          allocation_success &= BrotliDecoderHuffmanTreeGroupInit(
              s, &s->distance_hgroup, num_distance_codes,
              s->num_dist_htrees);
          if (!allocation_success) {
            return SaveErrorCode(s,
                BROTLI_FAILURE(BROTLI_DECODER_ERROR_ALLOC_TREE_GROUPS));
          }
        }
        s->loop_counter = 0;
        s->state = BROTLI_STATE_TREE_GROUP;
        /* No break, continue to next state */
      case BROTLI_STATE_TREE_GROUP:
        {
          HuffmanTreeGroup* hgroup = NULL;
          switch (s->loop_counter) {
            case 0:
              hgroup = &s->literal_hgroup;
              break;
            case 1:
              hgroup = &s->insert_copy_hgroup;
              break;
            case 2:
              hgroup = &s->distance_hgroup;
              break;
            default:
              return SaveErrorCode(s, BROTLI_FAILURE(
                  BROTLI_DECODER_ERROR_UNREACHABLE));
          }
          result = HuffmanTreeGroupDecode(hgroup, s);
        }
        if (result != BROTLI_DECODER_SUCCESS) break;
        s->loop_counter++;
        if (s->loop_counter >= 3) {
          PrepareLiteralDecoding(s);
          s->dist_context_map_slice = s->dist_context_map;
          s->htree_command = s->insert_copy_hgroup.htrees[0];
          if (!BrotliEnsureRingBuffer(s)) {
            result = BROTLI_FAILURE(BROTLI_DECODER_ERROR_ALLOC_RING_BUFFER_2);
            break;
          }
          s->state = BROTLI_STATE_COMMAND_BEGIN;
        }
        break;
      case BROTLI_STATE_COMMAND_BEGIN:
      case BROTLI_STATE_COMMAND_INNER:
      case BROTLI_STATE_COMMAND_POST_DECODE_LITERALS:
      case BROTLI_STATE_COMMAND_POST_WRAP_COPY:
        result = ProcessCommands(s);
        if (result == BROTLI_DECODER_NEEDS_MORE_INPUT) {
          result = SafeProcessCommands(s);
        }
        break;
      case BROTLI_STATE_COMMAND_INNER_WRITE:
      case BROTLI_STATE_COMMAND_POST_WRITE_1:
      case BROTLI_STATE_COMMAND_POST_WRITE_2:
        result = WriteRingBuffer(
            s, available_out, next_out, total_out, BROTLI_FALSE);
        if (result != BROTLI_DECODER_SUCCESS) {
          break;
        }
        WrapRingBuffer(s);
        if (s->ringbuffer_size == 1 << s->window_bits) {
          s->max_distance = s->max_backward_distance;
        }
        if (s->state == BROTLI_STATE_COMMAND_POST_WRITE_1) {
          if (s->meta_block_remaining_len == 0) {
            /* Next metablock, if any */
            s->state = BROTLI_STATE_METABLOCK_DONE;
          } else {
            s->state = BROTLI_STATE_COMMAND_BEGIN;
          }
          break;
        } else if (s->state == BROTLI_STATE_COMMAND_POST_WRITE_2) {
          s->state = BROTLI_STATE_COMMAND_POST_WRAP_COPY;
        } else {  /* BROTLI_STATE_COMMAND_INNER_WRITE */
          if (s->loop_counter == 0) {
            if (s->meta_block_remaining_len == 0) {
              s->state = BROTLI_STATE_METABLOCK_DONE;
            } else {
              s->state = BROTLI_STATE_COMMAND_POST_DECODE_LITERALS;
            }
            break;
          }
          s->state = BROTLI_STATE_COMMAND_INNER;
        }
        break;
      case BROTLI_STATE_METABLOCK_DONE:
        if (s->meta_block_remaining_len < 0) {
          result = BROTLI_FAILURE(BROTLI_DECODER_ERROR_FORMAT_BLOCK_LENGTH_2);
          break;
        }
        BrotliDecoderStateCleanupAfterMetablock(s);
        if (!s->is_last_metablock) {
          s->state = BROTLI_STATE_METABLOCK_BEGIN;
          break;
        }
        if (!BrotliJumpToByteBoundary(br)) {
          result = BROTLI_FAILURE(BROTLI_DECODER_ERROR_FORMAT_PADDING_2);
          break;
        }
        if (s->buffer_length == 0) {
          BrotliBitReaderUnload(br);
          *available_in = br->avail_in;
          *next_in = br->next_in;
        }
        s->state = BROTLI_STATE_DONE;
        /* No break, continue to next state */
      case BROTLI_STATE_DONE:
        if (s->ringbuffer != 0) {
          result = WriteRingBuffer(
              s, available_out, next_out, total_out, BROTLI_TRUE);
          if (result != BROTLI_DECODER_SUCCESS) {
            break;
          }
        }
        return SaveErrorCode(s, result);
    }
  }
  return SaveErrorCode(s, result);
}

BROTLI_BOOL BrotliDecoderHasMoreOutput(const BrotliDecoderState* s) {
  /* After unrecoverable error remaining output is considered nonsensical. */
  if ((int)s->error_code < 0) {
    return BROTLI_FALSE;
  }
  return TO_BROTLI_BOOL(
      s->ringbuffer != 0 && UnwrittenBytes(s, BROTLI_FALSE) != 0);
}

const uint8_t* BrotliDecoderTakeOutput(BrotliDecoderState* s, size_t* size) {
  uint8_t* result = 0;
  size_t available_out = *size ? *size : 1u << 24;
  size_t requested_out = available_out;
  BrotliDecoderErrorCode status;
  if ((s->ringbuffer == 0) || ((int)s->error_code < 0)) {
    *size = 0;
    return 0;
  }
  WrapRingBuffer(s);
  status = WriteRingBuffer(s, &available_out, &result, 0, BROTLI_TRUE);
  /* Either WriteRingBuffer returns those "success" codes... */
  if (status == BROTLI_DECODER_SUCCESS ||
      status == BROTLI_DECODER_NEEDS_MORE_OUTPUT) {
    *size = requested_out - available_out;
  } else {
    /* ... or stream is broken. Normally this should be caught by
       BrotliDecoderDecompressStream, this is just a safeguard. */
    if ((int)status < 0) SaveErrorCode(s, status);
    *size = 0;
    result = 0;
  }
  return result;
}

BROTLI_BOOL BrotliDecoderIsUsed(const BrotliDecoderState* s) {
  return TO_BROTLI_BOOL(s->state != BROTLI_STATE_UNINITED ||
      BrotliGetAvailableBits(&s->br) != 0);
}

BROTLI_BOOL BrotliDecoderIsFinished(const BrotliDecoderState* s) {
  return TO_BROTLI_BOOL(s->state == BROTLI_STATE_DONE) &&
      !BrotliDecoderHasMoreOutput(s);
}

BrotliDecoderErrorCode BrotliDecoderGetErrorCode(const BrotliDecoderState* s) {
  return (BrotliDecoderErrorCode)s->error_code;
}

const char* BrotliDecoderErrorString(BrotliDecoderErrorCode c) {
  switch (c) {
#define BROTLI_ERROR_CODE_CASE_(PREFIX, NAME, CODE) \
    case BROTLI_DECODER ## PREFIX ## NAME: return #NAME;
#define BROTLI_NOTHING_
    BROTLI_DECODER_ERROR_CODES_LIST(BROTLI_ERROR_CODE_CASE_, BROTLI_NOTHING_)
#undef BROTLI_ERROR_CODE_CASE_
#undef BROTLI_NOTHING_
    default: return "INVALID";
  }
}

uint32_t BrotliDecoderVersion() {
  return BROTLI_VERSION;
}

#if defined(__cplusplus) || defined(c_plusplus)
}  /* extern "C" */


// Source: decode.c
// Lines 939-2383
