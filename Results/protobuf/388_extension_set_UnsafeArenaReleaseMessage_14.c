MessageLite* ExtensionSet::UnsafeArenaReleaseMessage(
    int number, const MessageLite& prototype) {
  Extension* extension = FindOrNull(number);
  if (extension == nullptr) {
    // Not present.  Return nullptr.
    return nullptr;
  } else {
    GOOGLE_DCHECK_TYPE(*extension, OPTIONAL_FIELD, MESSAGE);
    MessageLite* ret = nullptr;
    if (extension->is_lazy) {
      ret = extension->lazymessage_value->UnsafeArenaReleaseMessage(prototype,
                                                                    arena_);
      if (arena_ == nullptr) {
        delete extension->lazymessage_value;
      }
    } else {
      ret = extension->message_value;
    }
    Erase(number);
    return ret;
  }


// Source: extension_set.cc
// Lines 776-796
