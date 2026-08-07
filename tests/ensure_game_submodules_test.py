#!/usr/bin/env python3
"""验证游戏清单只展开当前构建需要的子模块。"""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import ensure_game_submodules


class RequiredSubmodulesTest(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        (self.root / "games" / "one").mkdir(parents=True)
        (self.root / "games" / "two").mkdir(parents=True)
        (self.root / ".gitmodules").write_text(
            '[submodule "Arduboy2"]\n\tpath = third_party/Arduboy2\n\turl = example.invalid/common\n'
            '[submodule "One"]\n\tpath = third_party/One\n\turl = example.invalid/one\n'
            '[submodule "Two"]\n\tpath = third_party/Two\n\turl = example.invalid/two\n',
            encoding="utf-8",
        )
        (self.root / "games" / "one" / "game.toml").write_text(
            'id = "one"\nentry = "third_party/One/one.ino"\n', encoding="utf-8"
        )
        (self.root / "games" / "two" / "game.toml").write_text(
            'id = "two"\nentry = "third_party/Two/two.ino"\n', encoding="utf-8"
        )

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def test_selected_game_excludes_other_upstream(self) -> None:
        self.assertEqual(
            ensure_game_submodules.required_submodules(self.root, "one"),
            [Path("third_party/Arduboy2"), Path("third_party/One")],
        )

    def test_aggregate_includes_all_upstreams(self) -> None:
        self.assertEqual(
            ensure_game_submodules.required_submodules(self.root, None),
            [Path("third_party/Arduboy2"), Path("third_party/One"), Path("third_party/Two")],
        )


if __name__ == "__main__":
    unittest.main()
