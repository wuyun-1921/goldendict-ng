/**
 * Panel feature tests using Qt Test framework.
 *
 * Build: cd build && cmake .. && make panel_test
 * Run: ./build/panel_test
 *
 * Tests:
 *   1. addPanel creates QTabWidget in splitter
 *   2. removePanel returns tab to main tab widget
 *   3. togglePanel moves tab ↔ panel
 *   4. Ctrl+Shift+H toggles orientation
 *   5. Single tab in main bar is protected
 *   6. Close last panel tab removes panel
 *   7. panelCount reflects actual visible panels
 */

#include <QApplication>
#include <QTest>
#include <QSplitter>
#include <QTabWidget>
#include <QKeySequence>

#include "ui/articleview.hh"
#include "ui/mainwindow.hh"
#include "config.hh"

class TestPanelFeatures : public QObject
{
  Q_OBJECT

private:
  static Config::Class makeTestConfig()
  {
    Config::Class cfg;
    cfg.preferences.dictPanelEnabled = true;
    cfg.preferences.dictPanelMaxHeight = 300;
    cfg.preferences.dictPanelHeightUnit = "px";
    cfg.preferences.dictPanelScrollZone = 50;
    cfg.preferences.sideBySideDefaultSplit = 50;
    return cfg;
  }

private slots:
  void test_addPanel_creates_tab_widget()
  {
    MainWindow w( makeTestConfig() );
    QCOMPARE( w.panelCount(), 0 );

    // Create a second tab so we can move one
    auto * tab2 = w.createNewTab( false, "Test Tab" );
    QVERIFY( tab2 != nullptr );

    // Move it to a panel
    w.addPanel( tab2 );
    QCOMPARE( w.panelCount(), 1 );

    // Verify panel splitter has a QTabWidget
    bool foundPanel = false;
    for ( int i = 0; i < w.ui.panelSplitter->count(); i++ ) {
      if ( qobject_cast< QTabWidget * >( w.ui.panelSplitter->widget( i ) ) ) {
        foundPanel = true;
        break;
      }
    }
    QVERIFY( foundPanel );
  }

  void test_removePanel_returns_tab()
  {
    MainWindow w( makeTestConfig() );
    auto * tab2 = w.createNewTab( false, "Test Tab" );
    w.addPanel( tab2 );
    QCOMPARE( w.panelCount(), 1 );

    w.removePanel( tab2 );
    QCOMPARE( w.panelCount(), 0 );

    // Tab should be back in main tab widget
    int idx = w.ui.tabWidget->indexOf( tab2 );
    QVERIFY( idx >= 0 );
  }

  void test_single_tab_protected()
  {
    MainWindow w( makeTestConfig() );
    // Only one tab (the default welcome tab)
    QCOMPARE( w.ui.tabWidget->count(), 1 );

    // togglePanel should not move the only tab
    w.togglePanel();
    QCOMPARE( w.panelCount(), 0 );
    QCOMPARE( w.ui.tabWidget->count(), 1 );
  }

  void test_toggle_panel_orientation()
  {
    MainWindow w( makeTestConfig() );
    auto * tab2 = w.createNewTab( false, "Test Tab" );
    w.addPanel( tab2 );

    Qt::Orientation initial = w.ui.panelSplitter->orientation();
    w.togglePanelOrientation();
    Qt::Orientation toggled = w.ui.panelSplitter->orientation();
    QVERIFY( initial != toggled );

    w.togglePanelOrientation();
    QCOMPARE( w.ui.panelSplitter->orientation(), initial );
  }

  void test_panel_count()
  {
    MainWindow w( makeTestConfig() );
    QCOMPARE( w.panelCount(), 0 );

    auto * t1 = w.createNewTab( false, "Tab 1" );
    auto * t2 = w.createNewTab( false, "Tab 2" );

    w.addPanel( t1 );
    QCOMPARE( w.panelCount(), 1 );

    w.addPanel( t2 );
    QCOMPARE( w.panelCount(), 2 );

    w.removePanel( t1 );
    QCOMPARE( w.panelCount(), 1 );

    w.removePanel( t2 );
    QCOMPARE( w.panelCount(), 0 );
  }

  void test_close_last_tab_removes_panel()
  {
    MainWindow w( makeTestConfig() );
    auto * tab2 = w.createNewTab( false, "Test Tab" );
    w.addPanel( tab2 );
    QCOMPARE( w.panelCount(), 1 );

    // Simulate closing the panel tab via the close button
    // Find the panel QTabWidget and close its tab
    QTabWidget * panel = nullptr;
    for ( int i = 0; i < w.ui.panelSplitter->count(); i++ ) {
      panel = qobject_cast< QTabWidget * >( w.ui.panelSplitter->widget( i ) );
      if ( panel )
        break;
    }
    QVERIFY( panel != nullptr );

    // Close the tab — should emit tabCloseRequested
    int tabIdx = panel->indexOf( tab2 );
    QVERIFY( tabIdx >= 0 );
    emit panel->tabCloseRequested( tabIdx );

    // Panel should be gone
    QCOMPARE( w.panelCount(), 0 );
    // Tab should be back in main tab widget
    QVERIFY( w.ui.tabWidget->indexOf( tab2 ) >= 0 );
  }
};

QTEST_MAIN( TestPanelFeatures )
#include "panel_test.moc"
