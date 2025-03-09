from __future__ import annotations
from dataclasses import dataclass, field
from pprint import pprint


class SuperRadixTree:
    @dataclass
    class Node:
        parent: SuperRadixTree.Node = None
        children: dict = field(default_factory=dict)
        expression: str = ""
        support: list = field(default_factory=list)

    def __init__(self, sequence):
        self.__build_from_sequence(sequence)
        self.__update_support_aux(self.__root)
        self.__start_idx = 0
        self.__end_idx = len(sequence)
        self.__sequence = sequence

    def __build_from_sequence(self, sequence):
        self.__root = self.Node()
        nodes: dict[str, SuperRadixTree.Node] = self.__root.children
        for level in range(len(sequence)):
            for idx in range(len(sequence) - level):
                seq = sequence[idx:idx+level + 1]

                if level == 0 and seq not in nodes:
                    if seq not in nodes:
                        nodes[seq] = self.Node(expression=seq, support={idx})
                elif level != 0:
                    preq = seq[:-1]
                    node = nodes[preq]

                    parent = node
                    length = 0
                    while parent is not None:
                        length += len(parent.expression)
                        parent = parent.parent

                    token = seq[length:]
                    if token not in node.children:
                        node.children[token] = self.Node(expression=token, parent=node, support={idx})

            if level != 0:
                new_nodes = {}
                for key, node in nodes.items():
                    if len(node.children) == 1:
                        new_expr = node.expression + list(node.children)[0]
                        if node.parent:
                            del node.parent.children[node.expression]
                            node.parent.children[new_expr] = node
                        node.expression = new_expr
                        node.children = {}
                        new_nodes[key + node.expression[-1]] = node
                    else:
                        new_nodes = {
                            **new_nodes,
                            **{key + kt[-1]: pt for kt, pt in node.children.items()}
                        }
                nodes = new_nodes

    def __update_support_aux(self, node: Node):
        if not node.children:
            return node.support
        node.support = set.union(*[self.__update_support_aux(child) for child in node.children.values()])
        return node.support

    def __print_aux(self, node: Node, tabs: int):
        print('\t' * tabs + f'"{node.expression}"')
        for child in node.children.values():
            self.__print_aux(child, tabs + 1)

    def print(self):
        self.__print_aux(self.__root, 0)

    def __print_support_aux(self, node: Node, tabs: int):
        print('\t' * tabs + f'{node.support} -> "{node.expression}"')
        for child in node.children.values():
            self.__print_support_aux(child, tabs + 1)

    def print_support(self):
        self.__print_support_aux(self.__root, 0)

    def __purge_symbol(self, node: Node, symbol):
        to_removal = []
        for tag, child in node.children.items():
            if symbol in child.support:
                if len(child.support) == 1:
                    to_removal.append(tag)
                else:
                    self.__purge_symbol(child, symbol)
        for tag in to_removal:
            del node.children[tag]
        node.support.remove(symbol)

    def pop_front(self):
        self.__purge_symbol(self.__root, self.__start_idx)
        self.__start_idx += 1

    def __get_position_aux(self, string: str, node: Node) -> int | None:
        if not string:
            return list(node.support)[0]
        for children in node.children.values():
            tag = children.expression
            if len(tag) > len(string):
                if string == tag[:len(string)]:
                    return list(children.support)[0] - self.__start_idx
            elif string[:len(tag)] == tag:
                return self.__get_position_aux(string[len(tag):], children)
        return None

    def get_position_of_substring(self, string: str):
        return self.__get_position_aux(string, self.__root)

    @staticmethod
    def __longest_common_prefix(left: str, right: str) -> tuple[str, str, str]:
        for i, (a, b) in enumerate(zip(left, right)):
            if a != b:
                return left[:i], left[i:], right[i:]

    def __add_sequence(self, sequence: str, node: Node, position):
        for children in node.children.values():
            tag = children.expression
            if len(sequence) > len(tag) and sequence[:len(tag)] == tag:
                return self.__add_sequence(sequence[len(tag):], children, position)
            elif tag[:len(sequence)] == sequence:
                return children.support.union(set((position,)))
            elif tag[0] == sequence[0]: # Ok now node have to be splitted
                stem, left, right = self.__longest_common_prefix(tag, sequence)
                children.expression = left
                node = self.Node(parent=children.parent, children={
                    left: children
                }, expression=stem, support=set.union(children.support, set((position,))))
                node.children[right] = self.Node(parent=node, expression=right, support=set((position,)))
                del children.parent.children[tag]
                children.parent.children[stem] = node
                children.parent = node
                return
        node.children[sequence] = self.Node(parent=node, expression=sequence, support=set((position,)))

    def push_back(self, letter: str) -> None:
        assert len(letter) == 1
        self.__sequence += letter
        for idx in range(len(letter)):
            sequence = self.__sequence[-idx-1:]
            self.__add_sequence(sequence, self.__root, self.__end_idx)
        self.__end_idx += 1

tree = SuperRadixTree("ala ma kota a kot ma ale")
tree.print_support()

print(tree.get_position_of_substring("ala ma kota a kot ma ale"))

tree.push_back("5")

tree.print_support()

# tree.pop_front()

# tree.print_support()

# tree = SuperRadixTree("la ma kota a kot ma ale")
# tree.print()
