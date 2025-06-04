static void EncodeContextMap(MemoryManager* m,
                             const uint32_t* context_map,
                             size_t context_map_size,
                             size_t num_clusters,
                             HuffmanTree* tree,
                             size_t* storage_ix, uint8_t* storage) {
  size_t i;
  uint32_t* rle_symbols;
  uint32_t max_run_length_prefix = 6;
  size_t num_rle_symbols = 0;
  uint32_t histogram[BROTLI_MAX_CONTEXT_MAP_SYMBOLS];
  static const uint32_t kSymbolMask = (1u << SYMBOL_BITS) - 1u;
  uint8_t depths[BROTLI_MAX_CONTEXT_MAP_SYMBOLS];
  uint16_t bits[BROTLI_MAX_CONTEXT_MAP_SYMBOLS];

  StoreVarLenUint8(num_clusters - 1, storage_ix, storage);

  if (num_clusters == 1) {
    return;
  }

  rle_symbols = BROTLI_ALLOC(m, uint32_t, context_map_size);
  if (BROTLI_IS_OOM(m)) return;
  MoveToFrontTransform(context_map, context_map_size, rle_symbols);
  RunLengthCodeZeros(context_map_size, rle_symbols,
                     &num_rle_symbols, &max_run_length_prefix);
  memset(histogram, 0, sizeof(histogram));
  for (i = 0; i < num_rle_symbols; ++i) {
    ++histogram[rle_symbols[i] & kSymbolMask];
  }
  {
    BROTLI_BOOL use_rle = TO_BROTLI_BOOL(max_run_length_prefix > 0);
    BrotliWriteBits(1, (uint64_t)use_rle, storage_ix, storage);
    if (use_rle) {
      BrotliWriteBits(4, max_run_length_prefix - 1, storage_ix, storage);
    }
  }
  BuildAndStoreHuffmanTree(histogram, num_clusters + max_run_length_prefix,
                           tree, depths, bits, storage_ix, storage);
  for (i = 0; i < num_rle_symbols; ++i) {
    const uint32_t rle_symbol = rle_symbols[i] & kSymbolMask;
    const uint32_t extra_bits_val = rle_symbols[i] >> SYMBOL_BITS;
    BrotliWriteBits(depths[rle_symbol], bits[rle_symbol], storage_ix, storage);
    if (rle_symbol > 0 && rle_symbol <= max_run_length_prefix) {
      BrotliWriteBits(rle_symbol, extra_bits_val, storage_ix, storage);
    }
  }
  BrotliWriteBits(1, 1, storage_ix, storage);  /* use move-to-front */
  BROTLI_FREE(m, rle_symbols);
}

/* Stores the block switch command with index block_ix to the bit stream. */
static BROTLI_INLINE void StoreBlockSwitch(BlockSplitCode* code,
                                           const uint32_t block_len,
                                           const uint8_t block_type,
                                           BROTLI_BOOL is_first_block,
                                           size_t* storage_ix,
                                           uint8_t* storage) {
  size_t typecode = NextBlockTypeCode(&code->type_code_calculator, block_type);
  size_t lencode;
  uint32_t len_nextra;
  uint32_t len_extra;
  if (!is_first_block) {
    BrotliWriteBits(code->type_depths[typecode], code->type_bits[typecode],
                    storage_ix, storage);
  }
  GetBlockLengthPrefixCode(block_len, &lencode, &len_nextra, &len_extra);

  BrotliWriteBits(code->length_depths[lencode], code->length_bits[lencode],
                  storage_ix, storage);
  BrotliWriteBits(len_nextra, len_extra, storage_ix, storage);
}

/* Builds a BlockSplitCode data structure from the block split given by the
   vector of block types and block lengths and stores it to the bit stream. */
