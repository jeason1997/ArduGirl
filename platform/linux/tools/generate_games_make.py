#!/usr/bin/env python3
"""从游戏清单生成 Linux 平台使用的 Make 元数据。"""

from __future__ import annotations

import argparse
import sys
import tomllib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools"))

import prepare_game


def make_path(path: Path) -> str:
    """输出相对仓库根目录的 Make 路径，避免平台文件包含开发机绝对路径。"""
    return path.relative_to(ROOT).as_posix()


def make_value(value: object) -> str:
    """拒绝会破坏生成文件语法的值，并保留普通编译参数。"""
    text = str(value)
    if "\n" in text or "\r" in text:
        raise ValueError(f"清单字段不能包含换行：{text!r}")
    return text.replace("$", "$$").replace("#", r"\#")


def discover_manifests(selected_game: str | None) -> list[tuple[Path, dict]]:
    """自动发现清单；单游戏构建只解析被选择的一个清单。"""
    manifests: list[tuple[Path, dict]] = []
    seen: set[str] = set()
    for path in sorted((ROOT / "games").rglob("game.toml")):
        with path.open("rb") as stream:
            data = tomllib.load(stream)
        game_id = data.get("id")
        if not isinstance(game_id, str) or not game_id:
            raise ValueError(f"游戏清单缺少有效 id：{make_path(path)}")
        if game_id in seen:
            raise ValueError(f"游戏编号重复：{game_id}")
        seen.add(game_id)
        if selected_game is None or selected_game == game_id:
            manifests.append((path, data))
    if selected_game is not None and not manifests:
        raise ValueError(f"未找到 Linux 游戏：{selected_game}")
    if not manifests:
        raise ValueError("没有发现任何游戏清单")
    return manifests


def source_paths(entry: Path, suffix: str, excluded: set[str]) -> list[str]:
    """按 Arduino sketch 目录递归发现编译单元，并映射到生成快照。"""
    result: list[str] = []
    for source in sorted(entry.parent.rglob(f"*{suffix}")):
        if source.name in excluded:
            continue
        result.append(source.relative_to(entry.parent).as_posix())
    return result


def emit_game(path: Path, data: dict) -> list[str]:
    game_id = data["id"]
    entry = ROOT / data["entry"]
    if not entry.is_file():
        raise ValueError(f"缺少游戏上游入口：{make_path(entry)}")
    build_path, build = prepare_game.load_build_config(path)
    excluded = set(build.get("exclude_sources", []))
    generated_dir = Path("build") / "generated" / game_id
    patches = [path.parent / item for item in data.get("port", {}).get("patches", [])]
    test = data.get("test", {})
    replay_source = test.get("replay_source", "")
    terminal = test.get("terminal", False)
    cpp_sources = [
        (generated_dir / source).as_posix()
        for source in source_paths(entry, ".cpp", excluded)
    ]
    c_sources = [
        (generated_dir / source).as_posix()
        for source in source_paths(entry, ".c", set())
    ]
    flags = list(build.get("cpp_flags", []))
    for condition in build.get("defines_if_contains", []):
        source = entry.parent / condition["file"]
        if source.is_file() and condition["text"] in source.read_text(encoding="utf-8"):
            flags.append(f'-D{condition["define"]}')

    values: dict[str, object] = {
        "MANIFEST": make_path(path),
        "BUILD_CONFIG": make_path(build_path),
        "UPSTREAM_ENTRY": make_path(entry),
        "SNAPSHOT_SOURCES": " ".join(
            make_path(source) for source in sorted(entry.parent.rglob("*")) if source.is_file()
        ),
        "GENERATED_ENTRY": (generated_dir / entry.name).as_posix(),
        "ENTRY_CPP": (generated_dir / "ardugirl_entry.cpp").as_posix(),
        "CPP_SOURCES": " ".join(cpp_sources),
        "C_SOURCES": " ".join(c_sources),
        "EXTRA_CPP_SOURCES": " ".join(build.get("extra_cpp_sources", [])),
        "CPP_FLAGS": " ".join(flags),
        "PATCHES": " ".join(make_path(item) for item in patches),
        "SMOKE_FRAMES": test.get("smoke_frames", 180),
        "REPLAY_SOURCE": replay_source,
        "TERMINAL": "1" if terminal else "",
    }
    lines = [f"# {game_id} 的字段来自 {make_path(path)}。"]
    lines.extend(f"GAME_{name}_{game_id} := {make_value(value)}" for name, value in values.items())
    return lines


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--game")
    arguments = parser.parse_args()
    try:
        manifests = discover_manifests(arguments.game)
        game_ids = [data["id"] for _, data in manifests]
        lines = ["# 此文件由游戏清单自动生成，请勿手工修改。", f"GAME_IDS := {' '.join(game_ids)}", ""]
        for path, data in manifests:
            lines.extend(emit_game(path, data))
            lines.append("")
        content = "\n".join(lines)
        output = arguments.output if arguments.output.is_absolute() else ROOT / arguments.output
        output.parent.mkdir(parents=True, exist_ok=True)
        if not output.is_file() or output.read_text(encoding="utf-8") != content:
            output.write_text(content, encoding="utf-8", newline="\n")
    except (OSError, ValueError, tomllib.TOMLDecodeError) as error:
        print(f"无法生成 Linux 游戏目录：{error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
