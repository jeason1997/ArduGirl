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


def load_build_config(manifest: Path) -> tuple[Path, dict]:
    """读取集合共有构建描述；普通独立游戏返回空描述。"""
    path = manifest.parent.parent / "build.toml"
    if not path.is_file():
        return manifest, {}
    with path.open("rb") as stream:
        return path, tomllib.load(stream)


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
    build_config_path, build_config = load_build_config(manifest)
    excluded_sources = set(build_config.get("exclude_sources", []))
    cpp_flags: list[str] = list(build_config.get("cpp_flags", []))
    for condition in build_config.get("defines_if_contains", []):
        source = upstream_entry.parent / condition["file"]
        if source.is_file() and condition["text"] in source.read_text(encoding="utf-8"):
            cpp_flags.append(f'-D{condition["define"]}')
    values = {
        "entry_cpp": ROOT / "build" / "generated" / game_id / "ardugirl_entry.cpp",
        "include_dir": upstream_entry.parent,
        "generated_entry": ROOT / "build" / "generated" / game_id / upstream_entry.name,
        "manifest": manifest,
        "build_config": build_config_path,
        "cpp_sources": " ".join(
            (ROOT / "build" / "generated" / game_id / path.relative_to(upstream_entry.parent)).relative_to(ROOT).as_posix()
            for path in sorted(upstream_entry.parent.rglob("*.cpp"))
            if path.name not in excluded_sources
        ),
        "c_sources": " ".join(
            (ROOT / "build" / "generated" / game_id / path.relative_to(upstream_entry.parent)).relative_to(ROOT).as_posix()
            for path in sorted(upstream_entry.parent.rglob("*.c"))
        ),
        "extra_cpp_sources": " ".join(build_config.get("extra_cpp_sources", [])),
        "cpp_flags": " ".join(cpp_flags),
    }
    value = values[field]
    print(value.relative_to(ROOT).as_posix() if isinstance(value, Path) else value)


def resolve_patch_target(directory: Path, target_name: str, patch_path: Path) -> Path:
    """把上游仓库路径解析到源码快照内，并拒绝不唯一的模糊匹配。"""
    parts = Path(target_name).parts
    for offset in range(len(parts)):
        candidate = directory.joinpath(*parts[offset:])
        if candidate.is_file():
            return candidate
    raise OSError(f"补丁目标不在源码快照中：{patch_path}：{target_name}")


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
        target = resolve_patch_target(directory, target_name, patch_path)
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

    # Arduino 会共同编译 sketch 目录内的源码；递归快照同时覆盖把实现放在 src/ 的库式游戏。
    for source in upstream_entry.parent.rglob("*"):
        if source.is_file():
            relative = source.relative_to(upstream_entry.parent)
            target = temporary / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            content = source.read_bytes()
            if source.suffix.lower() in {".h", ".hpp", ".c", ".cpp", ".ino"}:
                content = content.replace(b"\r\n", b"\n")
            target.write_bytes(content)

    # 集合可用声明式替换描述共同的上游源码差异，避免把规则复制到每个平台。
    build_config_path, build_config = load_build_config(manifest)
    if build_config:
        replacements = build_config.get("replacements", [])
        for replacement in replacements:
            target = temporary / replacement["file"].format(game_id=game_id)
            if not target.is_file() and replacement.get("optional", False):
                continue
            content = target.read_text(encoding="utf-8")
            old = replacement["old"]
            new = replacement["new"].format(game_id=game_id)
            if replacement.get("regex", False):
                updated, count = re.subn(old, lambda _: new, content, count=1, flags=re.MULTILINE)
            else:
                count = 1 if old in content else 0
                updated = content.replace(old, new, 1)
            if count == 0:
                if replacement.get("optional", False):
                    continue
                raise SystemExit(f"集合源码替换上下文不匹配：{build_config_path}：{target.name}")
            target.write_text(updated, encoding="utf-8", newline="\n")

    for relative_patch in data.get("port", {}).get("patches", []):
        patch = (manifest.parent / relative_patch).resolve()
        try:
            apply_patch(temporary, patch)
        except (OSError, UnicodeError):
            shutil.rmtree(temporary)
            raise SystemExit(f"应用补丁失败：{patch}")

    # 统一入口避免平台构建理解游戏集合的私有宏，并确保实际编译的是已应用补丁的快照。
    wrapper = temporary / "ardugirl_entry.cpp"
    wrapper.write_text(
        f'#include "{upstream_entry.name}"\n\n'
        f'const char* ardugirl_game_id() noexcept {{ return "{game_id}"; }}\n'
        f'const char* ardugirl_game_title() noexcept {{ return "{data.get("name", game_id)}"; }}\n',
        encoding="utf-8",
        newline="\n",
    )

    # 复制会保留上游时间戳；刷新主入口，避免它仍早于准备器而触发重复生成。
    (temporary / upstream_entry.name).touch()

    if destination.exists():
        shutil.rmtree(destination)
    # Windows 不允许 os.replace 用目录替换目录；上面已显式移除目标，因此普通重命名即可。
    temporary.rename(destination)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("game_id")
    parser.add_argument("--field", choices=("entry_cpp", "include_dir", "generated_entry", "manifest", "build_config", "cpp_sources", "c_sources", "extra_cpp_sources", "cpp_flags"))
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
