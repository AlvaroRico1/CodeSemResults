void UnknownFieldSetSerializer(const uint8_t* base, uint32_t offset,
                               uint32_t /*tag*/, uint32_t /*has_offset*/,
                               io::CodedOutputStream* output) {
  const void* ptr = base + offset;
  const InternalMetadata* metadata = static_cast<const InternalMetadata*>(ptr);
  if (metadata->have_unknown_fields()) {
    metadata->unknown_fields<UnknownFieldSet>(UnknownFieldSet::default_instance)
        .SerializeToCodedStream(output);
  }
}


// Source: generated_message_reflection.cc
// Lines 3099-3108
