Message* Reflection::UnsafeArenaReleaseMessage(Message* message,
                                               const FieldDescriptor* field,
                                               MessageFactory* factory) const {
  USAGE_CHECK_ALL(ReleaseMessage, SINGULAR, MESSAGE);
  CheckInvalidAccess(schema_, field);

  if (factory == nullptr) factory = message_factory_;

  if (field->is_extension()) {
    return static_cast<Message*>(
        MutableExtensionSet(message)->UnsafeArenaReleaseMessage(field,
                                                                factory));
  } else {
    if (!(field->is_repeated() || schema_.InRealOneof(field))) {
      ClearBit(message, field);
    }
    if (schema_.InRealOneof(field)) {
      if (HasOneofField(*message, field)) {
        *MutableOneofCase(message, field->containing_oneof()) = 0;
      } else {
        return nullptr;
      }
    }
    Message** result = MutableRaw<Message*>(message, field);
    Message* ret = *result;
    *result = nullptr;
    return ret;
  }


// Source: generated_message_reflection.cc
// Lines 2104-2131
