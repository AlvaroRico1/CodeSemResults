void ExtensionSet::SetAllocatedMessage(int number, FieldType type,
                                       const FieldDescriptor* descriptor,
                                       MessageLite* message) {
  if (message == nullptr) {
    ClearExtension(number);
    return;
  }
  GOOGLE_DCHECK(message->GetOwningArena() == nullptr ||
         message->GetOwningArena() == arena_);
  Arena* message_arena = message->GetOwningArena();
  Extension* extension;
  if (MaybeNewExtension(number, descriptor, &extension)) {
    extension->type = type;
    GOOGLE_DCHECK_EQ(cpp_type(extension->type), WireFormatLite::CPPTYPE_MESSAGE);
    extension->is_repeated = false;
    extension->is_lazy = false;
    if (message_arena == arena_) {
      extension->message_value = message;
    } else if (message_arena == nullptr) {
      extension->message_value = message;
      arena_->Own(message);  // not nullptr because not equal to message_arena
    } else {
      extension->message_value = message->New(arena_);
      extension->message_value->CheckTypeAndMergeFrom(*message);
    }
  } else {
    GOOGLE_DCHECK_TYPE(*extension, OPTIONAL_FIELD, MESSAGE);
    if (extension->is_lazy) {
      extension->lazymessage_value->SetAllocatedMessage(message, arena_);
    } else {
      if (arena_ == nullptr) {
        delete extension->message_value;
      }
      if (message_arena == arena_) {
        extension->message_value = message;
      } else if (message_arena == nullptr) {
        extension->message_value = message;
        arena_->Own(message);  // not nullptr because not equal to message_arena
      } else {
        extension->message_value = message->New(arena_);
        extension->message_value->CheckTypeAndMergeFrom(*message);
      }
    }
  }
  extension->is_cleared = false;
}


// Source: extension_set.cc
// Lines 671-716
