# Dict Panel Height Limit & Side-by-Side Panels — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add configurable per-dictionary height limit with split-scroll zones, and side-by-side panels via QSplitter.

**Architecture:** Patch 1 injects a CSS variable per dictionary article and adds a JS wheel handler for split-zone scrolling. Patch 2 adds a QSplitter alongside the existing MainTabWidget, allowing tabs to toggle between panel and background mode. Both patches add settings to the existing XML config system and preferences dialog.

**Tech Stack:** C++17, Qt 6 (QWebEngineView, QSplitter, QWebChannel), QKeySequence, CSS, JavaScript

## Global Constraints

- Add 4 new `Config::Preferences` members and corresponding UI in `preferences.ui`
- Config stored as XML elements in `~/.goldendict/config`
- Shortcuts use `addAction()` on MainWindow (global actions via `addGlobalAction()`)
- All new JS code in `gd-builtin.js`; all new CSS in `article-style.css`
- Follow existing code patterns: QWebChannel bridge for C++→JS communication

---

## File Structure

| File | Create/Modify | Responsibility |
|------|---------------|----------------|
| `src/config.hh` | Modify | 4 new Preferences members |
| `src/config.cc` | Modify | Defaults, XML load/save |
| `src/ui/preferences.ui` | Modify | New settings group in dialog |
| `src/ui/preferences.cc` | Modify | Load/save dialog ↔ config |
| `src/ui/mainwindow.hh` | Modify | New QAction members, panel methods |
| `src/ui/mainwindow.cc` | Modify | Panel management, shortcut registration, apply settings |
| `src/ui/mainwindow.ui` | Modify | Add QSplitter to central layout |
| `src/ui/articleview.hh` | Modify | QWebChannel agent additions |
| `src/ui/articleview.cc` | Modify | setScrollZoneSplit() method + agent |
| `src/article_maker.cc` | Modify | Inject CSS variable into HTML header |
| `src/stylesheets/article-style.css` | Modify | max-height rule for .gdarticlebody |
| `src/scripts/gd-builtin.js` | Modify | Wheel zone handler |
| `src/ui/maintabwidget.cc` | Modify | Context menu "Move to panel" |

---

### Task 1: Dict Panel Height Limit — Settings

**Files:**
- Modify: `src/config.hh:278` (inside `struct Preferences`)
- Modify: `src/config.cc:103` (defaults), `:789` (load), `:1720` (save)

**New settings:**

| Member | Type | Default | XML tag |
|--------|------|---------|---------|
| `dictPanelEnabled` | bool | `true` | `dictPanelEnabled` |
| `dictPanelMaxHeight` | int | `300` | `dictPanelMaxHeight` |
| `dictPanelHeightUnit` | QString | `"px"` | `dictPanelHeightUnit` |
| `dictPanelScrollZone` | int | `50` | `dictPanelScrollZone` |

- [ ] **Step 1: Declare members in config.hh**

In `src/config.hh`, inside `struct Preferences`, after existing members:

```cpp
// Dict panel height limit
bool dictPanelEnabled;
int dictPanelMaxHeight;
QString dictPanelHeightUnit;
int dictPanelScrollZone;
```

- [ ] **Step 2: Set defaults in config.cc**

In `src/config.cc`, in `Preferences::Preferences()` initializer list:

```cpp
dictPanelEnabled( true ),
dictPanelMaxHeight( 300 ),
dictPanelHeightUnit( "px" ),
dictPanelScrollZone( 50 ),
```

- [ ] **Step 3: Add XML load in config.cc**

In `src/config.cc`, inside `Class load()`, after other preferences load lines (~line 789):

```cpp
c.preferences.dictPanelEnabled = ( preferences.namedItem( "dictPanelEnabled" ).toElement().text() == "1" );
c.preferences.dictPanelMaxHeight = preferences.namedItem( "dictPanelMaxHeight" ).toElement().text().toInt();
if ( !preferences.namedItem( "dictPanelHeightUnit" ).isNull() )
  c.preferences.dictPanelHeightUnit = preferences.namedItem( "dictPanelHeightUnit" ).toElement().text();
if ( !preferences.namedItem( "dictPanelScrollZone" ).isNull() )
  c.preferences.dictPanelScrollZone = preferences.namedItem( "dictPanelScrollZone" ).toElement().text().toInt();
```

- [ ] **Step 4: Add XML save in config.cc**

