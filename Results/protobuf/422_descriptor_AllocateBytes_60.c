void* DescriptorPool::Tables::AllocateBytes(int size) {
  if (size == 0) return nullptr;
  void* p = ::operator new(size + RoundUpTo<8>(sizeof(int)));
  int* sizep = static_cast<int*>(p);
  misc_allocs_.emplace_back(sizep);
  *sizep = size;
  return static_cast<char*>(p) + RoundUpTo<8>(sizeof(int));
}


// Source: descriptor.cc
// Lines 1813-1820
