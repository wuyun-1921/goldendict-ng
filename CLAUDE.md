# goldendict-ng fork (wuyun-1921)

## Upstream
- **upstream remote**: `https://github.com/xiaoyifang/goldendict-ng`
- **Main dev branch**: `staged` (PRs target this)

## Branch Layout

| Branch | Purpose |
|---|---|
| `wy-main` | **Default branch.** Release branch. Built from upstream/staged + wy-patches + feat/* branches merged in. |
| `wy-patches` | General changes: CI workflows, .gitignore, project config, docs. Never commit directly to wy-main — always go through wy-patches. |
| `feat/dict-panels` | Multi-panel feature (PR #3001 on upstream). Side-by-side dictionary panels. |
| `feat/always-query` | Always Query forwarding per tab. |
| `feat/session-store` | Session save/restore with per-tab collapsed dictionary state. |
| `feat/article-extras` | Article extras: scroll zones, article height limit, display CSS. |
| `staged` | Tracking upstream/staged. |

## Workflow

1. Reset wy-main to upstream/staged
2. Make changes in wy-patches, feat branches
3. Merge them into wy-main
4. Release from wy-main

If we are doing general changes, like CI, .gitignored etc, do it in wy-patches. Never directly make changes in wy-main.