static void BuildAndStoreBlockSplitCode(const uint8_t* types,
                                        const uint32_t* lengths,
                                        const size_t num_blocks,
                                        const size_t num_types,
                                        HuffmanTree* tree,
                                        BlockSplitCode* code,
                                        size_t* storage_ix,
                                        uint8_t* storage) {
  uint32_t type_histo[BROTLI_MAX_BLOCK_TYPE_SYMBOLS];
  uint32_t length_histo[BROTLI_NUM_BLOCK_LEN_SYMBOLS];
  size_t i;
  BlockTypeCodeCalculator type_code_calculator;
  memset(type_histo, 0, (num_types + 2) * sizeof(type_histo[0]));
  memset(length_histo, 0, sizeof(length_histo));
  InitBlockTypeCodeCalculator(&type_code_calculator);
  for (i = 0; i < num_blocks; ++i) {
    size_t type_code = NextBlockTypeCode(&type_code_calculator, types[i]);
    if (i != 0) ++type_histo[type_code];
    ++length_histo[BlockLengthPrefixCode(lengths[i])];
  }
  StoreVarLenUint8(num_types - 1, storage_ix, storage);
  if (num_types > 1) {  /* TODO: else? could StoreBlockSwitch occur? */
    BuildAndStoreHuffmanTree(&type_histo[0], num_types + 2, tree,
                             &code->type_depths[0], &code->type_bits[0],
                             storage_ix, storage);
    BuildAndStoreHuffmanTree(&length_histo[0], BROTLI_NUM_BLOCK_LEN_SYMBOLS,
                             tree, &code->length_depths[0],
                             &code->length_bits[0], storage_ix, storage);
    StoreBlockSwitch(code, lengths[0], types[0], 1, storage_ix, storage);
  }
}

/* Stores a context map where the histogram type is always the block type. */
static void StoreTrivialContextMap(size_t num_types,
                                   size_t context_bits,
                                   HuffmanTree* tree,
                                   size_t* storage_ix,
                                   uint8_t* storage) {
  StoreVarLenUint8(num_types - 1, storage_ix, storage);
  if (num_types > 1) {
    size_t repeat_code = context_bits - 1u;
    size_t repeat_bits = (1u << repeat_code) - 1u;
    size_t alphabet_size = num_types + repeat_code;
    uint32_t histogram[BROTLI_MAX_CONTEXT_MAP_SYMBOLS];
    uint8_t depths[BROTLI_MAX_CONTEXT_MAP_SYMBOLS];
    uint16_t bits[BROTLI_MAX_CONTEXT_MAP_SYMBOLS];
    size_t i;
    memset(histogram, 0, alphabet_size * sizeof(histogram[0]));
    /* Write RLEMAX. */
    BrotliWriteBits(1, 1, storage_ix, storage);
    BrotliWriteBits(4, repeat_code - 1, storage_ix, storage);
    histogram[repeat_code] = (uint32_t)num_types;
    histogram[0] = 1;
    for (i = context_bits; i < alphabet_size; ++i) {
      histogram[i] = 1;
    }
    BuildAndStoreHuffmanTree(histogram, alphabet_size, tree,
                             depths, bits, storage_ix, storage);
    for (i = 0; i < num_types; ++i) {
      size_t code = (i == 0 ? 0 : i + context_bits - 1);
      BrotliWriteBits(depths[code], bits[code], storage_ix, storage);
      BrotliWriteBits(
          depths[repeat_code], bits[repeat_code], storage_ix, storage);
      BrotliWriteBits(repeat_code, repeat_bits, storage_ix, storage);
    }
    /* Write IMTF (inverse-move-to-front) bit. */
    BrotliWriteBits(1, 1, storage_ix, storage);
  }
}

/* Manages the encoding of one block category (literal, command or distance). */
typedef struct BlockEncoder {
  size_t alphabet_size_;
  size_t num_block_types_;
  const uint8_t* block_types_;  /* Not owned. */
  const uint32_t* block_lengths_;  /* Not owned. */
  size_t num_blocks_;
  BlockSplitCode block_split_code_;
  size_t block_ix_;
  size_t block_len_;
  size_t entropy_ix_;
  uint8_t* depths_;
  uint16_t* bits_;
} BlockEncoder;

static void InitBlockEncoder(BlockEncoder* self, size_t alphabet_size,
    size_t num_block_types, const uint8_t* block_types,
    const uint32_t* block_lengths, const size_t num_blocks) {
  self->alphabet_size_ = alphabet_size;
  self->num_block_types_ = num_block_types;
  self->block_types_ = block_types;
  self->block_lengths_ = block_lengths;
  self->num_blocks_ = num_blocks;
  InitBlockTypeCodeCalculator(&self->block_split_code_.type_code_calculator);
  self->block_ix_ = 0;
  self->block_len_ = num_blocks == 0 ? 0 : block_lengths[0];
  self->entropy_ix_ = 0;
  self->depths_ = 0;
  self->bits_ = 0;
}

