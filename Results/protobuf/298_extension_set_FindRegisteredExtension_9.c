const ExtensionInfo* FindRegisteredExtension(const MessageLite* extendee,
                                             int number) {
  if (!global_registry) return nullptr;

  ExtensionInfo info;
  info.message = extendee;
  info.number = number;

  auto it = global_registry->find(info);
  if (it == global_registry->end()) {
    return nullptr;
  } else {
    return &*it;
  }
}


// Source: extension_set.cc
// Lines 124-138
