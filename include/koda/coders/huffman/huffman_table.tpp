#pragma once

#include <koda/collections/forward_list.hpp>

#include <variant>

namespace koda {

namespace details {

/// There are a couple of ways in which the huffman table creator coould be
/// implemented. Firstly it could be implemented using a std::priority_queue.
/// There are two problems with this approach. Firstly the std::priority_queue
/// does not support constexprs during the time of this code creation. Secondly
/// it would require a lot of copying of node-leaf variants since the underlying
/// collection is a std::vector. Second option is to use a specialized
/// collection such as a Fibonnacy tree. The problem with this approach is that
/// for two elements removed a one additional is added. Fibbonaci heap is not
/// designed with a lot of insertions in mind so it would get slow very fast.
/// The third option is a multimap. The problem with multimap is that for much
/// of STL containers the insertion/removal operations on them invalidates their
/// pointers. That would require a manual scanning for the second symbol after
/// the first one was removed (koda::Map may also invalidate iterator after node
/// removal due to the internal rebalancing mechanism that moves node value
/// during a node with two children removal). This implementation utilized
/// fourth startegy that is a simple map with an equviariance classes. Each
/// equivariance class contains elements with the same key. This logic is ment
/// to be used only once so it doesn't have to be the fastest one and this is
/// why there is no explicit allocation control given to the user

template <typename Token, std::integral CountTp>
class MakeHuffmanTableFn {
   public:
    explicit constexpr MakeHuffmanTableFn(const Map<Token, CountTp>& count) {
        InitializeWorkTable(count);
        ProcessWorkTable();
    }

    constexpr HuffmanTable<Token>&& table() && {
        if (NodePtr* root = std::get_if<NodePtr>(&GetRoot())) {
            UnwindTree(root->get());
        } else {
            table_.Emplace(std::get<Token>(GetRoot()), std::vector<bool>{});
        }
        return std::move(table_);
    }

   private:
    struct Node {
        using NodeOrLeaf = std::variant<std::unique_ptr<Node>, Token>;

        NodeOrLeaf left;
        NodeOrLeaf right;
        Node* parent = nullptr;
    };

    using NodeOrLeaf = Node::NodeOrLeaf;
    using NodePtr = std::unique_ptr<Node>;

    Map<CountTp, ForwardList<NodeOrLeaf>> work_table_;
    std::vector<bool> symbols_;
    HuffmanTable<Token> table_;

    constexpr void InitializeWorkTable(const Map<Token, CountTp>& count) {
        for (const auto& [token, occurences] : count) {
            Emplace(occurences, NodeOrLeaf{token});
        }
    }

    constexpr void Emplace(CountTp occurences, NodeOrLeaf token) {
        if (auto iter = work_table_.Find(occurences);
            iter != work_table_.end()) {
            assert(iter->first == occurences);
            iter->second.PushFront(std::move(token));
        } else {
            // std::initializer_list contains only constant elements so if they
            // are move-only then the container cannot be initialized with it
            ForwardList<NodeOrLeaf> equivariant;
            equivariant.PushFront(std::move(token));
            assert(work_table_.Emplace(occurences, std::move(equivariant)) !=
                   work_table_.end());
        }
    }

    constexpr NodeOrLeaf ConcatenateNodes(NodeOrLeaf&& left,
                                          NodeOrLeaf&& right) {
        std::unique_ptr<Node> node{new Node{std::move(left), std::move(right)}};
        if (auto* left_node = std::get_if<NodePtr>(&node->left)) {
            (*left_node)->parent = node.get();
        }
        if (auto* right_node = std::get_if<NodePtr>(&node->right)) {
            (*right_node)->parent = node.get();
        }
        return node;
    }

    constexpr void RemoveElementsFromEquivariantNodes(const auto& iter,
                                                      size_t length = 1) {
        auto& equivariant = iter->second;
        for (size_t i = 0; i < length; ++i) {
            equivariant.PopFront();
        }
        if (equivariant.empty()) {
            work_table_.Remove(iter);
        }
    }