static void CleanupBlockEncoder(MemoryManager* m, BlockEncoder* self) {
  BROTLI_FREE(m, self->depths_);
  BROTLI_FREE(m, self->bits_);
}

/* Creates entropy codes of block lengths and block types and stores them
   to the bit stream. */
static void BuildAndStoreBlockSwitchEntropyCodes(BlockEncoder* self,
    HuffmanTree* tree, size_t* storage_ix, uint8_t* storage) {
  BuildAndStoreBlockSplitCode(self->block_types_, self->block_lengths_,
      self->num_blocks_, self->num_block_types_, tree, &self->block_split_code_,
      storage_ix, storage);
}

/* Stores the next symbol with the entropy code of the current block type.
   Updates the block type and block length at block boundaries. */
static void StoreSymbol(BlockEncoder* self, size_t symbol, size_t* storage_ix,
    uint8_t* storage) {
  if (self->block_len_ == 0) {
    size_t block_ix = ++self->block_ix_;
    uint32_t block_len = self->block_lengths_[block_ix];
    uint8_t block_type = self->block_types_[block_ix];
    self->block_len_ = block_len;
    self->entropy_ix_ = block_type * self->alphabet_size_;
    StoreBlockSwitch(&self->block_split_code_, block_len, block_type, 0,
        storage_ix, storage);
  }
  --self->block_len_;
  {
    size_t ix = self->entropy_ix_ + symbol;
    BrotliWriteBits(self->depths_[ix], self->bits_[ix], storage_ix, storage);
  }
}

/* Stores the next symbol with the entropy code of the current block type and
   context value.
   Updates the block type and block length at block boundaries. */
static void StoreSymbolWithContext(BlockEncoder* self, size_t symbol,
    size_t context, const uint32_t* context_map, size_t* storage_ix,
    uint8_t* storage, const size_t context_bits) {
  if (self->block_len_ == 0) {
    size_t block_ix = ++self->block_ix_;
    uint32_t block_len = self->block_lengths_[block_ix];
    uint8_t block_type = self->block_types_[block_ix];
    self->block_len_ = block_len;
    self->entropy_ix_ = (size_t)block_type << context_bits;
    StoreBlockSwitch(&self->block_split_code_, block_len, block_type, 0,
        storage_ix, storage);
  }
  --self->block_len_;
  {
    size_t histo_ix = context_map[self->entropy_ix_ + context];
    size_t ix = histo_ix * self->alphabet_size_ + symbol;
    BrotliWriteBits(self->depths_[ix], self->bits_[ix], storage_ix, storage);
  }
}

#define FN(X) X ## Literal
/* NOLINTNEXTLINE(build/include) */
#include "./block_encoder_inc.h"
#undef FN

#define FN(X) X ## Command
/* NOLINTNEXTLINE(build/include) */
#include "./block_encoder_inc.h"
#undef FN

#define FN(X) X ## Distance
/* NOLINTNEXTLINE(build/include) */
#include "./block_encoder_inc.h"
#undef FN

static void JumpToByteBoundary(size_t* storage_ix, uint8_t* storage) {
  *storage_ix = (*storage_ix + 7u) & ~7u;
  storage[*storage_ix >> 3] = 0;
}