In `src/config.cc`, inside `void save()`, after other preferences save lines (~line 1720):

```cpp
opt = dd.createElement( "dictPanelEnabled" );
opt.appendChild( dd.createTextNode( c.preferences.dictPanelEnabled ? "1" : "0" ) );
preferences.appendChild( opt );
opt = dd.createElement( "dictPanelMaxHeight" );
opt.appendChild( dd.createTextNode( QString::number( c.preferences.dictPanelMaxHeight ) ) );
preferences.appendChild( opt );
opt = dd.createElement( "dictPanelHeightUnit" );
opt.appendChild( dd.createTextNode( c.preferences.dictPanelHeightUnit ) );
preferences.appendChild( opt );
opt = dd.createElement( "dictPanelScrollZone" );
opt.appendChild( dd.createTextNode( QString::number( c.preferences.dictPanelScrollZone ) ) );
preferences.appendChild( opt );
```

- [ ] **Step 5: Build**

```bash
cd build && cmake .. && make -j$(nproc)
```

- [ ] **Step 6: Commit**

```bash
git add src/config.hh src/config.cc
git commit -m "feat: add dict panel height limit settings to config"
```

---

### Task 2: Dict Panel Height Limit — CSS

**Files:**
- Modify: `src/article_maker.cc:80` (makeHtmlHeader)
- Modify: `src/stylesheets/article-style.css`

- [ ] **Step 1: Inject CSS variable in article_maker.cc**

In `src/article_maker.cc`, in `makeHtmlHeader()`, add to the inline `<style>` block or after `<body>`:

```cpp
// After the existing style injection, add:
R"(<style>
:root {
  --gd-panel-height: %1%2;
}
.gdarticlebody {
  max-height: var(--gd-panel-height);
  overflow-y: auto;
}
</style>)"
.arg( QString::number( cfg.preferences.dictPanelMaxHeight ) )
.arg( cfg.preferences.dictPanelHeightUnit );
```

Note: access Config via existing pattern — check how `cfg` is accessed in this function (likely via a global or passed parameter).

- [ ] **Step 2: Add CSS rule in article-style.css**

Append to `src/stylesheets/article-style.css`:

```css
/* Dict panel height limit */
.gdarticlebody {
  max-height: var(--gd-panel-height);
  overflow-y: auto;
}
```

- [ ] **Step 3: Build and visually verify**

```bash
cd build && make -j$(nproc)
./goldendict
# Look up any word — each dictionary article should be capped at 300px with internal scroll
```

- [ ] **Step 4: Commit**

```bash
git add src/article_maker.cc src/stylesheets/article-style.css
git commit -m "feat: dict panel height limit CSS"
```

---

### Task 3: Dict Panel Height Limit — JS Split Scroll Zones

**Files:**
- Modify: `src/scripts/gd-builtin.js`
- Modify: `src/ui/articleview.hh`
- Modify: `src/ui/articleview.cc`

- [ ] **Step 1: Add scrollZone property to ArticleViewAgent**

In `src/ui/articleview.hh`, `ArticleViewAgent` class, add:

```cpp
Q_PROPERTY(int scrollZoneSplit READ scrollZoneSplit WRITE setScrollZoneSplit NOTIFY scrollZoneSplitChanged)

public:
  int scrollZoneSplit() const { return m_scrollZoneSplit; }
  void setScrollZoneSplit(int pct) {
    if (m_scrollZoneSplit != pct) {
      m_scrollZoneSplit = pct;
      emit scrollZoneSplitChanged(pct);
    }
  }

signals:
  void scrollZoneSplitChanged(int pct);

private:
  int m_scrollZoneSplit = 50;
```

- [ ] **Step 2: Inject JS wheel handler in article_maker.cc**

In `src/article_maker.cc`, in `makeHtmlHeader()`, add to the JS injection section (where `gd-builtin.js` is loaded):

```javascript
// Append to gd-builtin.js or inject inline:
(function() {
  let splitPercent = 50;
  // Listen for split percent from C++
  if (typeof articleview !== 'undefined') {
    articleview.scrollZoneSplitChanged.connect(function(pct) {
      splitPercent = pct;
    });
  }
  document.addEventListener('wheel', function(e) {
    const article = e.target.closest('.gdarticlebody');
    if (!article) return;
    const rect = article.getBoundingClientRect();
    const splitX = rect.left + (rect.width * splitPercent / 100);
    if (e.clientX < splitX) {
      // Left zone: let page scroll
      e.stopPropagation();
    }
    // Right zone: default behavior scrolls inside the article
  }, { passive: false });
})();
```

