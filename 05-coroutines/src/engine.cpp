#include <coroutine/engine.h>

#include <setjmp.h>

#include <iostream>
#include <string.h>
#include <unistd.h>

namespace Coroutine {

void Engine::Store(context& ctx) {
	char base;
	ctx.High = ctx.Low = StackBottom;

	if (ctx.Low > &base) {
		ctx.Low = &base;
	} else {
		ctx.High = &base;
	}
	size_t size = ctx.High - ctx.Low;
	// allocate if needed
	char *buf = std::get<0>(ctx.Stack);
	if (std::get<1>(ctx.Stack) < size || buf == nullptr) {
		delete []buf;
		buf = new char[size];
		if (buf == nullptr) {
			throw std::runtime_error("bad alloc");
		}
	}

	// storing the stack
	memcpy(buf, ctx.Low, size);
	std::get<0>(ctx.Stack) = buf;
	std::get<1>(ctx.Stack) = size;
}

void Engine::Restore(context& ctx) {
	char base;
	char *basep = &base;

	if ( ctx.Low <= basep && basep <= ctx.High) {
		Restore(ctx);
	}
	memcpy(ctx.Low, std::get<0>(ctx.Stack), std::get<1>(ctx.Stack));
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

		if (!head->callee) {
			// we found not blocked context
			sched(head);
		} else {
			head = head->next;
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
		// otherwise nothing
		return;
	}

	context *ctx = (context *) routine;

	// save current if it exists
	if (cur_routine) {
		if (setjmp(cur_routine->Environment) == 0) {
			// we should store everything
			Store(*cur_routine);

			if (cur_routine->caller == ctx) {
				ctx->callee = cur_routine->caller = nullptr;
			} else {
				ctx->caller = cur_routine;
				cur_routine->callee = ctx;
			}
		} else {
			return;
		}
	}
	// restore that and jump
	cur_routine = ctx;
	Restore(*ctx);
}

} // namespace Coroutine
