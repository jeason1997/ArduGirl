#!/usr/bin/env python3
"""根据游戏清单生成平台无关的可编译源码快照。"""

from __future__ import annotations

import argparse
import re
import shutil
import tomllib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
HUNK_HEADER = re.compile(r"^@@ -(\d+)(?:,\d+)? \+(\d+)(?:,\d+)? @@")


def find_manifest(game_id: str) -> tuple[Path, dict]:
    for path in sorted((ROOT / "games").rglob("game.toml")):
        with path.open("rb") as stream:
            data = tomllib.load(stream)
        if data.get("id") == game_id:
            return path, data
    raise SystemExit(f"找不到游戏清单：{game_id}")


def find_entry_cpp(manifest: Path) -> Path:
    directory = manifest.parent
    games_root = ROOT / "games"
    while directory != games_root.parent:
        candidate = directory / "entry.cpp"
        if candidate.is_file():
            return candidate
        if directory == games_root:
            break
        directory = directory.parent
    raise SystemExit(f"游戏清单没有对应的 entry.cpp：{manifest}")


def describe(game_id: str, field: str) -> None:
    manifest, data = find_manifest(game_id)
    upstream_entry = ROOT / data["entry"]
    values = {
        "entry_cpp": find_entry_cpp(manifest),
        "include_dir": upstream_entry.parent,
        "generated_entry": ROOT / "build" / "generated" / game_id / upstream_entry.name,
        "manifest": manifest,
    }
    value = values[field]
    print(value.relative_to(ROOT).as_posix())


def apply_patch(directory: Path, patch_path: Path) -> None:
    lines = patch_path.read_text(encoding="utf-8").splitlines()
    index = 0
    while index < len(lines):
        if not lines[index].startswith("--- "):
            index += 1
            continue
        if index + 1 >= len(lines) or not lines[index + 1].startswith("+++ "):
            raise SystemExit(f"补丁文件头无效：{patch_path}")
        target_name = lines[index + 1][4:].split("\t", 1)[0]
        target = directory / target_name
        original = target.read_text(encoding="utf-8").splitlines()
        result: list[str] = []
        source_cursor = 0
        index += 2

        while index < len(lines) and not lines[index].startswith("--- "):
            match = HUNK_HEADER.match(lines[index])
            if match is None:
                index += 1
                continue
            index += 1
            hunk_lines: list[str] = []
            while index < len(lines) and not lines[index].startswith(("@@ ", "--- ")):
                hunk_lines.append(lines[index])
                index += 1

            normalized_hunk = [" " if line == "" else line for line in hunk_lines]
            old_lines = [line[1:] for line in normalized_hunk if line[0] in " -"]
            hunk_start = -1
            for candidate in range(source_cursor, len(original) - len(old_lines) + 1):
                if original[candidate : candidate + len(old_lines)] == old_lines:
                    hunk_start = candidate
                    break
            if hunk_start < 0:
                raise SystemExit(f"补丁上下文不匹配：{patch_path}")
            result.extend(original[source_cursor:hunk_start])
            source_cursor = hunk_start

            for line in normalized_hunk:
                if line == r"\ No newline at end of file":
                    continue
                if line[0] not in " +-":
                    raise SystemExit(f"补丁内容无效：{patch_path}")
                marker, content = line[0], line[1:]
                if marker in " -":
                    if source_cursor >= len(original) or original[source_cursor] != content:
                        raise SystemExit(f"补丁上下文不匹配：{patch_path}:{source_cursor + 1}")
                    if marker == " ":
                        result.append(original[source_cursor])
                    source_cursor += 1
                else:
                    result.append(content)

        result.extend(original[source_cursor:])
        target.write_text("\n".join(result) + "\n", encoding="utf-8", newline="\n")


def prepare(game_id: str) -> None:
    manifest, data = find_manifest(game_id)
    upstream_entry = ROOT / data["entry"]
    if not upstream_entry.is_file():
        raise SystemExit(f"缺少游戏上游入口：{upstream_entry}")

    destination = ROOT / "build" / "generated" / game_id
    temporary = destination.with_name(destination.name + ".tmp")
    if temporary.exists():
        shutil.rmtree(temporary)
    temporary.mkdir(parents=True)

    # 同目录文件按原名复制，保持 Arduino sketch 的相对包含语义。
    for source in upstream_entry.parent.iterdir():
        if source.is_file() and source.suffix.lower() in {".h", ".hpp", ".c", ".cpp", ".ino"}:
            content = source.read_bytes().replace(b"\r\n", b"\n")
            (temporary / source.name).write_bytes(content)

    for relative_patch in data.get("port", {}).get("patches", []):
        patch = (manifest.parent / relative_patch).resolve()
        try:
            apply_patch(temporary, patch)
        except (OSError, UnicodeError):
            shutil.rmtree(temporary)
            raise SystemExit(f"应用补丁失败：{patch}")

    if destination.exists():
        shutil.rmtree(destination)
    temporary.replace(destination)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("game_id")
    parser.add_argument("--field", choices=("entry_cpp", "include_dir", "generated_entry", "manifest"))
    parser.add_argument("--prepare", action="store_true")
    arguments = parser.parse_args()
    if arguments.prepare:
        prepare(arguments.game_id)
    elif arguments.field:
        describe(arguments.game_id, arguments.field)
    else:
        parser.error("必须指定 --field 或 --prepare")


if __name__ == "__main__":
    main()