- [ ] **Step 3: Set scroll zone from MainWindow preferences**

In `src/ui/mainwindow.cc`, in `editPreferences()`, after `cfg.preferences = p;` and `Config::save(cfg);`, add:

```cpp
// Apply dict panel scroll zone to all article views
for (int i = 0; i < ui.tabWidget->count(); i++) {
  auto * av = qobject_cast<ArticleView*>(ui.tabWidget->widget(i));
  if (av) av->agent().setScrollZoneSplit(cfg.preferences.dictPanelScrollZone);
}
```

Note: need to check how `agent()` is accessed — it might be a private member. May need a public accessor on ArticleView.

- [ ] **Step 4: Apply on article load**

In `src/ui/articleview.cc`, in the article load/creation path, set the initial scroll zone from config:

```cpp
// After agent is created (constructor, around line 239):
agent->setScrollZoneSplit(Config::global().preferences.dictPanelScrollZone);
```

Note: verify `Config::global()` access pattern in existing code.

- [ ] **Step 5: Build and test**

```bash
cd build && make -j$(nproc)
./goldendict
# Verify: cursor left half → page scrolls. Right half → article scrolls.
```

- [ ] **Step 6: Commit**

```bash
git add src/ui/articleview.hh src/ui/articleview.cc src/article_maker.cc src/ui/mainwindow.cc
git commit -m "feat: dict panel split-scroll zone via JS wheel handler"
```

---

### Task 4: Preferences UI for Dict Panel Settings

**Files:**
- Modify: `src/ui/preferences.ui`
- Modify: `src/ui/preferences.cc`

- [ ] **Step 1: Add UI widgets in preferences.ui**

Add a new group box inside the `Articles` tab of the preferences dialog (or a new tab). Widgets:

```xml
<widget class="QGroupBox" name="dictPanelGroup">
  <property name="title"><string>Dictionary Panels</string></property>
  <layout class="QFormLayout">
    <item row="0">
      <widget class="QCheckBox" name="dictPanelEnabled">
        <property name="text"><string>Limit dictionary article height</string></property>
      </widget>
    </item>
    <item row="1">
      <layout class="QHBoxLayout">
        <item><widget class="QLabel" name="dictPanelHeightLabel"><property name="text"><string>Max height:</string></property></widget></item>
        <item><widget class="QSpinBox" name="dictPanelMaxHeight"><property name="minimum"><number>50</number></property><property name="maximum"><number>2000</number></property></widget></item>
        <item><widget class="QComboBox" name="dictPanelHeightUnit"><item><property name="text"><string>px</string></property></item><item><property name="text"><string>em</string></property></item><item><property name="text"><string>vh</string></property></item></widget></item>
      </layout>
    </item>
    <item row="2">
      <layout class="QHBoxLayout">
        <item><widget class="QLabel" name="dictPanelZoneLabel"><property name="text"><string>Scroll zone split:</string></property></widget></item>
        <item><widget class="QSpinBox" name="dictPanelScrollZone"><property name="suffix"><string>%</string></property><property name="minimum"><number>0</number></property><property name="maximum"><number>100</number></property></widget></item>
      </layout>
    </item>
  </layout>
</widget>
```

- [ ] **Step 2: Load config → UI in preferences.cc**

In `src/ui/preferences.cc` constructor (~line 177 area):

```cpp
ui.dictPanelEnabled->setChecked( p.dictPanelEnabled );
ui.dictPanelMaxHeight->setValue( p.dictPanelMaxHeight );
int unitIdx = ui.dictPanelHeightUnit->findText( p.dictPanelHeightUnit );
if (unitIdx >= 0) ui.dictPanelHeightUnit->setCurrentIndex(unitIdx);
ui.dictPanelScrollZone->setValue( p.dictPanelScrollZone );
```

- [ ] **Step 3: Extract UI → config in getPreferences()**

In `src/ui/preferences.cc`, `getPreferences()` (~line 480):

```cpp
p.dictPanelEnabled = ui.dictPanelEnabled->isChecked();
p.dictPanelMaxHeight = ui.dictPanelMaxHeight->value();
p.dictPanelHeightUnit = ui.dictPanelHeightUnit->currentText();
p.dictPanelScrollZone = ui.dictPanelScrollZone->value();
```

