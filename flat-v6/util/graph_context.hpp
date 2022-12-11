#pragma once
#include <vector>

namespace graph_util
{
    class GraphContext
    {
    private:
        struct NodeContainer
        {
            virtual ~NodeContainer() = default;
        };

        template<typename TNode>
        struct TNodeContainer : public NodeContainer, public TNode
        {
            TNode* get() { return this; }
        };

    private:
        std::vector<NodeContainer*> m_nodes;

    public:
        template<typename TNode>
        TNode* allocate()
        {
            auto node = new TNodeContainer<TNode>();
            m_nodes.push_back(node);
            return node->get();
        }

        template<typename TNode>
        TNode* make(TNode&& node)
        {
            return new (allocate<TNode>()) TNode(std::move(node));
        }

        ~GraphContext()
        {
            for (auto node : m_nodes)
                delete node;
        }
    };
}