#include <koda/collections/search_binary_tree.hpp>
#include <koda/utils/comparation.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <ranges>

namespace koda {

SearchBinaryTree::SearchBinaryTree(size_t string_size) noexcept
    : string_size_{string_size} {}

SearchBinaryTree::SearchBinaryTree(SearchBinaryTree&& other) noexcept
    : root_{std::exchange(other.root_, nullptr)},
      dictionary_start_index_{other.dictionary_start_index_},
      buffer_start_index_{other.buffer_start_index_},
      string_size_{other.string_size_} {}

SearchBinaryTree& SearchBinaryTree::operator=(
    SearchBinaryTree&& other) noexcept {
    Destroy();
    root_ = std::exchange(other.root_, nullptr);
    dictionary_start_index_ = other.dictionary_start_index_;
    buffer_start_index_ = other.buffer_start_index_;
    string_size_ = other.string_size_;
    return *this;
}

void SearchBinaryTree::AddString(StringView string) {
    assert(string.size() == string_size_ &&
           "Inserted string have to have fixed size equal to string_size_");

    InsertNewNode(string.data());
    ++buffer_start_index_;
}

bool SearchBinaryTree::RemoveString(StringView string) {
    Node* node = FindNodeToRemoval(string);

    if (!node) [[unlikely]] {
        return false;
    }

    if (!--node->ref_counter) {
        RemoveNode(node);
    }

    ++dictionary_start_index_;
    return true;
}

SearchBinaryTree::RepeatitionMarker SearchBinaryTree::FindMatch(
    StringView buffer) const {
    assert(
        buffer.size() <= string_size_ &&
        "Inserted string have to have fixed size not bigger than string_size_");

    auto [position, length] = FindString(buffer.data(), buffer.size());

    if (!length) {
        return {0, 0};
    }

    return {// Calculate relative offset from the start of the dictionary
            position - dictionary_start_index_, length};
}

SearchBinaryTree::~SearchBinaryTree() { Destroy(); }

SearchBinaryTree::Node::Node(const uint8_t* key, size_t insertion_index,
                             Node* parent, Color color)
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

void SearchBinaryTree::RotateLeft(Node* node) {
    Node* right = node->right;
    Node* right_left = right->left;

    node->right = right_left;
    right->left = node;

    RotateHelper(node, right_left, right);
}

void SearchBinaryTree::RotateRight(Node* node) {
    Node* left = node->left;
    Node* left_right = left->right;

    node->left = left_right;
    left->right = node;

    RotateHelper(node, left_right, left);
}

void SearchBinaryTree::RotateHelper(Node* node, Node* child, Node* root) {
    Node* parent = node->parent;
    root->parent = parent;
    node->parent = root;
    if (child) {
        child->parent = node;
    }
    if (parent) {
        if (node == parent->right) {
            parent->right = root;
        } else {
            parent->left = root;
        }
    } else {
        root_ = root;
    }
}

void SearchBinaryTree::RotateLeftRight(Node*& node) {
    RotateLeft(node->left);
    RotateRight(node);
}

void SearchBinaryTree::RotateRightLeft(Node*& node) {
    RotateRight(node->right);
    RotateLeft(node);
}

void SearchBinaryTree::InsertNewNode(const uint8_t* key) {
    if (!root_) [[unlikely]] {
        root_ = new Node{key, buffer_start_index_, nullptr};
        return;
    }
    if (auto inserted = TryToInserLeaf(key)) {
        BuildNode(key, inserted->first, inserted->second);
        assert(inserted->first->parent && "Parent has to exist");
        if (inserted->first->parent->color == Node::Color::kRed) {
            FixInsertionImbalance(inserted->first);
        }
    }
}

void SearchBinaryTree::UpdateNodeReference(Node* node, const uint8_t* key) {
    ++node->ref_counter;
    node->key = key;
    node->insertion_index = buffer_start_index_ - dictionary_start_index_;
}

void SearchBinaryTree::BuildNode(const uint8_t* key, Node*& node,
                                 Node* parent) {
    node = new Node{key, buffer_start_index_, parent, Node::Color::kRed};
}

void SearchBinaryTree::dumpTree() {
    if (root_) {
        std::cout << root_ << "]\t";
        dumpTree(root_, "");
    }
}

void SearchBinaryTree::dumpTree(Node* parent, std::string view) {
    if (parent->left) {
        dumpTree(parent->left, view + "\t");
    }
    std::cout << parent << "]\t";
    std::cout << view << "|"
              << std::string_view{reinterpret_cast<const char*>(parent->key), 4}
              << "\n";
    if (parent->right) {
        dumpTree(parent->right, view + "\t");
    }
}

std::optional<SearchBinaryTree::NodeSpot> SearchBinaryTree::TryToInserLeaf(
    const uint8_t* key) {
    const StringView key_view{key, string_size_};
    Node** node = &root_;
    Node* parent = nullptr;
    while (*node) {
        switch (
            OrderCast(key_view <=> StringView{(*node)->key, string_size_})) {
            case WeakOrdering::kEquivalent:
                UpdateNodeReference(*node, key);
                return std::nullopt;
            case WeakOrdering::kLess:
                parent = *node;
                node = &(*node)->left;
                break;
            case WeakOrdering::kGreater:
                parent = *node;
                node = &(*node)->right;
                break;
            default:
                std::unreachable();
        };
    }
    return NodeSpot{*node, parent};
}

void SearchBinaryTree::FixInsertionImbalance(Node* node) {
    Node* parent = node->parent;

    for (; parent && parent->color == Node::Color::kRed;) {
        Node* grand_parent = parent->parent;

        if (!grand_parent) {
            parent->color = Node::Color::kBlack;
            return;
        }

        if (FixLocalInsertionImbalance(node, parent, grand_parent)) {
            return;
        }

        parent->color = Node::Color::kBlack;
        grand_parent->color = Node::Color::kRed;
        node = grand_parent;
        parent = node->parent;
    }
}

bool SearchBinaryTree::FixLocalInsertionImbalance(Node*& node, Node*& parent,
                                                  Node*& grand_parent) {
    Node* uncle =
        grand_parent->right ? grand_parent->left : grand_parent->right;
    if (!uncle || uncle->color == Node::Color::kBlack) {
        if (grand_parent->right) {
            FixLocalInsertionImbalanceRight(node, parent, grand_parent, uncle);
        } else {
            FixLocalInsertionImbalanceLeft(node, parent, grand_parent, uncle);
        }
        parent->color = Node::Color::kBlack;
        grand_parent->color = Node::Color::kRed;
        return true;
    }
    uncle->color = Node::Color::kBlack;
    return false;
}

void SearchBinaryTree::FixLocalInsertionImbalanceRight(Node*& node,
                                                       Node*& parent,
                                                       Node*& grand_parent,
                                                       Node* uncle) {
    if (node == parent->left) {
        RotateRight(parent);
        node = parent;
        parent = grand_parent->right;
    }
    RotateLeft(grand_parent);
}

void SearchBinaryTree::FixLocalInsertionImbalanceLeft(Node*& node,
                                                      Node*& parent,
                                                      Node*& grand_parent,
                                                      Node* uncle) {
    if (node == parent->right) {
        RotateLeft(parent);
        node = parent;
        parent = grand_parent->left;
    }
    RotateRight(grand_parent);
}

std::pair<size_t, size_t> SearchBinaryTree::FindString(const uint8_t* buffer,
                                                       size_t length) const {
    std::pair<size_t, size_t> match{};
    for (const Node* node = root_; node;) {
        auto prefix_length = FindCommonPrefixSize(buffer, node->key, length);
        if (prefix_length == string_size_) {
            return {node->insertion_index, string_size_};
        }
        UpdateMatchInfo(match, prefix_length, node);

        if (MakeSuffixView(buffer, prefix_length) <
            MakeSuffixView(node->key, prefix_length)) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return match;
}

SearchBinaryTree::StringView SearchBinaryTree::MakeSuffixView(
    const uint8_t* buffer, size_t prefix_length) const {
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

size_t SearchBinaryTree::FindCommonPrefixSize(const uint8_t* buffer,
                                              const uint8_t* node,
                                              size_t length) const noexcept {
    for (size_t i = 0; i < length; ++i) {
        if (buffer[i] != node[i]) {
            return i;
        }
    }
    return string_size_;
}

SearchBinaryTree::Node* SearchBinaryTree::FindNodeToRemoval(
    StringView key_view) {
    for (Node* node = root_; node;) {
        switch (OrderCast(key_view <=> StringView{node->key, string_size_})) {
            case WeakOrdering::kEquivalent:
                return node;
            case WeakOrdering::kLess:
                node = node->left;
                break;
            case WeakOrdering::kGreater:
                node = node->right;
                break;
            default:
                std::unreachable();
        };
    }
    return nullptr;
}

void SearchBinaryTree::Destroy() { delete root_; }

}  // namespace koda
