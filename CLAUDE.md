# goldendict-ng fork (wuyun-1921)

If we are doing general changes, like CI, .gitignored etc, do it in wy-patches. Never directly make changes in wy-main.

Example workflow: reset wy-main to upstream/staged -> make changes in wy-patches, feat branches -> merged them into wy-main -> release.