void BrotliStoreMetaBlock(MemoryManager* m,
                          const uint8_t* input,
                          size_t start_pos,
                          size_t length,
                          size_t mask,
                          uint8_t prev_byte,
                          uint8_t prev_byte2,
                          BROTLI_BOOL is_last,
                          uint32_t num_direct_distance_codes,
                          uint32_t distance_postfix_bits,
                          ContextType literal_context_mode,
                          const Command *commands,
                          size_t n_commands,
                          const MetaBlockSplit* mb,
                          size_t *storage_ix,
                          uint8_t *storage) {
  size_t pos = start_pos;
  size_t i;
  size_t num_distance_codes =
      BROTLI_NUM_DISTANCE_SHORT_CODES + num_direct_distance_codes +
      (48u << distance_postfix_bits);
  HuffmanTree* tree;
  BlockEncoder literal_enc;
  BlockEncoder command_enc;
  BlockEncoder distance_enc;

  StoreCompressedMetaBlockHeader(is_last, length, storage_ix, storage);

  tree = BROTLI_ALLOC(m, HuffmanTree, MAX_HUFFMAN_TREE_SIZE);
  if (BROTLI_IS_OOM(m)) return;
  InitBlockEncoder(&literal_enc, 256, mb->literal_split.num_types,
      mb->literal_split.types, mb->literal_split.lengths,
      mb->literal_split.num_blocks);
  InitBlockEncoder(&command_enc, BROTLI_NUM_COMMAND_SYMBOLS,
      mb->command_split.num_types, mb->command_split.types,
      mb->command_split.lengths, mb->command_split.num_blocks);
  InitBlockEncoder(&distance_enc, num_distance_codes,
      mb->distance_split.num_types, mb->distance_split.types,
      mb->distance_split.lengths, mb->distance_split.num_blocks);

  BuildAndStoreBlockSwitchEntropyCodes(&literal_enc, tree, storage_ix, storage);
  BuildAndStoreBlockSwitchEntropyCodes(&command_enc, tree, storage_ix, storage);
  BuildAndStoreBlockSwitchEntropyCodes(
      &distance_enc, tree, storage_ix, storage);

  BrotliWriteBits(2, distance_postfix_bits, storage_ix, storage);
  BrotliWriteBits(4, num_direct_distance_codes >> distance_postfix_bits,
                  storage_ix, storage);
  for (i = 0; i < mb->literal_split.num_types; ++i) {
    BrotliWriteBits(2, literal_context_mode, storage_ix, storage);
  }

  if (mb->literal_context_map_size == 0) {
    StoreTrivialContextMap(mb->literal_histograms_size,
        BROTLI_LITERAL_CONTEXT_BITS, tree, storage_ix, storage);
  } else {
    EncodeContextMap(m,
        mb->literal_context_map, mb->literal_context_map_size,
        mb->literal_histograms_size, tree, storage_ix, storage);
    if (BROTLI_IS_OOM(m)) return;
  }

  if (mb->distance_context_map_size == 0) {
    StoreTrivialContextMap(mb->distance_histograms_size,
        BROTLI_DISTANCE_CONTEXT_BITS, tree, storage_ix, storage);
  } else {
    EncodeContextMap(m,
        mb->distance_context_map, mb->distance_context_map_size,
        mb->distance_histograms_size, tree, storage_ix, storage);
    if (BROTLI_IS_OOM(m)) return;
  }

  BuildAndStoreEntropyCodesLiteral(m, &literal_enc, mb->literal_histograms,
      mb->literal_histograms_size, tree, storage_ix, storage);
  if (BROTLI_IS_OOM(m)) return;
  BuildAndStoreEntropyCodesCommand(m, &command_enc, mb->command_histograms,
      mb->command_histograms_size, tree, storage_ix, storage);
  if (BROTLI_IS_OOM(m)) return;
  BuildAndStoreEntropyCodesDistance(m, &distance_enc, mb->distance_histograms,
      mb->distance_histograms_size, tree, storage_ix, storage);
  if (BROTLI_IS_OOM(m)) return;
  BROTLI_FREE(m, tree);

  for (i = 0; i < n_commands; ++i) {
    const Command cmd = commands[i];
    size_t cmd_code = cmd.cmd_prefix_;
    StoreSymbol(&command_enc, cmd_code, storage_ix, storage);
    StoreCommandExtra(&cmd, storage_ix, storage);
    if (mb->literal_context_map_size == 0) {
      size_t j;
      for (j = cmd.insert_len_; j != 0; --j) {
        StoreSymbol(&literal_enc, input[pos & mask], storage_ix, storage);
        ++pos;
      }
    } else {
      size_t j;
      for (j = cmd.insert_len_; j != 0; --j) {
        size_t context = Context(prev_byte, prev_byte2, literal_context_mode);
        uint8_t literal = input[pos & mask];
        StoreSymbolWithContext(&literal_enc, literal, context,
            mb->literal_context_map, storage_ix, storage,
            BROTLI_LITERAL_CONTEXT_BITS);
        prev_byte2 = prev_byte;
        prev_byte = literal;
        ++pos;
      }
    }
    pos += CommandCopyLen(&cmd);
    if (CommandCopyLen(&cmd)) {
      prev_byte2 = input[(pos - 2) & mask];
      prev_byte = input[(pos - 1) & mask];
      if (cmd.cmd_prefix_ >= 128) {
        size_t dist_code = cmd.dist_prefix_;
        uint32_t distnumextra = cmd.dist_extra_ >> 24;
        uint64_t distextra = cmd.dist_extra_ & 0xffffff;
        if (mb->distance_context_map_size == 0) {
          StoreSymbol(&distance_enc, dist_code, storage_ix, storage);
        } else {
          size_t context = CommandDistanceContext(&cmd);
          StoreSymbolWithContext(&distance_enc, dist_code, context,
              mb->distance_context_map, storage_ix, storage,
              BROTLI_DISTANCE_CONTEXT_BITS);
        }
        BrotliWriteBits(distnumextra, distextra, storage_ix, storage);
      }
    }
  }
  CleanupBlockEncoder(m, &distance_enc);
  CleanupBlockEncoder(m, &command_enc);
  CleanupBlockEncoder(m, &literal_enc);
  if (is_last) {
    JumpToByteBoundary(storage_ix, storage);
  }
}

