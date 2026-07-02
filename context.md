# GoldenDict-ng Keyboard Shortcuts / Hotkeys Architecture

## 1. Shortcut Categories

GoldenDict-ng has three distinct shortcut systems:

| Category | Scope | Mechanism |
|---|---|---|
| **Widget shortcuts** | Within widget focus | `QAction::setShortcut()` + `addAction()` |
| **Global actions** | Across all main window widgets + dialogs | `MainWindow::addGlobalAction()` |
| **OS-level global hotkeys** | System-wide, even when GD is backgrounded | `HotkeyWrapper::setGlobalKey()` (X11/Win/macOS) |

---

## 2. Widget Shortcuts (Simple Pattern)

### Pattern

```cpp
// 1. Declare QAction member (in .hh)
QAction myAction;

// 2. In constructor (in .cc)
myAction.setShortcut(QKeySequence("Ctrl+F"));
myAction.setShortcutContext(Qt::WidgetWithChildrenShortcut); // optional
addAction(&myAction);                                         // register on `this` widget
connect(&myAction, &QAction::triggered, this, &MyClass::myHandler);
```

### Concrete Example: Ctrl+F in ScanPopup — Full Trace

**Definition → Registration → Handler**

**Step 1 – Declare member:**
`src/ui/scanpopup.hh:135`:
```cpp
QAction openSearchAction;
```

**Step 2 – Initialize in constructor initializer list:**
`src/ui/scanpopup.cc:43`:
```cpp
openSearchAction( this ),
```

**Step 3 – Register shortcut + connect:**
`src/ui/scanpopup.cc:148-151`:
```cpp
openSearchAction.setShortcut( QKeySequence( "Ctrl+F" ) );
openSearchAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
addAction( &openSearchAction );
connect( &openSearchAction, &QAction::triggered, definition, &ArticleView::openSearch );
```

**Step 4 – Handler:**
`src/ui/articleview.cc:2023`:
```cpp
void ArticleView::openSearch()
{
  if ( !isVisible() ) return;
  if ( ftsSearchPanel->isVisible() ) closeSearch();
  if ( !searchPanel->isVisible() ) {
    searchPanel->show();
    searchPanel->lineEdit->setText( getTitle() );
  }
  searchPanel->lineEdit->setFocus();
  searchPanel->lineEdit->selectAll();
  // ...
}
```

### Concrete Example: Ctrl+T (New Tab) — Full Trace

**Declare:**
`src/ui/mainwindow.hh:118`:
```cpp
QAction escAction, focusTranslateLineAction, addTabAction, closeCurrentTabAction, ...
```

**Initialize:**
`src/ui/mainwindow.cc:161`:
```cpp
focusTranslateLineAction( this ),
```

**Register:**
`src/ui/mainwindow.cc:490-500`:
```cpp
addTabAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
addTabAction.setShortcut( QKeySequence( "Ctrl+T" ) );
connect( &addTabAction, &QAction::triggered, this, &MainWindow::addNewTab );
addAction( &addTabAction );
```

Note: `addTabAction` uses plain `addAction(&addTabAction)` (registered only on the main window widget), NOT `addGlobalAction()`. This is because tab operations are only meaningful in the main window context.

---

## 3. Global Actions (Works in All Widgets + Dialogs)

### `MainWindow::addGlobalAction()` — `src/ui/mainwindow.cc:1304-1316`

```cpp
void MainWindow::addGlobalAction( QAction * action, const std::function< void() > & slotFunc )
{
  action->setShortcutContext( Qt::WidgetWithChildrenShortcut );
  connect( action, &QAction::triggered, this, slotFunc );
  // Register on every major widget/dock so it fires regardless of focus
  ui.centralWidget->addAction( action );
  ui.dictsPane->addAction( action );
  ui.searchPaneWidget->addAction( action );
  ui.favoritesPane->addAction( action );
  ui.historyPane->addAction( action );
  groupList->addAction( action );
  translateBox->addAction( action );
}
```

Also `addGlobalActionsToDialog()` (`src/ui/mainwindow.cc:1318-1325`) adds global actions to dialogs (headwords dialog, FTS dialog).

### Example: Ctrl+D (Focus Headwords) — Global Action

`src/ui/mainwindow.cc:471-474`:
```cpp
addGlobalAction( &focusHeadwordsDlgAction, [ this ]() {
  focusHeadwordsDialog();
} );
focusHeadwordsDlgAction.setShortcut( QKeySequence( "Ctrl+D" ) );
```

### Example: Ctrl+L / Alt+D (Focus Translate Line) — Multiple Shortcuts

`src/ui/mainwindow.cc:465-469`:
```cpp
addGlobalAction( &focusTranslateLineAction, [ this ]() {
  focusTranslateLine();
} );
focusTranslateLineAction.setShortcuts( QList< QKeySequence >()
    << QKeySequence( "Alt+D" ) << QKeySequence( "Ctrl+L" ) );
```

