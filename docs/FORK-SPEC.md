# GoldenDict-ng wy fork — Feature & Bug-avoidance Spec

Purpose: a survival guide for re-implementing fork features after a rebase onto
`upstream/staged`. It lists what the fork adds and the functional traps to not
reintroduce. Packaging/CI notes are intentionally brief.

Upstream: `xiaoyifang/goldendict-ng`, branch `staged`. Fork branch: `wy-dev`.
Sync: selective cherry-pick; rebase/replay when possible, rewrite otherwise;
merges only sparingly.

## Fork features added

- **Side-by-side Panels** (`Panel`): multiple view containers, each holding one
  or more `Tab`s. The window always has >=1 Panel.
- **Tab / Group model**: a Tab belongs to exactly one `Group`, which decides
  which dictionaries answer its queries. Group is a property of the Tab,
  independent of the Panel.
- **Always Query**: a Tab flagged to receive every query (from any source, not
  only the search bar) in addition to the focused Tab, answering in its own
  Group, with color markers.
- **Website tabs**: a Tab that displays a website dictionary, opened
  automatically from a website-dictionary result in its Linked Tab, inheriting
  the Linked Tab's Group. Never an Always-Query target.
- **Muted dictionaries**: per-Group suppression; a muted dict produces no
  results and never auto-opens a Website tab.
- **Session persistence**: open Tabs, Panels, Always-Query flags, and collapsed
  dictionaries restored on launch.
- **Panel focus navigation**: `Ctrl+Alt+Left/Right` moves focus between panels.
- **Cross-panel navigation**: `jumpToDictionary` and `Ctrl+/-` zoom target the
  focused view's panel and all panels respectively.
- **found-in-dictionaries side panel**: synced to focus, with website fallback.
- **Persistent website data**: custom `QWebEngineProfile` keeps cookies,
  localStorage, IndexedDB across restarts.
- **Reverse scroll-zone toggle**: swap peripheral/middle scroll behavior.
- **Release pipeline**: `wy-release.yml`, Arch `PKGBUILD`, version `26.7.0`.

## Bugs to avoid (functional traps)

1. Website tab must inherit its Linked Tab's Group at open time via `groupId`
   threaded through `websiteDictionarySignal` -> `openWebsiteInNewTab`, with
   `GlobalBroadcaster::websiteRequestGroup` set around `getArticle`.
2. **Website tab Group must refresh on every query, not freeze at creation.**
   A Website tab is matched by `dictId`, so once opened it is *reused* by any
   later query hitting that dictionary. If `setCurrentGroupId( groupId )` runs
   only inside the `view == nullptr` (creation) branch, the tab stays on the
   originating tab's Group forever. Symptom: mute the website dict in Tab A,
   enable it in Tab B, query in B -> the Website tab still reports Group A, so
   its muted/enabled state and subsequent lookups are wrong. Fix: call
   `setCurrentGroupId( groupId )` on both creation and reuse, in both
   `MainWindow::openWebsiteInNewTab` and `ScanPopup::openWebsiteInNewTab`.
3. Session restore must reset `contentLoaded` + tab title on window reuse,
   else restored tabs render blank/duplicate.
4. Focus change must refresh the found-in-dictionaries list and Always-Query
   color markers.