static void BuildHistograms(const uint8_t* input,
                            size_t start_pos,
                            size_t mask,
                            const Command *commands,
                            size_t n_commands,
                            HistogramLiteral* lit_histo,
                            HistogramCommand* cmd_histo,
                            HistogramDistance* dist_histo) {
  size_t pos = start_pos;
  size_t i;
  for (i = 0; i < n_commands; ++i) {
    const Command cmd = commands[i];
    size_t j;
    HistogramAddCommand(cmd_histo, cmd.cmd_prefix_);
    for (j = cmd.insert_len_; j != 0; --j) {
      HistogramAddLiteral(lit_histo, input[pos & mask]);
      ++pos;
    }
    pos += CommandCopyLen(&cmd);
    if (CommandCopyLen(&cmd) && cmd.cmd_prefix_ >= 128) {
      HistogramAddDistance(dist_histo, cmd.dist_prefix_);
    }
  }
}

static void StoreDataWithHuffmanCodes(const uint8_t* input,
                                      size_t start_pos,
                                      size_t mask,
                                      const Command *commands,
                                      size_t n_commands,
                                      const uint8_t* lit_depth,
                                      const uint16_t* lit_bits,
                                      const uint8_t* cmd_depth,
                                      const uint16_t* cmd_bits,
                                      const uint8_t* dist_depth,
                                      const uint16_t* dist_bits,
                                      size_t* storage_ix,
                                      uint8_t* storage) {
  size_t pos = start_pos;
  size_t i;
  for (i = 0; i < n_commands; ++i) {
    const Command cmd = commands[i];
    const size_t cmd_code = cmd.cmd_prefix_;
    size_t j;
    BrotliWriteBits(
        cmd_depth[cmd_code], cmd_bits[cmd_code], storage_ix, storage);
    StoreCommandExtra(&cmd, storage_ix, storage);
    for (j = cmd.insert_len_; j != 0; --j) {
      const uint8_t literal = input[pos & mask];
      BrotliWriteBits(
          lit_depth[literal], lit_bits[literal], storage_ix, storage);
      ++pos;
    }
    pos += CommandCopyLen(&cmd);
    if (CommandCopyLen(&cmd) && cmd.cmd_prefix_ >= 128) {
      const size_t dist_code = cmd.dist_prefix_;
      const uint32_t distnumextra = cmd.dist_extra_ >> 24;
      const uint32_t distextra = cmd.dist_extra_ & 0xffffff;
      BrotliWriteBits(dist_depth[dist_code], dist_bits[dist_code],
                      storage_ix, storage);
      BrotliWriteBits(distnumextra, distextra, storage_ix, storage);
    }
  }
}

