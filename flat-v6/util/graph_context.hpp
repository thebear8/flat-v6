#pragma once
#include <vector>

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
        TNodeContainer(TNode&& value) : TNode(std::forward<TNode>(value)) {}
        virtual ~TNodeContainer() = default;

        TNode* get() { return this; }
    };

private:
    std::vector<NodeContainer*> m_nodes;

public:
    template<typename TNode>
    TNode* make(TNode&& node)
    {
        auto container = new TNodeContainer<TNode>(std::forward<TNode>(node));
        m_nodes.push_back(container);
        return container->get();
    }

    ~GraphContext()
    {
        // TODO: FIX MEMORY DEALLOCATION CRASH!!!
        // THIS IS NOT A GOOD SOLUTION!!!!
        // for (auto node : m_nodes)
        // delete node;
    }
};