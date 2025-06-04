Message* Reflection::MutableMessage(Message* message,
                                    const FieldDescriptor* field,
                                    MessageFactory* factory) const {
  USAGE_CHECK_ALL(MutableMessage, SINGULAR, MESSAGE);
  CheckInvalidAccess(schema_, field);

  if (factory == nullptr) factory = message_factory_;

  if (field->is_extension()) {
    return static_cast<Message*>(
        MutableExtensionSet(message)->MutableMessage(field, factory));
  } else {
    Message* result;

    Message** result_holder = MutableRaw<Message*>(message, field);

    if (schema_.InRealOneof(field)) {
      if (!HasOneofField(*message, field)) {
        ClearOneof(message, field->containing_oneof());
        result_holder = MutableField<Message*>(message, field);
        const Message* default_message = GetDefaultMessageInstance(field);
        *result_holder = default_message->New(message->GetArenaForAllocation());
      }
    } else {
      SetBit(message, field);
    }

    if (*result_holder == nullptr) {
      const Message* default_message = GetDefaultMessageInstance(field);
      *result_holder = default_message->New(message->GetArenaForAllocation());
    }
    result = *result_holder;
    return result;
  }


// Source: generated_message_reflection.cc
// Lines 2002-2035
