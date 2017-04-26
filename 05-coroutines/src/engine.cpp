#include <coroutine/engine.h>

#include <setjmp.h>

#include <iostream>
#include <string.h>
#include <unistd.h>

namespace Coroutine {

void Engine::Store(context& ctx) {
	char base;
	ctx.Low = StackBottom;
	ctx.High = &base;

	uint32_t size = dir_stack * (ctx.High - ctx.Low);
	// storing the stack
    //std::cout << size << std::endl;	
    //std::cout << (int)dir_stack << std::endl;	
	// clear existing
	if (std::get<0>(ctx.Stack) != nullptr) {
		delete []std::get<0>(ctx.Stack);
	}

	char *buf = new char[size];
	if (buf == nullptr) {
		throw std::runtime_error("bad alloc");
	}

	memcpy(buf, (dir_stack > 0)? ctx.Low: ctx.High, size);
	std::get<0>(ctx.Stack) = buf;
	std::get<1>(ctx.Stack) = size;
}

void Engine::Restore(context& ctx) {
	char base;
	char *basep = &base;
	
	if ((dir_stack > 0 && basep <= ctx.High) || (dir_stack < 0 && basep >= ctx.High - std::get<1>(ctx.Stack))) {
		Restore(ctx);
	}
	memcpy((dir_stack > 0)? ctx.Low: ctx.High, std::get<0>(ctx.Stack), std::get<1>(ctx.Stack));
    longjmp(ctx.Environment, 1);
}

void Engine::yield() {
    // TODO: implements
    // setjmp, longjmp...
    if (alive == nullptr) {
    	return;
    }
	context *head = alive;
	while (head) {
		if (head == cur_routine) {
			head = head->next;
			continue;
		}
		context *check = head;
		while (check->callee) {
			if (check->callee == cur_routine) {
				head = head->next;
				break;
			}
		}

		if (!check->callee) {
			// we found context not predecessor of current
			sched(head);
		} else {
			continue;
		}
	}
}

void Engine::sched(void* routine) {

    if (routine == nullptr) {
        // try give to caller
	if (cur_routine) {
		if (cur_routine->caller) {
			sched(cur_routine->caller);
		} else {
			yield();
		}
	}
        // otherwise yield
	return;
    }

    context *ctx = (context *) routine;
 
    // save current if it exists
    if (cur_routine) {
        if (setjmp(cur_routine->Environment) == 0) { 
            // we should store everything
            Store(*cur_routine);

            if (cur_routine->caller == ctx) {
                ctx->caller = cur_routine->caller->caller;
                cur_routine->caller = nullptr;
            } else {
                ctx->caller = cur_routine;
            }
            cur_routine->callee = ctx;
            cur_routine = ctx;
            Restore(*ctx);
        }
        std::cout << "cur_rout env::" << cur_routine->Environment << std::endl;
        std::cout << "ctx env::" << ctx->Environment << std::endl;
        //sleep(1);
    } else {
            cur_routine = ctx;
            Restore(*ctx);
    }

    // restore that and jump
    // setjmp, longjmp...
}

} // namespace Coroutine
