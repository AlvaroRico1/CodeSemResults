Message* Reflection::ReleaseMessage(Message* message,
                                    const FieldDescriptor* field,
                                    MessageFactory* factory) const {
  CheckInvalidAccess(schema_, field);

  Message* released = UnsafeArenaReleaseMessage(message, field, factory);
#ifdef PROTOBUF_FORCE_COPY_IN_RELEASE
  released = MaybeForceCopy(message->GetArenaForAllocation(), released);
#endif  // PROTOBUF_FORCE_COPY_IN_RELEASE
  if (message->GetArenaForAllocation() != nullptr && released != nullptr) {
    Message* copy_from_arena = released->New();
    copy_from_arena->CopyFrom(*released);
    released = copy_from_arena;
  }


// Source: generated_message_reflection.cc
// Lines 2134-2147
