
#include <limits>
#include <utility>
#include <memory>
#include <atomic>
#include <thread>
#include <vector>
#include <pthread.h>
#include <functional>
#include <bitset>
/**
 * Create new thread local variable
 */
template <typename T> class ThreadLocal {
public:
    ThreadLocal(T *initial = nullptr, std::function<void(T *)> destructor = nullptr) {
        typedef void* function_t( void* ) ;
        pthread_key_create(&_th_key, nullptr);
        set(initial);
    }

    inline T *get() {
        void* data = pthread_getspecific(_th_key);
        if (data != nullptr){
            return static_cast<T*>(data);
        }
        return nullptr;
    }

    inline void set(T *val) {

        int err = pthread_setspecific(_th_key, val);
        if(err != 0){
            //throw some exception
        }
    }

    T &operator*() { return *get(); }

private:
    pthread_key_t _th_key;

};

template <typename T> class TagedPointer{
public:
    TagedPointer() noexcept : _orig_ptr(1){};


    // cond    -> _orig_ptr
    // nullptr -> 000001 always one ok?
    // tag = 0 -> XXXXX0
    // tag = 1 -> XXXXX1
    TagedPointer(T * ptr, uint64_t tag = 0){

        if(tag>1) {throw std::runtime_error("wrong tag");}
        if(ptr == nullptr){
            _orig_ptr = uint64_t(1);
            return;
        }
        if(tag > 0 ){
            _orig_ptr = (uint64_t)ptr | TAG_MASK;
        }else{
            _orig_ptr = (uint64_t)ptr;
        }
    }

    T* get_ptr(){
        if (_orig_ptr == 1 || _orig_ptr == 0) { return nullptr;}

        return (T*)(_orig_ptr & PTR_MASK );
    }

    uint64_t get_Tag(){
        return (_orig_ptr & TAG_MASK);
    }

    void set_Tag(uint64_t tag){
        if(tag > 1){ throw std::runtime_error("wrong tag"); }
        _orig_ptr &=  PTR_MASK;
        _orig_ptr |= tag;
    }

    void set_ptr(T* p){
        if (p == nullptr){
            _orig_ptr = (getTag() == 1) ? 1UL : 0UL;
            return;
        }
        uint64_t new_ptr = (uint64_t)p;
        _orig_ptr = new_ptr | get_Tag();
        //std::cout << std::bitset<64>(_orig_ptr.load())<<" - setPtr" << '\n';
    }

    T* get_tagged_ptr(){
        return (T*)_orig_ptr;
    }
private:
    static constexpr uint64_t PTR_MASK = ~uint64_t(1); //111110
    static constexpr uint64_t TAG_MASK = uint64_t(1);  //000001
    uint64_t _orig_ptr;
};
/**
 * Create new flat combine synchronizaion primitive
 *
 * @template_param OpNode
 * Class for a single pending operation descriptor. Must provides following API:
 * - complete() returns true is operation gets completed and false otherwise
 * - error(const std::exception &ex) set operation as failed. After this call return,
 *   subsequent calls to complete() method must return true
 *
 * @template_param QMS
 * Maximum array size that could be passed to a single Combine function call
 */
