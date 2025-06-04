void ExtensionSet::GrowCapacity(size_t minimum_new_capacity) {
  if (PROTOBUF_PREDICT_FALSE(is_large())) {
    return;  // LargeMap does not have a "reserve" method.
  }
  if (flat_capacity_ >= minimum_new_capacity) {
    return;
  }

  auto new_flat_capacity = flat_capacity_;
  do {
    new_flat_capacity = new_flat_capacity == 0 ? 1 : new_flat_capacity * 4;
  } while (new_flat_capacity < minimum_new_capacity);

  const KeyValue* begin = flat_begin();
  const KeyValue* end = flat_end();
  AllocatedData new_map;
  if (new_flat_capacity > kMaximumFlatCapacity) {
    new_map.large = Arena::Create<LargeMap>(arena_);
    LargeMap::iterator hint = new_map.large->begin();
    for (const KeyValue* it = begin; it != end; ++it) {
      hint = new_map.large->insert(hint, {it->first, it->second});
    }
    flat_size_ = static_cast<uint16_t>(-1);
    GOOGLE_DCHECK(is_large());
  } else {
    new_map.flat = Arena::CreateArray<KeyValue>(arena_, new_flat_capacity);
    std::copy(begin, end, new_map.flat);
  }

  if (arena_ == nullptr) {
    DeleteFlatMap(begin, flat_capacity_);
  }


// Source: extension_set.cc
// Lines 1686-1717
