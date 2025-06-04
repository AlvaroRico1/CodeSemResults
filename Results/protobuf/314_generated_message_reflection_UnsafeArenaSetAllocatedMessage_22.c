void Reflection::UnsafeArenaSetAllocatedMessage(
    Message* message, Message* sub_message,
    const FieldDescriptor* field) const {
  USAGE_CHECK_ALL(SetAllocatedMessage, SINGULAR, MESSAGE);
  CheckInvalidAccess(schema_, field);


  if (field->is_extension()) {
    MutableExtensionSet(message)->UnsafeArenaSetAllocatedMessage(
        field->number(), field->type(), field, sub_message);
  } else {
    if (schema_.InRealOneof(field)) {
      if (sub_message == nullptr) {
        ClearOneof(message, field->containing_oneof());
        return;
      }
        ClearOneof(message, field->containing_oneof());
        *MutableRaw<Message*>(message, field) = sub_message;
      SetOneofCase(message, field);
      return;
    }

    if (sub_message == nullptr) {
      ClearBit(message, field);
    } else {
      SetBit(message, field);
    }
    Message** sub_message_holder = MutableRaw<Message*>(message, field);
    if (message->GetArenaForAllocation() == nullptr) {
      delete *sub_message_holder;
    }
    *sub_message_holder = sub_message;
  }


// Source: generated_message_reflection.cc
// Lines 2038-2070
