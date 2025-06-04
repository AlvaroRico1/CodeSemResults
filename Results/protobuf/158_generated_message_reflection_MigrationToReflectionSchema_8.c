ReflectionSchema MigrationToReflectionSchema(
    const Message* const* default_instance, const uint32_t* offsets,
    MigrationSchema migration_schema) {
  ReflectionSchema result;
  result.default_instance_ = *default_instance;
  // First 7 offsets are offsets to the special fields. The following offsets
  // are the proto fields.
  result.offsets_ = offsets + migration_schema.offsets_index + 6;
  result.has_bit_indices_ = offsets + migration_schema.has_bit_indices_index;
  result.has_bits_offset_ = offsets[migration_schema.offsets_index + 0];
  result.metadata_offset_ = offsets[migration_schema.offsets_index + 1];
  result.extensions_offset_ = offsets[migration_schema.offsets_index + 2];
  result.oneof_case_offset_ = offsets[migration_schema.offsets_index + 3];
  result.object_size_ = migration_schema.object_size;
  result.weak_field_map_offset_ = offsets[migration_schema.offsets_index + 4];
  result.inlined_string_donated_offset_ =
      offsets[migration_schema.offsets_index + 5];
  result.inlined_string_indices_ =
      offsets + migration_schema.inlined_string_indices_index;
  return result;
}


// Source: generated_message_reflection.cc
// Lines 2866-2886
