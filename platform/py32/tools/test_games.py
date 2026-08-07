#!/usr/bin/env python3
"""自动发现全部游戏并逐个执行 PY32 冷构建。"""

from __future__ import annotations

import shutil
import subprocess
import sys
import tomllib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
PY32_MAKEFILE = "platform/py32/Makefile"


def discover_games() -> list[str]:
    """从清单读取游戏编号，避免在构建规则中维护游戏白名单。"""
    games: list[str] = []
    seen: set[str] = set()
    for manifest in sorted((ROOT / "games").rglob("game.toml")):
        with manifest.open("rb") as stream:
            game_id = tomllib.load(stream).get("id")
        if not isinstance(game_id, str) or not game_id:
            raise RuntimeError(f"游戏清单缺少有效 id：{manifest.relative_to(ROOT)}")
        if game_id in seen:
            raise RuntimeError(f"游戏编号重复：{game_id}")
        seen.add(game_id)
        games.append(game_id)
    if not games:
        raise RuntimeError("没有发现任何游戏清单")
    return games


def build_game(game_id: str) -> bool:
    """只清理当前游戏的产物，并用独立 make 进程执行完整构建。"""
    print(f"\n===== PY32 冷构建：{game_id} =====", flush=True)
    # clean 与 all 作为同一次 make 的并列目标时，会基于清理前的旧产物计算依赖图，
    # 可能跳过随后已被删除的对象；拆成两个进程才能保证每款游戏都是真正冷构建。
    clean = subprocess.run(
        ["make", "-f", PY32_MAKEFILE, f"GAME={game_id}", "clean"],
        cwd=ROOT,
        check=False,
    )
    if clean.returncode != 0:
        return False
    # 仅清理平台输出不足以覆盖准备阶段；删除精确的游戏快照，确保测试能发现生成依赖竞态。
    generated = ROOT / "build" / "generated" / game_id
    if generated.exists():
        shutil.rmtree(generated)
    result = subprocess.run(
        ["make", "-f", PY32_MAKEFILE, f"GAME={game_id}", "-j4", "all"],
        cwd=ROOT,
        check=False,
    )
    return result.returncode == 0


def main() -> int:
    try:
        games = discover_games()
    except (OSError, RuntimeError, tomllib.TOMLDecodeError) as error:
        print(f"PY32 编译测试无法开始：{error}", file=sys.stderr)
        return 2

    failures = [game_id for game_id in games if not build_game(game_id)]
    passed = len(games) - len(failures)
    print(f"\nPY32 编译测试完成：通过 {passed}/{len(games)}。", flush=True)
    if failures:
        print(f"构建失败：{', '.join(failures)}", file=sys.stderr, flush=True)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