5. `jumpToDictionary` must target the focused view's panel, not only main.
6. Refresh `lastFocusedArticleView` on new-tab creation before using it.
7. Forward history/navigation (`showTranslationFor`) to Always-Query tabs.
8. Route `Ctrl+/-` zoom through MainWindow to all panels.
 9. Use a custom `QWebEngineProfile` so website cookies/storage persist.
 10. **Split-scroll zones: the percent pref is not optional and the binding must
      wait for the web channel.** `scrollZonePercent` (formerly
      `dictPanelScrollZone`; percent width of the middle band) was only ever
      read/written to config and the prefs UI — the JS in `gd-builtin.js`
      hardcoded the middle band to 0.25..0.75, so the pref was inert ("always
      50%"). The reverse toggle also failed because the JS read
      `articleview.reverseScrollZone` at script-load time, before the web channel
      object existed, so the connect was skipped and `reversed` stayed false.
      Fix: add a `scrollZonePercent` Q_PROPERTY on `ArticleViewAgent` (mirror
      `reverseScrollZone`), set it from `cfg.preferences.scrollZonePercent`
     in `createArticleView` and in the `editPreferences` per-view loop, and in
     JS derive the middle band from the percent and `setTimeout`-retry
     `bindArticleView` until `articleview` is defined.
 11. **Always Query must receive context-menu "Look up [selection]".** The
      article context-menu lookup (`lookupSelection` / `lookupSelectionGr` in
      `ArticleView::contextMenuRequested`) called `showDefinition()` directly and
      never emitted `wordLookedUp`, so `MainWindow::forwardToAlwaysQueryTabs`
      (wired to `ArticleView::wordLookedUp`) never fired and Always-Query tabs
      stayed silent. Fix: emit `wordLookedUp( this, word, group, QString() )`
      after the `showDefinition()` call in those two branches, mirroring the
      `gdlookup`/`bword` link-click path.
 12. **Never put `overflow`/`max-height` on `.gdarticlebody` unconditionally.**
      The "limit dictionary article height" feature (now `entryHeightLimit`,
      formerly `dictPanelEnabled`) added `overflow-y:auto` +
      `max-height:var(--gd-entry-height)` to `.gdarticlebody` (each article
      `<section>`). Because it formerly defaulted to `true`, every article
      became a scroll container — and Qt WebEngine then renders mdict (and other
      dictionaries that embed their own `<style>`) with the wrong font-family,
      ignoring the user's `article-style.css`. The CSS cascade is unaffected
      (the rule sets no `font-family`); it is a scroll-container rendering quirk
      specific to Qt WebEngine. Fix: gate `overflow`+`max-height` behind
      `cfg.entryHeightLimit` in `ArticleMaker::makeHtmlHeader`, keep
      `.gdarticlebody` as `clear:both` in the default stylesheet, and default
      `entryHeightLimit` to `false` so fresh installs do not break mdict.
      Caveat: with the feature still enabled, the per-article scroll container
      remains, so the
      height limit must instead be applied to the panel *container* (not
      `.gdarticlebody`) to fully fix mdict while keeping the feature.
 13. **Spurious scrollbar on entries that fit the height limit.** Four
      independent causes, each addressed:
      (a) Trailing bottom margin of the last content block is trapped inside
      the scroll container (a BFC) and inflates `scrollHeight` past `max-height`.
      Fix: `.gdarticlebody :last-child{margin-bottom:0 !important}` (at every
      nesting depth).
      (b) Trailing bottom padding — same BFC-trapping mechanism as margin.
      Fix: zeroed alongside margin on `:last-child`.
      (c) A trailing empty sibling element (e.g. `<br>`, clear `<div>`) blocks
      the real last block from being `:last-child`, so its margin/padding escape
      zeroing. Fix: `br:last-child,div:last-child:empty{display:none !important}`.
      (d) Chromium compositor ghost-scroll: even with `scrollHeight==clientHeight`,
      `overflow-y:auto` can allocate a tiny scroll extent for its internal scroll
      layer, producing a scrollbar that moves a few px. Fix: default to
      `overflow-y:hidden`; use JS (double `requestAnimationFrame` + `load` +
      `fonts.ready`) to switch to `overflow-y:auto` only when
      `scrollHeight > clientHeight` is confirmed after layout.
      Additionally, a 16px soft cap (`max-height:calc(var(--gd-entry-height) +
      16px)`) absorbs sub-pixel/metric overflow without shrinking the box.
      ANTI-PATTERN (do not use): a `::after{content:"";display:block;height:0;
      margin-top:-Npx}` "tolerance" hack. As the last child it pulls the
      container's own bottom edge up, shrinking EVERY entry by N px and creating
      scrollbars on entries that previously fit (observed on MW Unabridged).
      The `calc(... + 16px)` cap raise fixes it with no shrink.