### Complete list of global actions in MainWindow:

| Shortcut | Variable | Handler | File:Line |
|---|---|---|---|
| `Esc` | `escAction` | `handleEsc()` | mainwindow.cc:460 |
| `Alt+D`, `Ctrl+L` | `focusTranslateLineAction` | `focusTranslateLine()` | mainwindow.cc:465 |
| `Ctrl+D` | `focusHeadwordsDlgAction` | `focusHeadwordsDialog()` | mainwindow.cc:471 |
| `Ctrl+N` | `focusArticleViewAction` | `focusArticleView()` | mainwindow.cc:476 |
| `Ctrl+Shift+F` | `ui.fullTextSearchAction` | `showFullTextSearchDialog()` | mainwindow.cc:481 |
| `Ctrl+Shift+S` | `stopAudioAction` | `stopAudio()` | mainwindow.cc:485 |

---

## 4. Shortcut Storage (Config)

### Group Shortcuts (Ctrl+1-9 to switch groups)

**Config struct:** `src/config.hh:118`
```cpp
struct Group {
  unsigned id;
  QKeySequence shortcut;  // <-- stored per group
  // ...
};
```

**Save:** `src/config.cc:1214-1219` — serialized as XML attribute on `<group>` element:
```cpp
if ( !data.shortcut.isEmpty() ) {
  QDomAttr shortcut = dd.createAttribute( "shortcut" );
  shortcut.setValue( data.shortcut.toString() );
  group.setAttributeNode( shortcut );
}
```

**Load:** `src/config.cc:347-349`:
```cpp
if ( !grp.attribute( "shortcut" ).isEmpty() )
  g.shortcut = QKeySequence::fromString( grp.attribute( "shortcut" ) );
```

**Setting UI:** `src/ui/edit_group_tab.ui:84` — uses `QKeySequenceEdit` widget named `shortcut`.

**Runtime registration:** `src/ui/groupcombobox.cc:59-65`:
```cpp
if ( !groups[ x ].shortcut.isEmpty() ) {
  int id = grabShortcut( groups[ x ].shortcut );
  setShortcutEnabled( id );
  shortcuts.insert( id, x );
}
```
Handled via `GroupComboBox::event()` intercepting `QEvent::Shortcut` (`groupcombobox.cc:100-106`).

### Global Hotkey Preferences

**Config struct:** `src/config.hh:301-302`
```cpp
bool enableMainWindowHotkey;
bool enableClipboardHotkey;
QKeySequence mainWindowHotkey;   // default: "Ctrl+F11, Ctrl+F11"
QKeySequence clipboardHotkey;    // default: "Ctrl+C, Ctrl+C"
```

**Defaults:** `src/config.cc:121-122`

**Save:** `src/config.cc:1788-1797` — XML elements `<mainWindowHotkey>` and `<clipboardHotkey>`.

**Load:** `src/config.cc:850-857` — `QKeySequence::fromString()` from XML text.

**Note:** Per-widget shortcuts (Ctrl+F, Ctrl+T, etc.) are **not** user-configurable. They are hardcoded. Only group shortcuts and the two global OS hotkeys are stored in preferences.

---

## 5. OS-Level Global Hotkeys (System-Wide)

### HotkeyWrapper Architecture

**Header:** `src/hotkey/hotkeywrapper.hh`

**Platform implementations:**
- `src/hotkey/x11hotkeywrapper.cc` — X11 (XRecord extension)
- `src/hotkey/winhotkeywrapper.cc` — Windows (`RegisterHotKey` + native event filter)
- `src/hotkey/machotkeywrapper.mm` — macOS (Carbon `RegisterEventHotKey`)
- `src/hotkey/waylandhotkeywrapper.cc` — Wayland (stub, returns false)

**Key API:**
```cpp
class HotkeyWrapper : public QThread {
  bool setGlobalKey( const QKeySequence &, int handle );
  // handle 0 = show main window
  // handle 1 = translate clipboard
signals:
  void hotkeyActivated( int handle );
};
```

**Two-key sequences** (e.g. `"Ctrl+F11, Ctrl+F11"`) are supported. The wrapper detects the first key combo, then waits 500ms for the second combo (`HotkeyWrapper::waitKey2()`).

### Registration in MainWindow

**Function:** `MainWindow::installHotKeys()` — `src/ui/mainwindow.cc:3234-3266`