void BrotliStoreMetaBlockTrivial(MemoryManager* m,
                                 const uint8_t* input,
                                 size_t start_pos,
                                 size_t length,
                                 size_t mask,
                                 BROTLI_BOOL is_last,
                                 const Command *commands,
                                 size_t n_commands,
                                 size_t *storage_ix,
                                 uint8_t *storage) {
  HistogramLiteral lit_histo;
  HistogramCommand cmd_histo;
  HistogramDistance dist_histo;
  uint8_t lit_depth[BROTLI_NUM_LITERAL_SYMBOLS];
  uint16_t lit_bits[BROTLI_NUM_LITERAL_SYMBOLS];
  uint8_t cmd_depth[BROTLI_NUM_COMMAND_SYMBOLS];
  uint16_t cmd_bits[BROTLI_NUM_COMMAND_SYMBOLS];
  uint8_t dist_depth[SIMPLE_DISTANCE_ALPHABET_SIZE];
  uint16_t dist_bits[SIMPLE_DISTANCE_ALPHABET_SIZE];
  HuffmanTree* tree;

  StoreCompressedMetaBlockHeader(is_last, length, storage_ix, storage);

  HistogramClearLiteral(&lit_histo);
  HistogramClearCommand(&cmd_histo);
  HistogramClearDistance(&dist_histo);

  BuildHistograms(input, start_pos, mask, commands, n_commands,
                  &lit_histo, &cmd_histo, &dist_histo);

  BrotliWriteBits(13, 0, storage_ix, storage);

  tree = BROTLI_ALLOC(m, HuffmanTree, MAX_HUFFMAN_TREE_SIZE);
  if (BROTLI_IS_OOM(m)) return;
  BuildAndStoreHuffmanTree(lit_histo.data_, BROTLI_NUM_LITERAL_SYMBOLS, tree,
                           lit_depth, lit_bits,
                           storage_ix, storage);
  BuildAndStoreHuffmanTree(cmd_histo.data_, BROTLI_NUM_COMMAND_SYMBOLS, tree,
                           cmd_depth, cmd_bits,
                           storage_ix, storage);
  BuildAndStoreHuffmanTree(dist_histo.data_, SIMPLE_DISTANCE_ALPHABET_SIZE,
                           tree,
                           dist_depth, dist_bits,
                           storage_ix, storage);
  BROTLI_FREE(m, tree);
  StoreDataWithHuffmanCodes(input, start_pos, mask, commands,
                            n_commands, lit_depth, lit_bits,
                            cmd_depth, cmd_bits,
                            dist_depth, dist_bits,
                            storage_ix, storage);
  if (is_last) {
    JumpToByteBoundary(storage_ix, storage);
  }
}

