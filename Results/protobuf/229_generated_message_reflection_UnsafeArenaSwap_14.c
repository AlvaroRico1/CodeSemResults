void Reflection::UnsafeArenaSwap(Message* lhs, Message* rhs) const {
  if (lhs == rhs) return;

  MutableInternalMetadata(lhs)->InternalSwap(MutableInternalMetadata(rhs));

  for (int i = 0; i <= last_non_weak_field_index_; i++) {
    const FieldDescriptor* field = descriptor_->field(i);
    if (schema_.InRealOneof(field)) continue;
    if (schema_.IsFieldStripped(field)) continue;
    UnsafeShallowSwapField(lhs, rhs, field);
  }
  const int oneof_decl_count = descriptor_->oneof_decl_count();
  for (int i = 0; i < oneof_decl_count; i++) {
    const OneofDescriptor* oneof = descriptor_->oneof_decl(i);
    if (!oneof->is_synthetic()) {
      SwapOneofField<true>(lhs, rhs, oneof);
    }
  }

  // Swapping bits need to happen after swapping fields, because the latter may
  // depend on the has bit information.
  if (schema_.HasHasbits()) {
    uint32_t* lhs_has_bits = MutableHasBits(lhs);
    uint32_t* rhs_has_bits = MutableHasBits(rhs);

    int fields_with_has_bits = 0;
    for (int i = 0; i < descriptor_->field_count(); i++) {
      const FieldDescriptor* field = descriptor_->field(i);
      if (field->is_repeated() || schema_.InRealOneof(field)) {
        continue;
      }
      fields_with_has_bits++;
    }

    int has_bits_size = (fields_with_has_bits + 31) / 32;

    for (int i = 0; i < has_bits_size; i++) {
      std::swap(lhs_has_bits[i], rhs_has_bits[i]);
    }
  }

  if (schema_.HasInlinedString()) {
    uint32_t* lhs_donated_array = MutableInlinedStringDonatedArray(lhs);
    uint32_t* rhs_donated_array = MutableInlinedStringDonatedArray(rhs);
    int inlined_string_count = 0;
    for (int i = 0; i < descriptor_->field_count(); i++) {
      const FieldDescriptor* field = descriptor_->field(i);
      if (field->is_extension() || field->is_repeated() ||
          schema_.InRealOneof(field) ||
          field->options().ctype() != FieldOptions::STRING ||
          !IsInlined(field)) {
        continue;
      }
      inlined_string_count++;
    }

    int donated_array_size = inlined_string_count == 0
                                 ? 0
                                 // One extra bit for the arena dtor tracking.
                                 : (inlined_string_count + 1 + 31) / 32;
    GOOGLE_CHECK_EQ((lhs_donated_array[0] & 0x1u) == 0,
             (rhs_donated_array[0] & 0x1u) == 0);
    for (int i = 0; i < donated_array_size; i++) {
      std::swap(lhs_donated_array[i], rhs_donated_array[i]);
    }
  }


// Source: generated_message_reflection.cc
// Lines 1101-1166