```cpp
void MainWindow::installHotKeys()
{
  hotkeyWrapper.reset(); // Remove old
  if ( cfg.preferences.enableMainWindowHotkey || cfg.preferences.enableClipboardHotkey ) {
    hotkeyWrapper = std::make_shared< HotkeyWrapper >( this );
    if ( cfg.preferences.enableMainWindowHotkey )
      hotkeyWrapper->setGlobalKey( cfg.preferences.mainWindowHotkey, 0 );
    if ( cfg.preferences.enableClipboardHotkey )
      hotkeyWrapper->setGlobalKey( cfg.preferences.clipboardHotkey, 1 );
    connect( hotkeyWrapper.get(), &HotkeyWrapper::hotkeyActivated,
             this, &MainWindow::hotKeyActivated, Qt::AutoConnection );
  }
}
```

### Handler: `MainWindow::hotKeyActivated()` — `src/ui/mainwindow.cc:3273-3290`

```cpp
void MainWindow::hotKeyActivated( int hk )
{
  if ( !hk ) {
    toggleMainWindow( false );   // handle 0 → show/hide main window
  } else {
    ensureScanPopup();
    // handle 1 → translate clipboard word
    // ... triggers clipboard scan
  }
}
```

### Preferences UI

Found in preferences dialog — `src/ui/preferences.cc` (hotkey section around line 223, bound by current global hotkey implementations).

---

## 6. How to Add a New Shortcut

### Option A: Widget-level shortcut (simplest)

```cpp
// In your widget's .hh:
QAction myNewAction;

// In your widget's .cc constructor:
myNewAction.setShortcut(QKeySequence("Ctrl+Shift+P"));
myNewAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
addAction(&myNewAction);
connect(&myNewAction, &QAction::triggered, this, &MyClass::myHandler);
```

### Option B: Global action (main window wide, also in dialogs)

```cpp
// 1. Add QAction member to src/ui/mainwindow.hh (around line 118):
QAction myNewAction;

// 2. Initialize in constructor initializer list (src/ui/mainwindow.cc ~line 161):
myNewAction(this),

// 3. Register in constructor body (after other addGlobalAction calls):
addGlobalAction(&myNewAction, [this]() { myNewHandler(); });
myNewAction.setShortcut(QKeySequence("Ctrl+Shift+P"));

// 4. Optionally add to dialogs:
// In addGlobalActionsToDialog() (mainwindow.cc:1318):
dialog->addAction(&myNewAction);

// 5. Implement handler method in MainWindow
```

### Option C: OS-level global hotkey (system-wide)

```cpp
// 1. Add fields to Config::Preferences (src/config.hh ~line 301):
bool enableMyHotkey;
QKeySequence myHotkey;

// 2. Set default in Preferences constructor (src/config.cc ~line 122):
myHotkey(QKeySequence("Ctrl+Shift+P")),

// 3. Add save/load in config.cc:
//   Load: around line 857
//   Save: around line 1797

// 4. Register in MainWindow::installHotKeys() (mainwindow.cc:3242):
if (cfg.preferences.enableMyHotkey)
  hotkeyWrapper->setGlobalKey(cfg.preferences.myHotkey, 2); // unique handle

// 5. Handle in MainWindow::hotKeyActivated() (mainwindow.cc:3273):
if (hk == 2) { myHandler(); return; }

// 6. Add UI controls in preferences dialog (src/ui/preferences.cc)
```

---

## 7. Key Files Summary

| File | Role |
|---|---|
| `src/ui/mainwindow.hh` | QAction member declarations (line 118) |
| `src/ui/mainwindow.cc` | Registration in constructor (~line 460-520), `addGlobalAction()` (line 1304), `installHotKeys()` (line 3234), `hotKeyActivated()` (line 3273) |
| `src/ui/mainwindow.ui` | UI-defined actions (shortcuts in XML) |
| `src/config.hh` | `Group::shortcut`, `HotKey` struct, global hotkey prefs (line 301) |
| `src/config.cc` | Save/load hotkey prefs (lines 850, 1788) |
| `src/hotkey/hotkeywrapper.hh` | `HotkeyWrapper` class, `HotkeyStruct`, `setGlobalKey()` API |
| `src/hotkey/x11hotkeywrapper.cc` | X11 implementation (XRecord) |
| `src/hotkey/winhotkeywrapper.cc` | Windows implementation |
| `src/hotkey/machotkeywrapper.mm` | macOS implementation |
| `src/ui/scanpopup.cc` | ScanPopup shortcuts (Ctrl+F, Esc, Alt+D, etc.) |
| `src/ui/groupcombobox.cc` | Group shortcut grab/release/event handling |
| `src/ui/edit_group_tab.ui` | `QKeySequenceEdit` for group shortcut editing |
| `src/ui/scanpopup_toolbar.ui` | Toolbar action shortcuts in XML |

---

## 8. Shortcut Context

`Qt::WidgetWithChildrenShortcut` is the standard context — shortcut fires when the widget or any child has focus. This prevents shortcuts from conflicting across dialogs.
