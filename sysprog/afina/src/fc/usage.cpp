class StorageSlot {
  int opcode; // put, set, delete, e.t.c
  std::string *key;
  std::string *value;

  bool completed;
  std::exception *ex;

  bool complete() {
    return (completed || ex != nullptr);
  }
}

class StorageFcImpl : public Storage {
public:
    StorageFcImpl() : _combiner(flat_combine) {}

    bool Put(const std::string &key, const std::string &value) {
        StorageSlot *slot = _combiner.get_slot();
        slot.opcode = OpCode::Put;
        slot.key = &key;
        slot.value = &value;

        combiner.apply_slot(*slot);
        if (slot.ex != nullptr) { throw *slot.ex; }

        return (*slot.value == value);
    }

    ....
protected:
    void flat_combine(StorageSlot *begin, StorageSlot *end) override {
        std::Sort(begin, end, key_op_comparator);

        for (Slot *p = begin; p != end; p = p->next) {
            // eliminate as much ops as possible
            // use map methods with a hint to use the fact keys are ordered
            ....
        }
    }

private:
   FlatCombiner<StorageSlot> _combiner;
   std::map<std::string, std::string> _backend;
}
