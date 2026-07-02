# Dict Panel Height Limit & Side-by-Side Panels

## Overview

Two patches to GoldenDict-ng:

1. **Dict panel height limit** — each dictionary article gets a configurable max-height with internal scroll. Left/right split-zone on the article: hover left → page scrolls, hover right → article scrolls internally.
2. **Side-by-side panels** — multiple ArticleView widgets in a QSplitter. Tabs toggle between panel mode (visible in splitter) and background tab mode (in tab bar).

## Patch 1: Dict Panel Height Limit

### Settings

Group `dictPanel` in Preferences → Articles:

| Setting | Default | Options |
|---------|---------|---------|
| `dictPanel/enabled` | `true` | checkbox |
| `dictPanel/maxHeight` | `300` | integer |
| `dictPanel/heightUnit` | `px` | px, em, vh |
| `dictPanel/scrollZoneSplit` | `50` | 0–100 (%) |

0% = cursor always scrolls page. 100% = cursor always scrolls article.

### CSS

`ArticleMaker::makeHtmlHeader()` injects:

```css
.gdarticlebody {
  max-height: var(--gd-panel-height); /* e.g. 300px */
  overflow-y: auto;
}
```

### JS (gd-builtin.js)

Wheel event handler on each `.gdarticlebody`:

```
article.scrollWidth * (splitPercent / 100) → left zone
cursorX < left zone width → e.stopPropagation()
cursorX >= left zone width → default (scroll inside article)
```

`splitPercent` received from C++ via QWebChannel → `ArticleViewAgent.setScrollZoneSplit(pct)`.

### Files touched

- `src/article_maker.cc` — inject CSS variable
- `src/stylesheets/article-style.css` — max-height rule
- `src/scripts/gd-builtin.js` — wheel zone handler
- `src/ui/articleview.hh/.cc` — QWebChannel property for split percent
- Preferences UI (settings .ui + .cc) — settings group

---

## Patch 2: Side-by-Side Panels

### Architecture

`MainWindow` central widget layout changes from `QHBoxLayout → MainTabWidget` to `QHBoxLayout → MainTabWidget | QSplitter`. The splitter is hidden when no panels are pinned.

```
QMainWindow
  └── centralWidget (QHBoxLayout)
        ├── QSplitter (panel container, initially hidden)
        │     ├── ArticleView (pinned tab 1)
        │     ├── ArticleView (pinned tab 2)
        │     └── ...
        └── MainTabWidget (background tabs)
```

Each `ArticleView` in the splitter is a full independent web view — it looks up its own word, group, and dictionaries. Panels degrade gracefully to background tabs (all existing functionality preserved).

### Tab → Panel Toggle

`Ctrl+Shift+P` moves the current tab between panel and background:

```
If current tab is in MainTabWidget:
  1. Remove ArticleView from MainTabWidget
  2. Add to QSplitter (last position)
  3. Focus the new panel

If current tab is in QSplitter:
  1. Remove ArticleView from QSplitter
  2. Add as new tab to MainTabWidget
  3. Show MainTabWidget if it was hidden
  4. Focus the new tab
```

Right-click tab context menu adds "Move to panel / Move to tab bar".

### New Panel / Split

`Ctrl+Shift+H` or `Ctrl+Shift+V`:

- If no panels exist: create two panels in the splitter (horizontal or vertical), move current tab to one, clone empty panel for the other.
- If panels exist: toggle splitter orientation between horizontal and vertical.

`View` menu adds:
- `Split Panel Horizontally` (`Ctrl+Shift+H`)
- `Split Panel Vertically` (`Ctrl+Shift+V`)
- `Close Panel` (right-click panel tab bar — uses existing close action)

### Settings

Group `sideBySide`:

| Setting | Default | Options |
|---------|---------|---------|
| `sideBySide/orientation` | `horizontal` | horizontal, vertical |
| `sideBySide/defaultSplit` | `50` | 10–90 (%) |

### State Persistence

On close: save splitter sizes, orientation, and which tabs were pinned to panels. On reopen: restore panel layout.

### Files touched

- `src/ui/mainwindow.ui` — add QSplitter to layout
- `src/ui/mainwindow.hh/.cc` — panel management (createPanel, removePanel, togglePanel)
- `src/ui/maintabwidget.hh/.cc` — context menu "Move to panel"
- Preferences UI — new settings group

---

## Self-Review

- No placeholders or TODOs
- Internal consistency: Patch 1 CSS/JS changes are self-contained. Patch 2 splitter is orthogonal to Patch 1 — panels use the same ArticleView and inherit Patch 1 behavior automatically.
- Scope focused: two independent patches, same codebase, no architectural changes to article rendering pipeline.
- Ambiguity: split-zone percentage is clear (0=page, 100=article). Panel hotkey toggle behavior is explicit.
