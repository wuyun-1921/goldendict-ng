# goldendict-ng (wy fork) Domain Model

Glossary of terms specific to the wuyun-1921 fork of GoldenDict-ng. General
programming concepts are intentionally omitted.

## Language

**Panel**:
A side-by-side view container that holds one or more Tabs. The window always
contains at least one Panel; further Panels are added alongside it.
_Avoid_: split, pane, view

**Tab**:
A single article or website display surface. A Tab is always contained in
exactly one Panel.
_Avoid_: page, article, window

**Group**:
A named subset of dictionaries. Each Tab is associated with exactly one Group,
which decides which dictionaries answer that Tab's queries. Group is a property
of the Tab, independent of the Panel that contains it; Tabs within the same
Panel may therefore belong to different Groups.
_Avoid_: dictionary set, collection

**Always Query**:
A Tab flagged to receive every query, from any source (not only the search
bar), in addition to the focused Tab. It answers in its own Group.
_Avoid_: pin, sticky, auto-query

**Website tab**:
A Tab that displays a website dictionary. It opens automatically when that
website dictionary appears as a result in its linked Tab, but only while the
dictionary is unmuted in that Group. A Website tab is never an Always Query
target.
_Avoid_: web tab, browser tab

**Linked Tab**:
The Tab from which a Website tab was opened — the Tab whose article contained
the website dictionary result. A Website tab inherits the Group of its linked
Tab.
_Avoid_: source tab, parent tab

**Muted (dictionary)**:
A dictionary that is not queried — the query event is suppressed for it, so it
produces no results. Muting is per-Group, so every Tab of that Group shares the
same muted/unmuted state. A muted website dictionary is never queried and
therefore never auto-opens.
_Avoid_: disabled, hidden

**Session**:
The persisted multi-panel layout — open Tabs, Panels, Always Query flags, and
collapsed dictionaries — saved on exit and restored on launch.
_Avoid_: state, layout

## Workflow

**wy-dev**:
The fork's sole branch. All development happens here, and releases are cut
from it — either by manual dispatch from the Actions tab (no tag needed) or by
pushing a `v*` tag. Upstream merges are done manually and are not tracked to a
fixed branch.
_Avoid_: main, master, wy-main

**Release trigger**:
The assistant must never manually create or push a `v*` tag (or otherwise
trigger a release) on its own. Cutting a release is a human action — either the
user dispatches the `WY Release` workflow from the Actions tab, or the user
pushes the tag. The assistant may commit and push code to `wy-dev`, but it must
stop short of the release tag and ask the user to trigger the release.
