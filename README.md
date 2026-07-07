# GoldenDict-ng wy fork

A [goldendict-ng](https://xiaoyifang.github.io/goldendict-ng/developer/) fork with extra features. Main dev branch is `wy-dev`.

## Features added on top of upstream

### Multi-panel (side-by-side)
- Side-by-side dictionary panels — split the window and view multiple tabs at once.
- Move tabs between panels via context menu.
- Move tab between first panel and a new panel with `Ctrl+Shift+P`.
- Toggle panel orientation (horizontal/vertical) with `Ctrl+Shift+H`.

### Always Query
- Per-tab "Always Query" flag — when enabled, every word lookup also queries this tab.
- `Ctrl+Shift+E` toggles Always Query on the current tab.
- Context menu: "Always Query This Tab" checkbox.
- `[A]` marker on tab titles, dimmed tab text color for always-query tabs.

### Article display extras
- Scroll zone split: middle of article scrolls within article, outer edges scroll the page.
- Configurable scroll zone width (0–100%).
- Reverse scroll zone toggle — swap which zone scrolls what.
- Per-dictionary max height with collapse/expand.

### Session save/restore
- Tabs, panels, layout, always-query flags, and collapsed dictionary state persist across restarts.
- Lazy loading: inactive tabs defer content loading until first visit.
- Configurable via "Restore session on startup" preference.