    constexpr void EmplaceEquivariantSupernode(const auto& iter) {
        auto& least_common = iter->second;
        auto fist_common_iter = least_common.begin();
        auto second_common_iter = std::next(fist_common_iter);
        auto new_node = ConcatenateNodes(std::move(*fist_common_iter),
                                         std::move(*second_common_iter));
        auto new_occur = 2 * iter->first;
        RemoveElementsFromEquivariantNodes(iter, 2);
        Emplace(new_occur, std::move(new_node));
    }

    constexpr void EmplaceSupernode(auto iter) {
        auto& least_common = iter->second;
        auto least_common_occur = iter->first;
        auto& second_least_common = (++iter)->second;

        auto new_node =
            ConcatenateNodes(std::move(least_common.front()),
                             std::move(second_least_common.front()));
        auto new_occur = least_common_occur + iter->first;
        RemoveElementsFromEquivariantNodes(iter, 1);
        work_table_.Remove(least_common_occur);
        Emplace(new_occur, std::move(new_node));
    }

    constexpr void ProcessWorkTable() {
        // Map sort elements in ascending order
        while (work_table_.size() > 1 ||
               work_table_.begin()->second.size() > 1) {
            auto iter = work_table_.begin();
            if (iter->second.size() > 1) {
                EmplaceEquivariantSupernode(iter);
            } else {
                EmplaceSupernode(iter);
            }
        }
    }

    constexpr Node* FindLeftmost(Node* node) {
        for (; std::holds_alternative<NodePtr>(node->left);
             node = std::get<NodePtr>(node->left).get()) {
            symbols_.push_back(0);
        }
        return node;
    }

    constexpr void AppendLeafNodes(Node* node) {
        auto set_token_fn = [&](const NodeOrLeaf& leaf, bool bit) {
            if (const Token* token = std::get_if<Token>(&leaf)) {
                symbols_.push_back(bit);
                table_.Emplace(*token, symbols_);
                symbols_.pop_back();
            }
        };
        set_token_fn(node->left, 0);
        set_token_fn(node->right, 1);
    }

    constexpr NodeOrLeaf& GetRoot() {
        assert((work_table_.size() == 1) &&
               (work_table_.begin()->second.size() == 1));
        return work_table_.begin()->second.front();
    }

    constexpr bool UnwindTreeCheckLeftSubtree(Node*& node) {
        if (std::holds_alternative<NodePtr>(node->right)) {
            symbols_.push_back(1);
            node = FindLeftmost(std::get<NodePtr>(node->right).get());
            AppendLeafNodes(node);
            return true;
        }
        return false;
    }

    constexpr void UnwindTreeReturnToFirstUnvisitedParent(Node*& node) {
        static constexpr auto kIsRightChild = [](const Node* node,
                                                 const Node* previous) {
            if (auto* right_node = std::get_if<NodePtr>(&node->right)) {
                return right_node->get() == previous;
            }
            return false;
        };

        Node* previous;

        do {
            previous = node;
            node = node->parent;
            symbols_.pop_back();
        } while (node && kIsRightChild(node, previous));
    }

    constexpr void UnwindTree(Node* root) {
        // For each visited node check whether its children are leaves (leaves
        // are NOT visited by this algorithm)
        Node* node = FindLeftmost(root);
        AppendLeafNodes(node);

        while (node) {
            if (UnwindTreeCheckLeftSubtree(node)) {
                continue;
            }

            UnwindTreeReturnToFirstUnvisitedParent(node);

            if (node) {
                AppendLeafNodes(node);
            }
        }
    }
};

}  // namespace details

template <typename Token, std::integral CountTp>
[[nodiscard]] constexpr HuffmanTable<Token> MakeHuffmanTable(
    const Map<Token, CountTp>& count) {
    if (count.empty()) [[unlikely]] {
        throw std::logic_error{"Given count map is empty"};
    }
    return details::MakeHuffmanTableFn{count}.table();
}

}  // namespace koda