template <typename OpNode, std::size_t QMS = 64> class FlatCombiner {
public:
    // User defined type for the pending operations description, must be plain object without
    // virtual functions
    using pending_operation = OpNode;

    // Function that combine multiple operations and apply it onto data structure
    using combiner = std::function<void(OpNode *, OpNode *)>;

    // Maximum number of pernding operations could be passed to a single Combine call
    static const std::size_t max_call_size = QMS;

    /**
     * @param Combine function that aplly pending operations onto some data structure. It accepts array
     * of pending ops and allowed to modify it in any way except delete pointers
     */
    FlatCombiner(std::function<void(OpNode *, OpNode *)> combine) : _slot(nullptr, orphan_slot), _combine(combine) {
        _dummy_tail = new Slot();
        auto dummy_head = new Slot();
        //auto dummy_head ??
        dummy_head->need_to_process.store(false);
        _dummy_tail->need_to_process.store(false);
        TagedPointer<Slot> tp{nullptr};
        TagedPointer<Slot> tp_head{_dummy_tail,1};
        dummy_head->next_and_alive.store(_dummy_tail);
        _dummy_tail->next_and_alive.store(tp);
        _queue.store(dummy_head);

    }
    ~FlatCombiner() {
     /* dequeue all slot, think about slot deletition */
     //TODO: DELETE DUMMY_HEAD!
     delete _dummy_tail.load()->get_ptr;
    }

    /**
     * Return pending operation slot to the calling thread, object stays valid as long
     * as current thread is alive or got called detach method
     */
    pending_operation *get_slot() {
        Slot *result = _slot.get();
        if (result == nullptr) {
            result = new Slot();
            // TODO: setup usage bit in the pointer
             //TagedPointer<Slot> tSlot{result,1};
             TagedPointer tmp{nullptr};
             result->next_and_alive.store(tmp);
            _slot.set(result);
        }
        return result->user_op;
    }

    /**
     * Put pending operation in the queue and try to execute it. Method gets blocked until
     * slot gets complete, in other words until slot.complete() returns false
     */
    void apply_slot(pending_operation &slot) {
        Slot *slot = reinterpret_cast<Slot *>(((void *)&slot) - containerof(Slot, user_op));
        // TODO: assert slot params
        // TODO: enqueue slot if needs
        // TODO: try to become executor (cquire lock)
        // TODO: scan qeue, dequeue stale nodes, prepare array to be passed
        // to Combine call
        // TODO: call Combine function
        // TODO: unlock
        // TODO: if lock fails, do thread_yeild and goto 3 TODO

        // TODO - need for make publication????
        // gen > 0 only for combiner
        uint64_t gen = try_lock();
        make_publication(slot);
        if( gen > 0 ){
            combining(gen);
            _combine(_combine_shot.begin(), _combine_shot.end());
            unlock();
        }else{
            //make_publication(slot);
            while (slot->need_to_process.load() != false) {
                gen = try_lock();

                if (gen > 0){
                    combining(gen);
                    _combine(_combine_shot.begin(), _combine_shot.end());
                    unlock();
                    break;
                }
            }
        }

    }

    /**
     * Detach calling thread from this flat combiner, in other word
     * destroy thread slot in the queue
     */
    void detach() {
        Slot *result = _slot.get();
        if (result != nullptr) {
            _slot.set(nullptr);
        }
        orphan_slot(result);
    }

protected:
    // Extend user provided pending operation type with fields required for the
    // flat combine algorithm to work
    using Slot = struct Slot {
        // User pending operation to be complete
        OpNode user_op;

        // When last time this slot was detected as been in use
        uint64_t generation;

        std::atomic<bool> need_to_process;

        // Pointer to the next slot. One bit of pointer is stolen to
        // mark if owner thread is still alive, based on this information
        // combiner/thread_local destructor able to take decission about
        // deleting node.
        //
        // So if stolen bit is set then the only reference left to this slot
        // if the queue. If pointer is zero and bit is set then the only ref
        // left is thread_local storage. If next is zero there are no
        // link left and slot could be deleted

        //std::atomic<uint64_t> next_and_alive;
        std::atomic<TagedPointer<Slot>> next_and_alive;

        /**
         * Remove alive bit from the next_and_alive pointer and return
         * only correct pointer to the next slot
         */
        Slot *next() {
            return next_and_alive.load().get_ptr();
        }
    };

    /**
    * all work with publication list
    */
    void combining(uint64_t gen){

        Slot *parent = _queue.load();
        auto cur_slot = parent->next();
        size_t i = 0;
        for (; cur_slot != nullptr || i < QMS; cur_slot = cur_slot->next()){
            if(cur_slot->need_to_process.load(std::memory_order_acquire)){
                _combine_shot[i] = cur_slot;
                cur_slot->generation = gen;
                i++;
            }else{
                if (cur_slot->generation < gen - generation_offset){
                    dequeue_slot(parent, cur_slot);
                    cur_slot = parent;
                }
            }
            parent = cur_slot;
        }
    }

    void make_publication(Slot * slot) {
        if(slot->next() == nullptr){
            // load dummy_head
            // queue doesn't change through the program, i hope...
            Slot* next_the_queue ;
            do{
                // get first slot
                next_the_queue = _queue.load(std::memory_order_relaxed)->next();
                // make it tagged
                TagedPointer t_ptr{next_the_queue};
                // set next field in our own slot
                slot->next_and_alive.store(t_ptr);
                // load our head
                Slot * qu = _queue.load(std::memory_order_relaxed)
            //
        }while(qu->next_and_alive.compare_exchange_weak(slot->next(), slot))

            do{
                tmp = _queue.load()
            }
        }

        slot->need_to_process.store(true, std::memory_order_release);

    }
    /**
     * Try to acquire "lock", in case of success returns current generation. If
     * fails the return 0
     *
     * @param suc memory barier to set in case of success lock
     * @param fail memory barrier to set in case of failure
     */
    uint64_t try_lock(std::memory_order suc, std::memory_order fail) {
        // Похоже тут acquire

        uint64_t unlocked_state = (_lock.load() & GEN_VAL_MASK);
        uint64_t locked_state = (_lock.load() | LCK_BIT_MASK);
        if(_lock.compare_exchange_strong(unlocked_state,locked_state,suc,fail)){
            return _lock & GEN_VAL_MASK;
        }

        return 0;
    }

    /**
     * Try to release "lock". Increase generation number in case of sucess
     *
     * @param suc memory barier to set in case of success lock
     * @param fail memory barrier to set in case of failure
     */
    void unlock(std::memory_order suc, std::memory_order fail) {
        // Похоже тут release
        // TODO: implements

        uint64_t new_gen_unlocked_state = (_lock.load() & GEN_VAL_MASK) + 1UL;
        if(_lock.compare_exchange_strong(_lock.load(),new_gen_state,suc,fail)){
            // ???
        }
    }

    /**
     * Remove slot from the queue. Note that method must be called only
     * under "lock" to eliminate concurrent queue modifications
     */
    void dequeue_slot(Slot *parent, Slot *slot2remove) {
        // TODO: remove node from the queue
        // TODO: set pointer pare of "next" to null, DO NOT modify usage bit
        // TODO: if next == 0, delete pointer

        TagedPointer<Slot> tp_next = slot2remove->next_and_alive.load();

        uint64_t parent_tag = parent->next_and_alive.load().getTag();
        uint64_t removed_slot_tag = tp_next.getTag();
        tp_next.setTag(parent_tag);
        parent->next_and_alive.store(tp_next)

        if(removed_slot_tag == 0){
            delete slot2remove;
        }else{
            slot2remove->next_and_alive.store(TagedPointer{nullptr});
        }

    }

    /**
     * Function called once thread owning this slot is going to die or to
     * destory slot in some other way
     *
     * @param Slot pointer to the slot is being to orphan
     */
    void orphan_slot(Slot * slot) {
        if(slot->next() != nullptr){
            TagedPointer<Slot> tps = slot->next_and_alive.load();
            tps.setTag(0UL);
            slot->next_and_alive.store(tps);
        }else{
            delete slot;
        }
    }

private:

    static constexpr uint64_t LCK_BIT_MASK = uint64_t(1) << 63L;    //10000
    static constexpr uint64_t GEN_VAL_MASK = ~LCK_BIT_MASK;         //01111


    // First bit is used to see if lock is acquired already or no. Rest of bits is
    // a counter showing how many "generation" has been passed. One generation is a
    // single call of flat_combine function.
    //
    // Based on that counter stale slots found and gets removed from the pending
    // operations queue
    std::atomic<uint64_t> _lock;
    uint64_t generation_offset = 100;

    // Pending operations queue. Each operation to be applied to the protected
    // data structure is ends up in this queue and then executed as a batch by
    // flat_combine method call
    std::atomic<Slot *> _queue;
    std::atomic<Slot *> _dummy_tail;

    // Function to call in order to execute operations
    combiner _combine;

    // Usual strategy for the combine flat would be sort operations by some creteria
    // and optimize it somehow. That array is using by executor thread to prepare
    // number of ops to pass to combine
    std::array<OpNode *, QMS> _combine_shot;

    // Slot of the current thread. If nullptr then cur thread gets access in the
    // first time or after a long period when slot has been deleted already
    ThreadLocal<Slot> _slot;
};
