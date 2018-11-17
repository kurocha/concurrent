//
//  Coentry.hpp
//  File file is part of the "Concurrent" project and released under the MIT License.
//
//  Created by Samuel Williams on 4/7/2017.
//  Copyright, 2017, by Samuel Williams. All rights reserved.
//

#pragma once

#include <memory>
#include "Stack.hpp"

#include <Coroutine/Context.h>

namespace Concurrent
{
	template <typename FunctionT>
	struct Coentry {
		FunctionT function;
		
		static COROUTINE cocall(CoroutineContext * from, CoroutineContext * self, void * argument);
		
		Coentry(FunctionT && function_) : function(std::move(function_)) {}
	};
	
	template <typename FunctionT>
	Coentry<FunctionT> make_coentry(FunctionT && function)
	{
		return Coentry<FunctionT>(std::move(function));
	}
	
	template <typename FunctionT>
	Coentry<FunctionT> * emplace_coentry(Stack & stack, FunctionT && function)
	{
		return stack.emplace<Coentry<FunctionT>>(std::move(function));
	}
}
