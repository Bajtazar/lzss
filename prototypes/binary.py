from __future__ import annotations
from pprint import pprint
from sortedcontainers import SortedDict


class BinaryTree:
    def __init__(self, look_ahead_buffer: str, dictionary_size: int, max_match_size: int):
        self.__dictionary_size = dictionary_size
        self.__max_match_size = max_match_size
        self.__idx = 0
        self.__tree: SortedDict[str, tuple[int, int]] = SortedDict()
        self.__look_ahead_buffer = look_ahead_buffer
        self.__dictionary = ""
        self.__buf_start_idx = 0

    def print(self):
        print(f'Dict size: {self.__dictionary_size}')
        print(f'Max match size: {self.__max_match_size}')
        print(f'Index: {self.__idx}')
        print(f"Buf start idx: {self.__buf_start_idx}")
        pprint(self.__tree)
        pprint(f'Look ahead: "{self.__look_ahead_buffer}"')
        pprint(f'Dictionary: "{self.__dictionary}"')

    @staticmethod
    def __find_overlap(left_str: str, right_str: str) -> int:
        for i, (x, y) in enumerate(zip(left_str, right_str)):
            if x != y:
                return i
        return len(left_str)

    def find_match(self) -> tuple[int, int]:
        keys = list(self.__tree.irange(minimum=self.__look_ahead_buffer))
        if not keys:
            return 0, 0
        nearest = keys[0]
        common_length = self.__find_overlap(self.__look_ahead_buffer, nearest)
        if common_length == 0:
            return 0, 0
        position, _ = self.__tree[nearest]
        return position - self.__buf_start_idx, common_length

    def __shift_aux(self, old_buffer: str) -> None:
        if old_buffer in self.__tree:
            idx, count = self.__tree[old_buffer]
            self.__tree[old_buffer] = (idx, count + 1)
        else:
            self.__tree[old_buffer] = (self.__idx, 1)

        if len(self.__dictionary) > self.__dictionary_size:
            invalid_substr = (self.__dictionary + self.__look_ahead_buffer)[:self.__max_match_size]
            self.__dictionary = self.__dictionary[1:]
            pos, count = self.__tree[invalid_substr]
            if count == 1:
                del self.__tree[invalid_substr]
            else:
                self.__tree[invalid_substr] = (pos, count - 1)
            self.__buf_start_idx += 1
        self.__idx += 1

    def shift(self, char: str) -> None:
        old_buffer = self.__look_ahead_buffer
        self.__dictionary += self.__look_ahead_buffer[0]
        self.__look_ahead_buffer = self.__look_ahead_buffer[1:] + char

        self.__shift_aux(old_buffer)

    def shift_empty(self) -> None:
        old_buffer = self.__look_ahead_buffer
        self.__dictionary += self.__look_ahead_buffer[0]
        self.__look_ahead_buffer = self.__look_ahead_buffer[1:]

        self.__shift_aux(old_buffer)



x = "ala ma kota a kot"

tree = BinaryTree(look_ahead_buffer=x[:8], dictionary_size=128, max_match_size=8)

for y in x[8:]:
    tree.shift(y)

for _ in range(5):
    tree.shift_empty()

tree.print()

print(tree.find_match())