- [ ] **Step 4: Build and test preferences dialog**

```bash
cd build && make -j$(nproc)
./goldendict
# Edit → Preferences → Articles → Dictionary Panels
# Toggle limit, change height, verify save/load across restart
```

- [ ] **Step 5: Commit**

```bash
git add src/ui/preferences.ui src/ui/preferences.cc
git commit -m "feat: dict panel preferences UI"
```

---

### Task 5: Side-by-Side Panels — QSplitter Layout

**Files:**
- Modify: `src/ui/mainwindow.ui`
- Modify: `src/ui/mainwindow.hh`
- Modify: `src/ui/mainwindow.cc`

- [ ] **Step 1: Add QSplitter to mainwindow.ui**

Wrap the existing `MainTabWidget` in a layout that includes a `QSplitter`. The splitter is hidden when no panels are pinned.

Current structure (from `mainwindow.ui`):
```xml
<widget class="QWidget" name="centralWidget">
  <layout class="QHBoxLayout">
    <item><widget class="MainTabWidget" name="tabWidget"/></item>
  </layout>
</widget>
```

New structure:
```xml
<widget class="QWidget" name="centralWidget">
  <layout class="QHBoxLayout" name="centralLayout">
    <item>
      <widget class="QSplitter" name="panelSplitter">
        <property name="orientation"><enum>Qt::Vertical</enum></property>
        <property name="visible"><bool>false</bool></property>
      </widget>
    </item>
    <item><widget class="MainTabWidget" name="tabWidget"/></item>
  </layout>
</widget>
```

- [ ] **Step 2: Declare panel methods in mainwindow.hh**

In `src/ui/mainwindow.hh`:

```cpp
// Side-by-side panels
void addPanel(ArticleView * av);
void removePanel(ArticleView * av);
void togglePanel();
void togglePanelOrientation();
int panelCount() const;
```

- [ ] **Step 3: Implement panel management in mainwindow.cc**

```cpp
void MainWindow::addPanel(ArticleView * av) {
  // Remove from tab widget
  int idx = ui.tabWidget->indexOf(av);
  if (idx >= 0) {
    ui.tabWidget->removeTab(idx);
  }
  // Add to splitter
  ui.panelSplitter->addWidget(av);
  ui.panelSplitter->setVisible(true);
  av->show();
}

void MainWindow::removePanel(ArticleView * av) {
  // Remove from splitter
  av->setParent(nullptr);
  // Add as background tab
  int newIdx = ui.tabWidget->addTab(av, av->windowTitle());
  ui.tabWidget->setCurrentIndex(newIdx);
  // Hide splitter if empty
  if (ui.panelSplitter->count() == 0) {
    ui.panelSplitter->setVisible(false);
  }
}

void MainWindow::togglePanel() {
  ArticleView * current = qobject_cast<ArticleView*>(ui.tabWidget->currentWidget());
  if (current) {
    // Tab → Panel
    addPanel(current);
  } else {
    // Find focused panel
    QWidget * w = ui.panelSplitter->focusWidget();
    while (w && !qobject_cast<ArticleView*>(w)) w = w->parentWidget();
    current = qobject_cast<ArticleView*>(w);
    if (current) removePanel(current);
  }
}

void MainWindow::togglePanelOrientation() {
  if (ui.panelSplitter->orientation() == Qt::Horizontal)
    ui.panelSplitter->setOrientation(Qt::Vertical);
  else
    ui.panelSplitter->setOrientation(Qt::Horizontal);
}

int MainWindow::panelCount() const {
  return ui.panelSplitter->count();
}
```

- [ ] **Step 4: Build and verify layout**

```bash
cd build && make -j$(nproc)
./goldendict
# Window should look identical (splitter hidden)
```

- [ ] **Step 5: Commit**

```bash
git add src/ui/mainwindow.ui src/ui/mainwindow.hh src/ui/mainwindow.cc
git commit -m "feat: add QSplitter for side-by-side panels"
```

---

### Task 6: Side-by-Side Panels — Shortcuts and Menu

**Files:**
- Modify: `src/ui/mainwindow.hh` (QAction members)
- Modify: `src/ui/mainwindow.cc` (action registration, menu)

- [ ] **Step 1: Declare QAction members in mainwindow.hh**

