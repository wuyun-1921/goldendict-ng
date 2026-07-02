Each panel is now a QTabWidget with closable, movable tabs.

## New Architecture

```
QSplitter (panelSplitter)
  ├── QTabWidget (panel — closable, movable tabs)
  │     ├── ArticleView (tab 1)
  │     ├── ArticleView (tab 2, website, linked to main)
  │     └── ArticleView (tab 3)
  └── QTabWidget (panel)
        └── ArticleView (tab 1)
```

Main TabWidget → same as before (background tabs)

## Behaviors

| Action | Key | Behavior |
|--------|-----|----------|
| Toggle tab ↔ panel | `Ctrl+Shift+P` | Move current tab between main tab bar and first panel |
| Close panel tab | `Ctrl+Shift+W` | Close current panel tab. Last tab → close panel |
| Toggle orientation | `Ctrl+Shift+H` | Horizontal / vertical split |
| Tab reorder | Mouse drag | TabBar → Movable, can drag between panels |
| Close tab button | `×` on tab | Close individual tab. Last → close panel |
| Website linked | — | Stays linked to main tab even when in panel |

## Implementation Notes

- Panels resize proportionally with window (stretch factor)
- When last tab of a panel closes, the panel widget is removed from splitter
- Remaining panel(s) reoccupy full space
- If all panels closed, splitter hides
