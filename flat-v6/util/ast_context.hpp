#pragma once
#include <vector>

#include "visitor.hpp"

namespace ast_util
{	
	// DEPRECATED: Replaced by GraphContext
	struct AstNodeBase
	{
		size_t begin, end;

		AstNodeBase() :
			begin(0), end(0) { }

		virtual ~AstNodeBase() { }
	};

	// DEPRECATED: Replaced by GraphContext
	class AstContext
	{
	private:
		std::vector<AstNodeBase*> nodes;

	public:
		AstContext() { }
		AstContext(AstContext&&) = delete;
		AstContext(AstContext const&) = delete;

		~AstContext()
		{
			for (auto node : nodes)
				delete node;
			nodes.clear();
		}

		inline void deallocate()
		{
			for (auto node : nodes)
				delete node;
			nodes.clear();
		}

	public:
		template<typename TNode, typename... TArgs>
		TNode* make(size_t begin, size_t end, TArgs&&... args)
		{
			nodes.push_back(new TNode(std::forward<TArgs>(args)...));
			nodes.back()->begin = begin;
			nodes.back()->end = end;
			return static_cast<TNode*>(nodes.back());
		}
	};
}