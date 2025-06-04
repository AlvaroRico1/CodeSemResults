UnicodeReplacer* FunctionReplacer::toReplacer() const {
  FunctionReplacer  *nonconst_this = const_cast<FunctionReplacer *>(this);
  UnicodeReplacer *nonconst_base = static_cast<UnicodeReplacer *>(nonconst_this);
  
  return nonconst_base;
}


// Source: funcrepl.cpp
// Lines 70-75