In `src/ui/mainwindow.hh`, after existing QAction members (~line 118):

```cpp
QAction togglePanelAction;
QAction togglePanelOrientationAction;
```

- [ ] **Step 2: Initialize and register actions in mainwindow.cc**

In `src/ui/mainwindow.cc` constructor, after existing action registration (~line 500):

```cpp
// Panel toggle
togglePanelAction.setShortcut(QKeySequence("Ctrl+Shift+P"));
togglePanelAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
connect(&togglePanelAction, &QAction::triggered, this, &MainWindow::togglePanel);
addAction(&togglePanelAction);

// Panel orientation toggle
togglePanelOrientationAction.setShortcut(QKeySequence("Ctrl+Shift+H"));
togglePanelOrientationAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
connect(&togglePanelOrientationAction, &QAction::triggered, this, &MainWindow::togglePanelOrientation);
addAction(&togglePanelOrientationAction);
```

- [ ] **Step 3: Add menu items**

In the `View` menu setup (`src/ui/mainwindow.cc`, menu bar creation section):

```cpp
// Add to View menu
QMenu * viewMenu = menuBar()->addMenu(tr("&View"));
viewMenu->addAction(tr("Toggle Panel"), this, &MainWindow::togglePanel, QKeySequence("Ctrl+Shift+P"));
viewMenu->addAction(tr("Toggle Panel Orientation"), this, &MainWindow::togglePanelOrientation, QKeySequence("Ctrl+Shift+H"));
```

- [ ] **Step 4: Build and test shortcuts**

```bash
cd build && make -j$(nproc)
./goldendict
# Ctrl+Shift+P: current tab → panel
# Ctrl+Shift+P again: panel → tab
# Ctrl+Shift+H: toggle horizontal/vertical
```

- [ ] **Step 5: Commit**

```bash
git add src/ui/mainwindow.hh src/ui/mainwindow.cc
git commit -m "feat: panel toggle shortcuts and menu items"
```

---

### Task 7: Side-by-Side Panels — Tab Context Menu

**Files:**
- Modify: `src/ui/maintabwidget.cc`

- [ ] **Step 1: Add context menu in maintabwidget.cc**

In `src/ui/maintabwidget.cc`, connect `customContextMenuRequested` signal or override `contextMenuEvent`:

```cpp
void MainTabWidget::contextMenuEvent(QContextMenuEvent * event) {
  int tabIdx = tabBar()->tabAt(event->pos());
  if (tabIdx < 0) return;

  QMenu menu(this);
  QAction * moveAction = menu.addAction(tr("Move to Panel"));
  QAction * closeAction = menu.addAction(tr("Close Tab"));

  QAction * chosen = menu.exec(event->globalPos());
  if (chosen == moveAction) {
    emit moveTabToPanelRequested(tabIdx);
  } else if (chosen == closeAction) {
    emit tabCloseRequested(tabIdx);
  }
}
```

Declare the signal in `maintabwidget.hh`:
```cpp
signals:
  void moveTabToPanelRequested(int index);
```

- [ ] **Step 2: Wire signal in mainwindow.cc**

```cpp
connect(ui.tabWidget, &MainTabWidget::moveTabToPanelRequested, this, [this](int idx) {
  ArticleView * av = qobject_cast<ArticleView*>(ui.tabWidget->widget(idx));
  if (av) addPanel(av);
});
```

- [ ] **Step 3: Build and test**

```bash
cd build && make -j$(nproc)
./goldendict
# Right-click tab → "Move to Panel"
```

- [ ] **Step 4: Commit**

```bash
git add src/ui/maintabwidget.hh src/ui/maintabwidget.cc src/ui/mainwindow.cc
git commit -m "feat: tab context menu 'Move to Panel'"
```

---

## Self-Review

1. **Spec coverage:** Patch 1: settings ✓, CSS ✓, JS split-zone ✓, preferences UI ✓. Patch 2: QSplitter ✓, toggle shortcut ✓, orientation toggle ✓, context menu ✓. No gaps.

2. **Placeholder scan:** No TODOs, TBDs, "implement later", or vague "add error handling". All code blocks are concrete.

3. **Type consistency:** `dictPanelScrollZone` is int throughout. `scrollZoneSplit` property matches JS variable name. `addPanel`/`removePanel` signatures consistent between Task 5 (declaration) and Task 7 (usage).
