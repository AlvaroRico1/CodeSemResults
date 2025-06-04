FileDescriptorTables::FindEnumValueByNumberCreatingIfUnknown(
    const EnumDescriptor* parent, int number) const {
  // First try, with map of compiled-in values.
  {
    const auto* value = FindEnumValueByNumber(parent, number);
    if (value != nullptr) {
      return value;
    }
  }

  Symbol::QueryKey query;
  query.parent = parent;
  query.field_number = number;

  // Second try, with reader lock held on unknown enum values: common case.
  {
    ReaderMutexLock l(&unknown_enum_values_mu_);
    auto it = unknown_enum_values_by_number_.find(query);
    if (it != unknown_enum_values_by_number_.end() &&
        it->enum_value_descriptor() != nullptr) {
      return it->enum_value_descriptor();
    }
  }
  // If not found, try again with writer lock held, and create new descriptor if
  // necessary.
  {
    WriterMutexLock l(&unknown_enum_values_mu_);
    auto it = unknown_enum_values_by_number_.find(query);
    if (it != unknown_enum_values_by_number_.end() &&
        it->enum_value_descriptor() != nullptr) {
      return it->enum_value_descriptor();
    }

    // Create an EnumValueDescriptor dynamically. We don't insert it into the
    // EnumDescriptor (it's not a part of the enum as originally defined), but
    // we do insert it into the table so that we can return the same pointer
    // later.
    std::string enum_value_name = StringPrintf(
        "UNKNOWN_ENUM_VALUE_%s_%d", parent->name().c_str(), number);
    auto* pool = DescriptorPool::generated_pool();
    auto* tables = const_cast<DescriptorPool::Tables*>(pool->tables_.get());
    internal::FlatAllocator alloc;
    alloc.PlanArray<EnumValueDescriptor>(1);
    alloc.PlanArray<std::string>(2);

    {
      // Must lock the pool because we will do allocations in the shared arena.
      MutexLockMaybe l2(pool->mutex_);
      alloc.FinalizePlanning(tables);
    }
    EnumValueDescriptor* result = alloc.AllocateArray<EnumValueDescriptor>(1);
    result->all_names_ = alloc.AllocateStrings(
        enum_value_name,
        StrCat(parent->full_name(), ".", enum_value_name));
    result->number_ = number;
    result->type_ = parent;
    result->options_ = &EnumValueOptions::default_instance();
    unknown_enum_values_by_number_.insert(Symbol::EnumValue(result, 0));
    return result;
  }


// Source: descriptor.cc
// Lines 1659-1718
