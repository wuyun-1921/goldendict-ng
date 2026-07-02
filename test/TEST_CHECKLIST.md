## Panel Feature Test Checklist

Run after building: `/home/t/repos/goldendict-ng/build/goldendict`

### CDP Web Tests (automated)
```bash
QTWEBENGINE_REMOTE_DEBUGGING=9222 ./build/goldendict &
sleep 3
node test/panel-cdp-test.js
```

### Manual Qt Panel Tests

| # | Test | Steps | Expected |
|---|------|-------|----------|
| 1 | Open panel | Open 2+ tabs, focus a tab, `Ctrl+Shift+P` | Tab moves to panel on the right, main tab widget shrinks |
| 2 | Close panel | Focus panel, `Ctrl+Shift+Q` | Tab returns to main tab bar, panel disappears |
| 3 | Single tab guard | Only 1 tab, `Ctrl+Shift+P` | Nothing happens |
| 4 | Orientation toggle | Panel open, `Ctrl+Shift+H` | Panels switch between side-by-side and stacked |
| 5 | Tab title preserved | Move tab to panel | Panel tab shows same title as original tab |
| 6 | Tab title back | Move panel tab back | Main tab bar shows correct title |
| 7 | Multiple panels | Move 2+ tabs to panels | Each panel gets own QTabWidget, equal share of splitter |
| 8 | Scroll zones | Middle 50% of article body, scroll | Article scrolls internally |
| 9 | Scroll zones edge | Left or right 25% of article body, scroll | Page scrolls, article stays still |
| 10 | Height limit | Dict panel height limit ON in preferences | Article bodies capped at configured height with scroll |
