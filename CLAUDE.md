# goldendict-ng fork (wuyun-1921)


## Branch Layout

| Branch | Purpose |
|---|---|
| `wy-main` | **Default branch.** Release branch. Rebuilt by CI from upstream/staged + feat/dict-panels + feat/article-extras + wy-patches. |
| `wy-patches` | Unique files not covered by feature branches: CI workflow (`wy-main-release.yml`), `MainTabWidget::moveTabToPanelRequested` signal. |

---

If we are doing general changes, like CI, .gitignored etc, do it in wy-patches. Never directly make changes in wy-main.

Example workflow: reset wy-main to upstream/staged -> make changes in wy-patches, feat branches -> merged them into wy-main -> release.
