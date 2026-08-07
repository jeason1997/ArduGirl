#!/usr/bin/env python3
"""按游戏清单初始化当前构建实际需要的 Git 子模块。"""

from __future__ import annotations

import argparse
import configparser
import subprocess
import sys
import tomllib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
COMMON_SUBMODULES = (Path("third_party/Arduboy2"),)


def load_submodule_paths(root: Path) -> list[Path]:
    """读取受版本控制的子模块路径，并优先匹配最深的嵌套路径。"""
    parser = configparser.ConfigParser()
    gitmodules = root / ".gitmodules"
    if not gitmodules.is_file():
        raise ValueError("仓库缺少 .gitmodules")
    parser.read(gitmodules, encoding="utf-8")
    paths = [Path(parser[section]["path"]) for section in parser.sections()]
    return sorted(paths, key=lambda path: len(path.parts), reverse=True)


def find_manifests(root: Path, selected_game: str | None) -> list[dict]:
    """只返回选中游戏的清单；未选择时返回聚合构建的全部清单。"""
    manifests: list[dict] = []
    for path in sorted((root / "games").rglob("game.toml")):
        with path.open("rb") as stream:
            data = tomllib.load(stream)
        if selected_game is None or data.get("id") == selected_game:
            manifests.append(data)
    if selected_game is not None and not manifests:
        raise ValueError(f"未找到游戏：{selected_game}")
    return manifests


def required_submodules(root: Path, selected_game: str | None) -> list[Path]:
    """把公共源码和游戏入口映射到最小子模块集合。"""
    submodules = load_submodule_paths(root)
    required = set(COMMON_SUBMODULES)
    for manifest in find_manifests(root, selected_game):
        entry = Path(manifest["entry"])
        owner = next((path for path in submodules if entry == path or path in entry.parents), None)
        if owner is not None:
            required.add(owner)
    unknown = required.difference(submodules)
    if unknown:
        names = ", ".join(path.as_posix() for path in sorted(unknown))
        raise ValueError(f"清单依赖不是已登记的子模块：{names}")
    return sorted(required)


def ensure(root: Path, selected_game: str | None) -> None:
    """仅在工作树缺少所需文件时初始化对应子模块。"""
    required = required_submodules(root, selected_game)
    missing = [path for path in required if not (root / path).is_dir() or not any((root / path).iterdir())]
    if not missing:
        return
    print(
        "正在初始化当前构建需要的子模块："
        + ", ".join(path.as_posix() for path in missing),
        file=sys.stderr,
    )
    subprocess.run(
        ["git", "-C", str(root), "submodule", "update", "--init", "--recursive", "--", *(path.as_posix() for path in missing)],
        check=True,
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--game")
    arguments = parser.parse_args()
    try:
        ensure(ROOT, arguments.game)
    except (OSError, ValueError, subprocess.CalledProcessError, tomllib.TOMLDecodeError) as error:
        print(f"无法准备游戏子模块：{error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
