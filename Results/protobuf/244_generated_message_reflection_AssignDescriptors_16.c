void AssignDescriptors(const DescriptorTable* table, bool eager) {
  if (!eager) eager = table->is_eager;
  call_once(*table->once, AssignDescriptorsImpl, table, eager);
}

AddDescriptorsRunner::AddDescriptorsRunner(const DescriptorTable* table) {
  AddDescriptors(table);
}

void RegisterFileLevelMetadata(const DescriptorTable* table) {
  AssignDescriptors(table);
  RegisterAllTypesInternal(table->file_level_metadata, table->num_messages);
}

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

}  // namespace internal


// Source: generated_message_reflection.cc
// Lines 3085-3110
