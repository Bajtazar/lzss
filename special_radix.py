from __future__ import annotations
from dataclasses import dataclass, field


class SpecialRadixTree:
    @dataclass
    class Node:
        parent: SpecialRadixTree.Node = None
        children: dict = field(default_factory=dict)
        expression: str = ""

    def __init__(self, sequence):
        self.__build_from_sequence(sequence)

    def __build_from_sequence(self, sequence):
        self.__root = self.Node()
        nodes: dict[str, SpecialRadixTree.Node] = self.__root.children
        for level in range(len(sequence)):
            for idx in range(len(sequence) - level):
                seq = sequence[idx:idx+level + 1]

                if level == 0 and seq not in nodes:
                    if seq not in nodes:
                        nodes[seq] = self.Node(expression=seq)
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
                        node.children[token] = self.Node(expression=token, parent=node)

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
        print(self.__root)


tree = SpecialRadixTree("ala ma kota a kot ma ale")
