bool ReflectionOps::IsInitialized(const Message& message, bool check_fields,
                                  bool check_descendants) {
  const Descriptor* descriptor = message.GetDescriptor();
  const Reflection* reflection = GetReflectionOrDie(message);
  if (const int field_count = descriptor->field_count()) {
    const FieldDescriptor* begin = descriptor->field(0);
    const FieldDescriptor* end = begin + field_count;
    GOOGLE_DCHECK_EQ(descriptor->field(field_count - 1), end - 1);

    if (check_fields) {
      // Check required fields of this message.
      for (const FieldDescriptor* field = begin; field != end; ++field) {
        if (field->is_required() && !reflection->HasField(message, field)) {
          return false;
        }
      }
    }


// Source: reflection_ops.cc
// Lines 191-207
