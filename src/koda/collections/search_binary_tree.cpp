#include <koda/collections/search_binary_tree.hpp>

#include <algorithm>
#include <assert>
#include <format>
#include <ranges>

namespace koda {

void SearchBinaryTree::AddString(StringView string) {
    assert(string.size() == string_size_ &&
           "Inserted string have to have fixed size equal to string_size_");

    InsertNewNode(string.data());
    ++buffer_start_index_;
}

void SearchBinaryTree::RemoveString(StringView string) {
    // auto iter = tree_.find(string);

    // if (iter == tree_.end()) {
    //     throw std::runtime_error{std::format(
    //         "Unknown string ({}) has been given",
    //         std::string_view{reinterpret_cast<const char*>(string.data()),
    //                          string.size()})};
    // }

    // // If it is present only one time in the dictionary then delete
    // if (iter->second.second == 1) {
    //     tree_.erase(iter);
    // } else {
    //     // Decrease a reference counter
    //     ++(iter->second.second);
    // }

    ++dictionary_start_index_;
}

SearchBinaryTree::RepeatitionMarker SearchBinaryTree::FindMatch(
    StringView buffer) const {
    assert(buffer.size() == string_size_ &&
           "Inserted string have to have fixed size equal to string_size_");

    auto [position, length] = FindString(buffer.data());

    if (!length) {
        return {0, 0}
    }

    return {// Calculate relative offset from the start of the dictionary
            position - dictionary_start_index_, length};
}

SearchBinaryTree::Node::Node(uint8_t* key, size_t insertion_index, Node* parent,
                             Color color)
    : key{std::move(key)},
      insertion_index{insertion_index},
      parent{parent},
      color{color} {}

SearchBinaryTree::Node::~Node() {
    if (left) {
        delete left;
    }
    if (right) {
        delete right;
    }
}

void SearchBinaryTree::RotateLeft(Node*& node) {
    Node* right = node->right;
    node->right = right->left;
    right->parent = node->parent;
    node->parent = right;
    right->left = node;
    node = right;
}

void SearchBinaryTree::RotateRight(Node*& node) {
    Node* left = node->left;
    node->left = left->right;
    left->parent = node->parent;
    node->parent = left;
    left->right = node;
    node = left;
}

void SearchBinaryTree::RotateLeftRight(Node*& node) {
    RotateLeft(node->left);
    RotateRight(node);
}

void SearchBinaryTree::RotateRightLeft(Node*& node) {
    RotateRight(node->right);
    RotateLeft(node);
}

void SearchBinaryTree::InsertNewNode(uint8_t* key) {
    if (!root_) [[unlikely]] {
        return root_.reset(
            new Node{.key = key, .insertion_index = buffer_start_index_});
    }
    if (auto inserted = TryToInserLeaf(key)) {
        BuildNode(key, inserted->first, inserted->second);
        assert(inserted->first->parent && "Parent has to exist");
        if (inserted->first->parent.color == Node::Color::kRed) {
            FixInsertionImbalance(inserted->first);
        }
    }
}

void SearchBinaryTree::UpdateNodeReference(Node* node, uint8_t* key) {
    ++node->ref_counter;
    node->key = key;
}

void SearchBinaryTree::BuildNode(uint8_t* key, Node*& node, Node* parent) {
    node.reset(
        new Node{.key = key, .parent = parent, .color = Node::Color::kRed});
}

std::optional<SearchBinaryTree::NodeSpot> SearchBinaryTree::TryToInserLeaf(
    uint8_t* key) {
    const StringView key_view{key, string_size_};
    Node* node = root_.get();
    Node* parent = nullptr;
    for (; node; parent = node) {
        switch (key_view <=> StringView{node->key, string_size_}) {
            case std::weak_ordering::equivalent:
                UpdateNodeReference(node, key);
                return std::nullopt;
            case std::weak_ordering::less:
                node = node->left;
                break;
            case std::weak_ordering::greater:
                node = node->right;
                break;
            default:
                std::unreachable();
        };
    }
    [[asume(parent != nullptr)]];
    return {std::in_place, node, parent};
}

void SearchBinaryTree::FixInsertionImbalance(Node*& node) {
    Node* parent = node->parent;
    Node* grand_parent = parent->parent;

    while (parent->color == Node::Color::kRed) {
        FixLocalInsertionImbalance(node, parent, grand_parent);
    }
    root_->color = Node::Color::kBlack;
}

void SearchBinaryTree::FixLocalInsertionImbalance(Node*& node, Node*& parent,
                                                  Node*& grandparent) {
    if (parent == grand_parent->left) {
        if (grand_parent->right == Node::Color::kRed) {
            FixInsertionGrandparentNodeColoring(node, grandparent);
        } else {
            FixInsertionLeftGrandparentChildOrientation(node, parent,
                                                        grandparent);
        }
    } else {
        if (grand_parent->left == Node::Color::kRed) {
            FixInsertionGrandparentNodeColoring(node, grandparent);
        } else {
            FixInsertionRightGrandparentChildOrientation(node, parent,
                                                         grandparent);
        }
    }
}

void SearchBinaryTree::FixInsertionGrandparentNodeColoring(Node*& node,
                                                           Node*& grandparent) {
    grand_parent->right->color = Node::Color::kBlack;
    grand_parent->left->color = Node::Color::kBlack;
    grand_parent->color = Node::Color::kRed;
    node = grand_parent;
}

void SearchBinaryTree::FixInsertionLeftGrandparentChildOrientation(
    Node*& node, Node*& parent, Node*& grandparent) {
    if (node == parent->right) {
        RotateLeft(node = parent);
    }
    parent->color = Node::Color::kBlack;
    grand_parent->color = Node::Color::kRed;
    RotateRight(grand_parent);
}

void SearchBinaryTree::FixInsertionRightGrandparentChildOrientation(
    Node*& node, Node*& parent, Node*& grandparent) {
    if (node == parent->left) {
        RotateRight(node = parent);
    }
    parent->color = Node::Color::kBlack;
    grand_parent->color = Node::Color::kRed;
    RotateLeft(grand_parent);
}

std::pair<size_t, size_t> SearchBinaryTree::FindString(
    const uint8_t* buffer) const {
    std::pair<size_t, size_t> match{};
    for (const Node* node = root_.get(); node;) {
        auto prefix_length = FindCommonPrefixSize(buffer, node->key);
        if (prefix_length == string_size_) {
            return {node->insertion_index, string_size_};
        }
        UpdateMatchInfo(match, node);

        if (MakeView(buffer, prefix_length) <
            MakeView(node->key, prefix_length)) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return match;
}

StringView SearchBinaryTree::MakeView(const uint8_t* buffer,
                                      size_t prefix_length) const {
    return StringView{buffer + prefix_length, string_size_ - prefix_length};
}

/*static*/ void SearchBinaryTree::UpdateMatchInfo(
    std::pair<size_t, size_t>& match_info, size_t prefix_length,
    const Node* node) noexcept {
    if (match_info.second < prefix_length) {
        match_info.first = node->insertion_index;
        match_info.second = prefix_length;
    }
}

size_t SearchBinaryTree::FindCommonPrefixSize(
    const uint8_t* buffer, const uint8_t* node) const noexcept {
    for (size_t i = 0; i < string_size_; ++i) {
        if (buffer[i] != node[i]) {
            return i;
        }
    }
    return string_size_;
}

}  // namespace koda
