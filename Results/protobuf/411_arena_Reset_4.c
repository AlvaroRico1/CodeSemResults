uint64_t ThreadSafeArena::Reset() {
  // Have to do this in a first pass, because some of the destructors might
  // refer to memory in other blocks.
  CleanupList();

  // Discard all blocks except the special block (if present).
  size_t space_allocated = 0;
  auto mem = Free(&space_allocated);
  arena_stats_.RecordReset();

  AllocationPolicy* policy = alloc_policy_.get();
  if (policy) {
    auto saved_policy = *policy;
    if (alloc_policy_.is_user_owned_initial_block()) {
      space_allocated += mem.size;
    } else {
      GetDeallocator(alloc_policy_.get(), &space_allocated)(mem);
      mem.ptr = nullptr;
      mem.size = 0;
    }
    ArenaMetricsCollector* collector = saved_policy.metrics_collector;
    if (collector) collector->OnReset(space_allocated);
    InitializeWithPolicy(mem.ptr, mem.size, saved_policy);
  } else {
    GOOGLE_DCHECK(!alloc_policy_.should_record_allocs());
    // Nullptr policy
    if (alloc_policy_.is_user_owned_initial_block()) {
      space_allocated += mem.size;
      InitializeFrom(mem.ptr, mem.size);
    } else {
      GetDeallocator(alloc_policy_.get(), &space_allocated)(mem);
      Init();
    }
  }


// Source: arena.cc
// Lines 373-406