void BrotliStoreMetaBlockFast(MemoryManager* m,
                              const uint8_t* input,
                              size_t start_pos,
                              size_t length,
                              size_t mask,
                              BROTLI_BOOL is_last,
                              const Command *commands,
                              size_t n_commands,
                              size_t *storage_ix,
                              uint8_t *storage) {
  StoreCompressedMetaBlockHeader(is_last, length, storage_ix, storage);

  BrotliWriteBits(13, 0, storage_ix, storage);

  if (n_commands <= 128) {
    uint32_t histogram[BROTLI_NUM_LITERAL_SYMBOLS] = { 0 };
    size_t pos = start_pos;
    size_t num_literals = 0;
    size_t i;
    uint8_t lit_depth[BROTLI_NUM_LITERAL_SYMBOLS];
    uint16_t lit_bits[BROTLI_NUM_LITERAL_SYMBOLS];
    for (i = 0; i < n_commands; ++i) {
      const Command cmd = commands[i];
      size_t j;
      for (j = cmd.insert_len_; j != 0; --j) {
        ++histogram[input[pos & mask]];
        ++pos;
      }
      num_literals += cmd.insert_len_;
      pos += CommandCopyLen(&cmd);
    }
    BrotliBuildAndStoreHuffmanTreeFast(m, histogram, num_literals,
                                       /* max_bits = */ 8,
                                       lit_depth, lit_bits,
                                       storage_ix, storage);
    if (BROTLI_IS_OOM(m)) return;
    StoreStaticCommandHuffmanTree(storage_ix, storage);
    StoreStaticDistanceHuffmanTree(storage_ix, storage);
    StoreDataWithHuffmanCodes(input, start_pos, mask, commands,
                              n_commands, lit_depth, lit_bits,
                              kStaticCommandCodeDepth,
                              kStaticCommandCodeBits,
                              kStaticDistanceCodeDepth,
                              kStaticDistanceCodeBits,
                              storage_ix, storage);
  } else {
    HistogramLiteral lit_histo;
    HistogramCommand cmd_histo;
    HistogramDistance dist_histo;
    uint8_t lit_depth[BROTLI_NUM_LITERAL_SYMBOLS];
    uint16_t lit_bits[BROTLI_NUM_LITERAL_SYMBOLS];
    uint8_t cmd_depth[BROTLI_NUM_COMMAND_SYMBOLS];
    uint16_t cmd_bits[BROTLI_NUM_COMMAND_SYMBOLS];
    uint8_t dist_depth[SIMPLE_DISTANCE_ALPHABET_SIZE];
    uint16_t dist_bits[SIMPLE_DISTANCE_ALPHABET_SIZE];
    HistogramClearLiteral(&lit_histo);
    HistogramClearCommand(&cmd_histo);
    HistogramClearDistance(&dist_histo);
    BuildHistograms(input, start_pos, mask, commands, n_commands,
                    &lit_histo, &cmd_histo, &dist_histo);
    BrotliBuildAndStoreHuffmanTreeFast(m, lit_histo.data_,
                                       lit_histo.total_count_,
                                       /* max_bits = */ 8,
                                       lit_depth, lit_bits,
                                       storage_ix, storage);
    if (BROTLI_IS_OOM(m)) return;
    BrotliBuildAndStoreHuffmanTreeFast(m, cmd_histo.data_,
                                       cmd_histo.total_count_,
                                       /* max_bits = */ 10,
                                       cmd_depth, cmd_bits,
                                       storage_ix, storage);
    if (BROTLI_IS_OOM(m)) return;
    BrotliBuildAndStoreHuffmanTreeFast(m, dist_histo.data_,
                                       dist_histo.total_count_,
                                       /* max_bits = */
                                       SIMPLE_DISTANCE_ALPHABET_BITS,
                                       dist_depth, dist_bits,
                                       storage_ix, storage);
    if (BROTLI_IS_OOM(m)) return;
    StoreDataWithHuffmanCodes(input, start_pos, mask, commands,
                              n_commands, lit_depth, lit_bits,
                              cmd_depth, cmd_bits,
                              dist_depth, dist_bits,
                              storage_ix, storage);
  }

  if (is_last) {
    JumpToByteBoundary(storage_ix, storage);
  }
}

/* This is for storing uncompressed blocks (simple raw storage of
   bytes-as-bytes). */
void BrotliStoreUncompressedMetaBlock(BROTLI_BOOL is_final_block,
                                      const uint8_t * BROTLI_RESTRICT input,
                                      size_t position, size_t mask,
                                      size_t len,
                                      size_t * BROTLI_RESTRICT storage_ix,
                                      uint8_t * BROTLI_RESTRICT storage) {
  size_t masked_pos = position & mask;
  BrotliStoreUncompressedMetaBlockHeader(len, storage_ix, storage);
  JumpToByteBoundary(storage_ix, storage);

  if (masked_pos + len > mask + 1) {
    size_t len1 = mask + 1 - masked_pos;
    memcpy(&storage[*storage_ix >> 3], &input[masked_pos], len1);
    *storage_ix += len1 << 3;
    len -= len1;
    masked_pos = 0;
  }
  memcpy(&storage[*storage_ix >> 3], &input[masked_pos], len);
  *storage_ix += len << 3;

  /* We need to clear the next 4 bytes to continue to be
     compatible with BrotliWriteBits. */
  BrotliWriteBitsPrepareStorage(*storage_ix, storage);

  /* Since the uncompressed block itself may not be the final block, add an
     empty one after this. */
  if (is_final_block) {
    BrotliWriteBits(1, 1, storage_ix, storage);  /* islast */
    BrotliWriteBits(1, 1, storage_ix, storage);  /* isempty */
    JumpToByteBoundary(storage_ix, storage);
  }
}

#if defined(__cplusplus) || defined(c_plusplus)
}  /* extern "C" */


// Source: brotli_bit_stream.c
// Lines 694-1329
