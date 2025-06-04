void ExtensionSet::InternalExtensionMergeFrom(const MessageLite* extendee,
                                              int number,
                                              const Extension& other_extension,
                                              Arena* other_arena) {
  if (other_extension.is_repeated) {
    Extension* extension;
    bool is_new =
        MaybeNewExtension(number, other_extension.descriptor, &extension);
    if (is_new) {
      // Extension did not already exist in set.
      extension->type = other_extension.type;
      extension->is_packed = other_extension.is_packed;
      extension->is_repeated = true;
    } else {
      GOOGLE_DCHECK_EQ(extension->type, other_extension.type);
      GOOGLE_DCHECK_EQ(extension->is_packed, other_extension.is_packed);
      GOOGLE_DCHECK(extension->is_repeated);
    }

    switch (cpp_type(other_extension.type)) {
#define HANDLE_TYPE(UPPERCASE, LOWERCASE, REPEATED_TYPE) \
  case WireFormatLite::CPPTYPE_##UPPERCASE:              \
    if (is_new) {                                        \
      extension->repeated_##LOWERCASE##_value =          \
          Arena::CreateMessage<REPEATED_TYPE>(arena_);   \
    }                                                    \
    extension->repeated_##LOWERCASE##_value->MergeFrom(  \
        *other_extension.repeated_##LOWERCASE##_value);  \
    break;

      HANDLE_TYPE(INT32, int32_t, RepeatedField<int32_t>);
      HANDLE_TYPE(INT64, int64_t, RepeatedField<int64_t>);
      HANDLE_TYPE(UINT32, uint32_t, RepeatedField<uint32_t>);
      HANDLE_TYPE(UINT64, uint64_t, RepeatedField<uint64_t>);
      HANDLE_TYPE(FLOAT, float, RepeatedField<float>);
      HANDLE_TYPE(DOUBLE, double, RepeatedField<double>);
      HANDLE_TYPE(BOOL, bool, RepeatedField<bool>);
      HANDLE_TYPE(ENUM, enum, RepeatedField<int>);
      HANDLE_TYPE(STRING, string, RepeatedPtrField<std::string>);
#undef HANDLE_TYPE

      case WireFormatLite::CPPTYPE_MESSAGE:
        if (is_new) {
          extension->repeated_message_value =
              Arena::CreateMessage<RepeatedPtrField<MessageLite>>(arena_);
        }
        // We can't call RepeatedPtrField<MessageLite>::MergeFrom() because
        // it would attempt to allocate new objects.
        RepeatedPtrField<MessageLite>* other_repeated_message =
            other_extension.repeated_message_value;
        for (int i = 0; i < other_repeated_message->size(); i++) {
          const MessageLite& other_message = other_repeated_message->Get(i);
          MessageLite* target =
              reinterpret_cast<internal::RepeatedPtrFieldBase*>(
                  extension->repeated_message_value)
                  ->AddFromCleared<GenericTypeHandler<MessageLite>>();
          if (target == nullptr) {
            target = other_message.New(arena_);
            extension->repeated_message_value->AddAllocated(target);
          }
          target->CheckTypeAndMergeFrom(other_message);
        }


// Source: extension_set.cc
// Lines 990-1051
