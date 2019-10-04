#include <afina/coroutine/Engine.h>


#include <stdio.h>
#include <string.h>

namespace Afina {
namespace Coroutine {

void Engine::Store(context &ctx) {
    char StackEndsHere;

    ctx.Hight = ctx.Low = StackBottom;
    if (&StackEndsHere > StackBottom) {
        ctx.Hight = &StackEndsHere;
    }
    else {
        ctx.Low = &StackEndsHere;
    }

    uint32_t sufficient_size = ctx.Hight - ctx.Low;
//    uint32_t sufficient_size = ctx.Low - ctx.Hight;

//    ctx.Hight = &StackEndsHere;
//    ctx.Low = StackBottom;


    //auto size = &(std::get<1>(ctx.Stack));
    char * &stack = std::get<0>(ctx.Stack);
    delete[] stack;

    uint32_t &av_size = std::get<1>(ctx.Stack);

    av_size = sufficient_size;

    stack = new char[sufficient_size];

    std::memcpy(stack, ctx.Low, sufficient_size);

    //auto tuple = std::make_tuple(stack,sufficient_size);

   // ctx.Stack = tuple;

}

void Engine::Restore(context &ctx) {
    char restore;
    std::cout<<"resH  "<< (void*)ctx.Hight<< std::endl;
    std::cout<<"resL  "<< (void*)ctx.Low<< std::endl;
    if(&restore >= ctx.Hight ){
        Restore(ctx);
    }

    cur_routine = &ctx == idle_ctx ? nullptr : &ctx;

    std::memcpy(ctx.Low, std::get<0>(ctx.Stack), std::get<1>(ctx.Stack));
    longjmp(ctx.Environment, 5);

}

void Engine::yield() {

    context * next_routine = alive;

    if (next_routine && next_routine == cur_routine) {
        next_routine = alive->next;
    }

    if (next_routine) {
        if (cur_routine != idle_ctx ){
            Store(*cur_routine);

            if(setjmp(cur_routine->Environment) > 0 ){
                return;
            }
        }

        Restore(*next_routine);
    }
}

void Engine::sched(void *routine_) {
    if (routine_ == nullptr){
        yield();
        return;
    }
    auto *other_ctx = static_cast<context*>(routine_);

    if(other_ctx != cur_routine){

        if (cur_routine) {

            Store(*cur_routine);
            if (setjmp(cur_routine->Environment) > 0) {
                return;
            }
        }

        Restore(*other_ctx);
    }


}

} // namespace Coroutine
} // namespace Afina
