#pragma once

#include <algorithm>
#include <cassert>
#include <ranges>

namespace koda {

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::RedBlackTree(
    const AllocatorTp& allocator) noexcept
    : pool_{allocator} {}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::RedBlackTree(
    RedBlackTree&& other) noexcept
    : root_{std::exchange(other.root_, nullptr)},
      pool_{std::move(other.pool_)} {}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>&
RedBlackTree<ValueTp, AllocatorTp>::operator=(RedBlackTree&& other) noexcept {
    Destroy();
    root_ = std::exchange(other.root_, nullptr);
    pool_ = std::move(other.pool_);
    return *this;
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::~RedBlackTree() {
    Destroy();
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr AllocatorTp
RedBlackTree<ValueTp, AllocatorTp>::get_allocator() const {
    return pool_.get_allocator();
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::Node::Node(ValueTp value,
                                                         Node* parent,
                                                         Color color)
    : value{std::move(value)}, parent{parent}, color{color} {}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp,
                       AllocatorTp>::NodePool::Scheduler::~Scheduler() {
    pool.ReturnNode(node);
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodePool::NodePool(
    const AllocatorTp& allocator) noexcept
    : allocator_{allocator} {}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodePool::NodePool(
    NodePool&& other) noexcept
    : allocator_{std::move(other.allocator_)},
      handle_{std::exchange(other.handle_, nullptr)} {}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodePool&
RedBlackTree<ValueTp, AllocatorTp>::NodePool::operator=(
    NodePool&& other) noexcept {
    Destroy();
    allocator_ = std::move(other.allocator_);
    handle_ = std::exchange(other.handle_, nullptr);
    return *this;
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::NodePool::ReturnNode(
    Node* handle) {
    handle->left = handle_;
    handle_ = handle;
    std::destroy_at(&handle_->value);
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodePool::Scheduler
RedBlackTree<ValueTp, AllocatorTp>::NodePool::ScheduleForReturn(Node* node) {
    return Scheduler{*this, node};
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::Node*
RedBlackTree<ValueTp, AllocatorTp>::NodePool::GetNode(ValueTp value,
                                                      Node* parent,
                                                      Node::Color color) {
    if (!handle_) {
        Node* node = NodeTraits::allocate(allocator_, 1);
        NodeTraits::construct(allocator_, node, std::move(value), parent,
                              color);
        return node;
    }
    Node* node = handle_;
    handle_ = node->left;

    NodeTraits::construct(allocator_, node, std::move(value), parent, color);
    return node;
}

template <typename ValueTp, typename AllocatorTp>
constexpr AllocatorTp
RedBlackTree<ValueTp, AllocatorTp>::NodePool::get_allocator() const {
    return allocator_;
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]]
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeAllocatorTp&
RedBlackTree<ValueTp, AllocatorTp>::NodePool::get_node_allocator() noexcept {
    return allocator_;
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodePool::~NodePool() {
    Destroy();
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::NodePool::Destroy() {
    for (Node* node = handle_; node;) {
        Node* old_node = node;
        node = node->left;
        NodeTraits::deallocate(allocator_, old_node, 1);
    }
}

// Recursiveless tree iterator!
template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeIteratorBase<
    IsConst>::NodeIteratorBase(pointer_type node, bool is_sentinel) noexcept
    : current_{(node && !is_sentinel) ? FindLeftmost(node) : node},
      is_sentinel_{is_sentinel || !node} {}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeIteratorBase<
    IsConst>::value_type
RedBlackTree<ValueTp, AllocatorTp>::NodeIteratorBase<IsConst>::operator*()
    const noexcept {
    return *current_;
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeIteratorBase<
    IsConst>::pointer_type
RedBlackTree<ValueTp, AllocatorTp>::NodeIteratorBase<IsConst>::operator->()
    const noexcept {
    return current_;
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeIteratorBase<IsConst>&
RedBlackTree<ValueTp,
             AllocatorTp>::NodeIteratorBase<IsConst>::operator++() noexcept {
    if (current_->right) {
        current_ = FindLeftmost(current_->right);
        return *this;
    }

    pointer_type previous;
    do {
        previous = current_;
        current_ = current_->parent;
    } while (current_ && previous == current_->right);

    if (!current_) {
        current_ = previous;
        is_sentinel_ = true;
    }

    return *this;
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr RedBlackTree<ValueTp,
                                     AllocatorTp>::NodeIteratorBase<IsConst>
RedBlackTree<ValueTp, AllocatorTp>::NodeIteratorBase<IsConst>::operator++(
    int) noexcept {
    auto temp = *this;
    ++(*this);
    return temp;
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeIteratorBase<IsConst>&
RedBlackTree<ValueTp,
             AllocatorTp>::NodeIteratorBase<IsConst>::operator--() noexcept {
    if (is_sentinel_) {
        current_ = FindRightmost(current_);
        is_sentinel_ = false;
        return *this;
    }

    if (current_->left) {
        current_ = FindRightmost(current_->left);
        return *this;
    }

    pointer_type previous;
    do {
        previous = current_;
        current_ = current_->parent;
    } while (current_ && previous == current_->left);
    return *this;
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr RedBlackTree<ValueTp,
                                     AllocatorTp>::NodeIteratorBase<IsConst>
RedBlackTree<ValueTp, AllocatorTp>::NodeIteratorBase<IsConst>::operator--(
    int) noexcept {
    auto temp = *this;
    --(*this);
    return temp;
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr bool
RedBlackTree<ValueTp, AllocatorTp>::NodeIteratorBase<IsConst>::operator==(
    const NodeIteratorBase& other) const noexcept {
    return (is_sentinel_ == other.is_sentinel_) && (current_ == other.current_);
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
/*static*/ constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeIteratorBase<
    IsConst>::pointer_type
RedBlackTree<ValueTp, AllocatorTp>::NodeIteratorBase<IsConst>::FindLeftmost(
    pointer_type node) noexcept {
    for (; node->left; node = node->left);
    return node;
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
/*static*/ constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeIteratorBase<
    IsConst>::pointer_type
RedBlackTree<ValueTp, AllocatorTp>::NodeIteratorBase<IsConst>::FindRightmost(
    pointer_type node) noexcept {
    for (; node->right; node = node->right);
    return node;
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodePtr&
RedBlackTree<ValueTp, AllocatorTp>::root() noexcept {
    return root_;
}

template <typename ValueTp, typename AllocatorTp>
constexpr const RedBlackTree<ValueTp, AllocatorTp>::NodePtr&
RedBlackTree<ValueTp, AllocatorTp>::root() const noexcept {
    return root_;
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodePtr
RedBlackTree<ValueTp, AllocatorTp>::InsertNode(ValueTp value) {
    if (!root_) [[unlikely]] {
        root_ = pool_.GetNode(std::move(value));
        CheckInvariants();
        return root_;
    }
    if (auto inserted = this->FindInsertionLocation(value)) {
        Node* new_node =
            BuildNode(std::move(value), inserted->first, inserted->second);
        assert(new_node->parent && "Parent has to exist");
        if (new_node->parent->color == Node::Color::kRed) {
            FixInsertionImbalance(new_node);
        }
        CheckInvariants();
        return new_node;
    }
    return nullptr;
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::RemoveNode(NodePtr node) {
    if (node->left && node->right) {
        RemoveNodeWithTwoChildren(node);
        return CheckInvariants();
    }

    if (node->left || node->right) {
        RemoveNodeWithOneChild(node, node->left ? node->left : node->right);
        return CheckInvariants();
    }

    RemoveChildlessNode(node);
    CheckInvariants();
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeIterator
RedBlackTree<ValueTp, AllocatorTp>::node_begin() noexcept {
    return NodeIterator{root_};
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeConstIterator
RedBlackTree<ValueTp, AllocatorTp>::node_begin() const noexcept {
    return NodeConstIterator{root_};
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeIterator
RedBlackTree<ValueTp, AllocatorTp>::node_end() noexcept {
    return NodeIterator{root_, true};
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeConstIterator
RedBlackTree<ValueTp, AllocatorTp>::node_end() const noexcept {
    return NodeConstIterator{root_, true};
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::RotateLeft(Node* node) {
    Node* right = node->right;
    Node* right_left = right->left;

    node->right = right_left;
    right->left = node;

    RotateHelper(node, right_left, right);
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::RotateRight(Node* node) {
    Node* left = node->left;
    Node* left_right = left->right;

    node->left = left_right;
    left->right = node;

    RotateHelper(node, left_right, left);
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::RotateHelper(Node* node,
                                                                Node* child,
                                                                Node* root) {
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

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::Node*
RedBlackTree<ValueTp, AllocatorTp>::BuildNode(ValueTp&& value, Node*& node,
                                              Node* parent) {
    return node = pool_.GetNode(std::move(value), parent, Node::Color::kRed);
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::FixInsertionImbalance(
    Node* node) {
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

template <typename ValueTp, typename AllocatorTp>
constexpr bool RedBlackTree<ValueTp, AllocatorTp>::FixLocalInsertionImbalance(
    Node*& node, Node*& parent, Node*& grand_parent) {
    Node* uncle = grand_parent->right == parent ? grand_parent->left
                                                : grand_parent->right;
    if (!uncle || uncle->color == Node::Color::kBlack) {
        if (grand_parent->right == parent) {
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

template <typename ValueTp, typename AllocatorTp>
constexpr void
RedBlackTree<ValueTp, AllocatorTp>::FixLocalInsertionImbalanceRight(
    Node*& node, Node*& parent, Node*& grand_parent, Node* uncle) {
    if (node == parent->left) {
        RotateRight(parent);
        node = parent;
        parent = grand_parent->right;
    }
    RotateLeft(grand_parent);
}

template <typename ValueTp, typename AllocatorTp>
constexpr void
RedBlackTree<ValueTp, AllocatorTp>::FixLocalInsertionImbalanceLeft(
    Node*& node, Node*& parent, Node*& grand_parent, Node* uncle) {
    if (node == parent->right) {
        RotateLeft(parent);
        node = parent;
        parent = grand_parent->left;
    }
    RotateRight(grand_parent);
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::Node*
RedBlackTree<ValueTp, AllocatorTp>::FindSuccessor(Node* node) {
    for (; node->left; node = node->left);
    return node;
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::RemoveNodeWithTwoChildren(
    Node* node) {
    Node* successor = FindSuccessor(node->right);

    auto allocator = pool_.get_allocator();
    ValueTraits::construct(allocator, &node->value,
                           std::move(successor->value));

    if (successor->right) {
        return RemoveNodeWithOneChild(successor, successor->right);
    }
    RemoveChildlessNode(successor);
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::RemoveNodeWithOneChild(
    Node* node, Node* children) {
    children->color = Node::Color::kBlack;
    children->parent = node->parent;

    if (node->parent) {
        if (node == node->parent->left) {
            node->parent->left = children;
        } else {
            node->parent->right = children;
        }
    } else {
        root_ = children;
    }

    pool_.ReturnNode(node);
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::RemoveRootNode() {
    pool_.ReturnNode(std::exchange(root_, nullptr));
}

template <typename ValueTp, typename AllocatorTp>
constexpr void
RedBlackTree<ValueTp, AllocatorTp>::PrepareToRemoveRedChildlessNode(
    Node* node) {
    if (node->parent->right == node) {
        node->parent->right = nullptr;
    } else {
        node->parent->left = nullptr;
    }
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::
    RemoveBlackChildlessNodeRightPathSiblingIsRed(Node* node, Node* parent,
                                                  Node* sibling,
                                                  Node* left_nephew,
                                                  Node* right_nephew) {
    RotateRight(parent);
    parent->color = Node::Color::kRed;
    sibling->color = Node::Color::kBlack;
    sibling = right_nephew;

    left_nephew = right_nephew->left;
    if (left_nephew && left_nephew->color == Node::Color::kRed) {
        return RemoveNodeRotateParentRightPath(parent, sibling, left_nephew);
    }
    right_nephew = right_nephew->right;
    if (right_nephew && right_nephew->color == Node::Color::kRed) {
        return RemoveNodeRotateSiblingRightPath(parent, sibling, right_nephew);
    }

    sibling->color = Node::Color::kRed;
    parent->color = Node::Color::kBlack;
}

template <typename ValueTp, typename AllocatorTp>
constexpr bool
RedBlackTree<ValueTp, AllocatorTp>::RemoveBlackChildlessNodeRightPath(
    Node* node, Node* parent) {
    Node* sibling = parent->left;
    Node* left_nephew = sibling->left;
    Node* right_nephew = sibling->right;

    if (sibling->color == Node::Color::kRed) {
        RemoveBlackChildlessNodeRightPathSiblingIsRed(
            node, parent, sibling, left_nephew, right_nephew);
        return true;
    }

    if (left_nephew && left_nephew->color == Node::Color::kRed) {
        RemoveNodeRotateParentRightPath(parent, sibling, left_nephew);
        return true;
    }

    if (right_nephew && right_nephew->color == Node::Color::kRed) {
        RemoveNodeRotateSiblingRightPath(parent, sibling, right_nephew);
        return true;
    }

    sibling->color = Node::Color::kRed;

    if (parent->color == Node::Color::kRed) {
        parent->color = Node::Color::kBlack;
        return true;
    }
    return false;
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::
    RemoveBlackChildlessNodeLeftPathSiblingIsRed(Node* node, Node* parent,
                                                 Node* sibling,
                                                 Node* left_nephew,
                                                 Node* right_nephew) {
    RotateLeft(parent);
    parent->color = Node::Color::kRed;
    sibling->color = Node::Color::kBlack;
    sibling = left_nephew;

    right_nephew = left_nephew->right;
    if (right_nephew && right_nephew->color == Node::Color::kRed) {
        return RemoveNodeRotateParentLeftPath(parent, sibling, right_nephew);
    }
    left_nephew = left_nephew->left;
    if (left_nephew && left_nephew->color == Node::Color::kRed) {
        return RemoveNodeRotateSiblingLeftPath(parent, sibling, left_nephew);
    }

    sibling->color = Node::Color::kRed;
    parent->color = Node::Color::kBlack;
}

template <typename ValueTp, typename AllocatorTp>
constexpr bool
RedBlackTree<ValueTp, AllocatorTp>::RemoveBlackChildlessNodeLeftPath(
    Node* node, Node* parent) {
    Node* sibling = parent->right;
    Node* left_nephew = sibling->left;
    Node* right_nephew = sibling->right;

    if (sibling->color == Node::Color::kRed) {
        RemoveBlackChildlessNodeLeftPathSiblingIsRed(node, parent, sibling,
                                                     left_nephew, right_nephew);
        return true;
    }

    if (right_nephew && right_nephew->color == Node::Color::kRed) {
        RemoveNodeRotateParentLeftPath(parent, sibling, right_nephew);
        return true;
    }

    if (left_nephew && left_nephew->color == Node::Color::kRed) {
        RemoveNodeRotateSiblingLeftPath(parent, sibling, left_nephew);
        return true;
    }

    sibling->color = Node::Color::kRed;

    if (parent->color == Node::Color::kRed) {
        parent->color = Node::Color::kBlack;
        return true;
    }
    return false;
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::RemoveBlackChildlessNode(
    Node* node) {
    if (node->parent->right == node) {
        node->parent->right = nullptr;
        if (RemoveBlackChildlessNodeRightPath(node, node->parent)) {
            return;
        }
    } else {
        node->parent->left = nullptr;
        if (RemoveBlackChildlessNodeLeftPath(node, node->parent)) {
            return;
        }
    }

    for (node = node->parent; Node* parent = node->parent;
         node = node->parent) {
        if (parent->right == node) {
            if (RemoveBlackChildlessNodeRightPath(node, parent)) {
                return;
            }
        } else {
            if (RemoveBlackChildlessNodeLeftPath(node, parent)) {
                return;
            }
        }
    }
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::RemoveChildlessNode(
    Node* node) {
    if (node == root_) {
        return RemoveRootNode();
    }

    // will deallocate node after all logic has been handled
    auto handle = pool_.ScheduleForReturn(node);

    if (node->color == Node::Color::kRed) {
        return PrepareToRemoveRedChildlessNode(node);
    }

    RemoveBlackChildlessNode(node);
}

template <typename ValueTp, typename AllocatorTp>
constexpr void
RedBlackTree<ValueTp, AllocatorTp>::RemoveNodeRotateSiblingRightPath(
    Node* parent, Node* sibling, Node* nephew) {
    RotateLeft(sibling);
    sibling->color = Node::Color::kRed;
    nephew->color = Node::Color::kBlack;
    RemoveNodeRotateParentRightPath(parent, nephew, sibling);
}

template <typename ValueTp, typename AllocatorTp>
constexpr void
RedBlackTree<ValueTp, AllocatorTp>::RemoveNodeRotateSiblingLeftPath(
    Node* parent, Node* sibling, Node* nephew) {
    RotateRight(sibling);
    sibling->color = Node::Color::kRed;
    nephew->color = Node::Color::kBlack;
    RemoveNodeRotateParentLeftPath(parent, nephew, sibling);
}

template <typename ValueTp, typename AllocatorTp>
constexpr void
RedBlackTree<ValueTp, AllocatorTp>::RemoveNodeRotateParentRightPath(
    Node* parent, Node* sibling, Node* nephew) {
    RotateRight(parent);
    sibling->color = parent->color;
    parent->color = Node::Color::kBlack;
    nephew->color = Node::Color::kBlack;
}

template <typename ValueTp, typename AllocatorTp>
constexpr void
RedBlackTree<ValueTp, AllocatorTp>::RemoveNodeRotateParentLeftPath(
    Node* parent, Node* sibling, Node* nephew) {
    RotateLeft(parent);
    sibling->color = parent->color;
    parent->color = Node::Color::kBlack;
    nephew->color = Node::Color::kBlack;
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::Destroy() {
    // Instead of calling a recursive destructor call, deallocate tree in place
    // in order to avoid stack overflow for large structures!
    for (Node* node = root_; root_;) {
        if (node->left != nullptr) {
            node = std::exchange(node->left, nullptr);
            continue;
        }

        if (node->right != nullptr) {
            node = std::exchange(node->right, nullptr);
            continue;
        }

        Node* parent = node->parent;
        NodeTraits::destroy(pool_.get_node_allocator(), node);
        NodeTraits::deallocate(pool_.get_node_allocator(), node, 1);
        if (!parent) {
            return;  // root can be left with dangling pointer
        }
        node = parent;
    }
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::CheckInvariants() const {
#ifdef KODA_CHECKED_BUILD
    ValidateRedNodeConstraint();
    ValidateBlackNodeConstraint();
#endif  // KODA_CHECKED_BUILD
}

#ifdef KODA_CHECKED_BUILD

template <typename ValueTp, typename AllocatorTp>
/*static*/ constexpr std::ranges::subrange<
    typename RedBlackTree<ValueTp, AllocatorTp>::NodeConstIterator,
    typename RedBlackTree<ValueTp, AllocatorTp>::NodeConstIterator>
RedBlackTree<ValueTp, AllocatorTp>::nodes(const Node* root) noexcept {
    return {NodeConstIterator{root}, NodeConstIterator{root, true}};
}

// A red node does not have a red child
template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::ValidateRedNodeConstraint()
    const {
    for (const Node& node : nodes(root_)) {
        if (node.color == Node::Color::kRed) {
            if (node.left && node.left->color == Node::Color::kRed) {
                throw std::logic_error{"Red node invariant broken!"};
            }

            if (node.right && node.right->color == Node::Color::kRed) {
                throw std::logic_error{"Red node invariant broken!"};
            }
        }
    }
}

// Every path from a given node to any of its leaf nodes goes through the same
// number of black nodes
template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::ValidateBlackNodeConstraint()
    const {
    class SubtreeDetacher {
       public:
        constexpr SubtreeDetacher(const Node& node) noexcept
            : node_{node}, parent_{node.parent} {
            const_cast<Node&>(node_).parent = nullptr;
        }

        constexpr ~SubtreeDetacher() {
            const_cast<Node&>(node_).parent = parent_;
        }

       private:
        const Node& node_;
        Node* parent_;
    };

    for (const Node& node : nodes(root_)) {
        int64_t total_black_count = -1;

        // temporarly remove parent!
        // Otherwise it will start seeking higher nodes and will miss sentinel!
        SubtreeDetacher detacher{node};
        for (const Node& child : nodes(&node)) {
            if (!child.left && !child.right) {
                int64_t count = 0;
                for (auto* path_node = &child; path_node != &node;
                     path_node = path_node->parent) {
                    if (path_node->color == Node::Color::kBlack) {
                        ++count;
                    }
                }
                if (total_black_count == -1) {
                    total_black_count = count;
                } else if (total_black_count != count) {
                    throw std::logic_error{"Black node invariant broken!"};
                }
            }
        }
    }
}

#endif  // KODA_CHECKED_BUILD

}  // namespace koda
