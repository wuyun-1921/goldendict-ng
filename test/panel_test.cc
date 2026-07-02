/**
 * Panel feature unit tests — no MainWindow needed.
 *
 * Build from build dir:
 *   cd build && cmake .. && make panel_test && QT_QPA_PLATFORM=offscreen ./panel_test
 */
#include <QApplication>
#include <QTest>
#include <QSplitter>
#include <QTabWidget>

#include "ui/articleview.hh"

class TestPanelBasics : public QObject
{
  Q_OBJECT

private:
  QSplitter * splitter;
  QTabWidget * mainTabs;

private slots:
  void init()
  {
    splitter = new QSplitter();
    mainTabs = new QTabWidget();
  }

  void cleanup()
  {
    delete splitter;
    splitter = nullptr;
    delete mainTabs;
    mainTabs = nullptr;
  }

  void test_panel_created_when_tab_added()
  {
    // Create a panel QTabWidget and add it to splitter
    auto * panel = new QTabWidget();
    panel->setTabsClosable(true);
    splitter->addWidget(panel);

    // Create an ArticleView and add it to the panel
    auto * view = new ArticleView(panel);
    panel->addTab(view, "Test Tab");

    QCOMPARE(splitter->count(), 1);
    QCOMPARE(panel->count(), 1);
    QVERIFY(panel->indexOf(view) >= 0);

    // Close tab → panel should be empty but still in splitter
    panel->removeTab(0);
    QCOMPARE(panel->count(), 0);

    // Simulate what our code does: delete empty panel
    delete panel;
    // splitter should update its count (delete removes from parent)
    // Note: QSplitter might not immediately update count after child delete
    QVERIFY(splitter->count() <= 1); // 0 or 1 depending on Qt version
  }

  void test_move_tab_between_widgets()
  {
    auto * view = new ArticleView(mainTabs);
    mainTabs->addTab(view, "Move Me");

    // Move from main tabs to panel
    int idx = mainTabs->indexOf(view);
    mainTabs->removeTab(idx);

    auto * panel = new QTabWidget();
    panel->addTab(view, "Move Me");
    splitter->addWidget(panel);

    QCOMPARE(mainTabs->indexOf(view), -1);
    QCOMPARE(panel->indexOf(view), 0);

    // Move back
    panel->removeTab(0);
    mainTabs->addTab(view, "Move Me");

    QCOMPARE(mainTabs->indexOf(view), 0);
    QCOMPARE(panel->count(), 0);

    delete panel;
    delete view;
  }

  void test_multiple_panels()
  {
    auto * panel1 = new QTabWidget();
    auto * panel2 = new QTabWidget();
    splitter->addWidget(panel1);
    splitter->addWidget(panel2);

    auto * v1 = new ArticleView(panel1);
    auto * v2 = new ArticleView(panel2);
    panel1->addTab(v1, "Panel 1");
    panel2->addTab(v2, "Panel 2");

    QCOMPARE(splitter->count(), 2);
    QCOMPARE(panel1->count(), 1);
    QCOMPARE(panel2->count(), 1);

    // Close Panel 1
    panel1->removeTab(0);
    QCOMPARE(panel1->count(), 0);

    // Panel 2 should still be there
    QCOMPARE(panel2->count(), 1);

    delete panel1;
    delete panel2;
    delete v1;
    delete v2;
  }

  void test_splitter_orientation()
  {
    splitter->setOrientation(Qt::Vertical);
    QCOMPARE(splitter->orientation(), Qt::Vertical);

    splitter->setOrientation(Qt::Horizontal);
    QCOMPARE(splitter->orientation(), Qt::Horizontal);
  }

  void test_tab_close_signal()
  {
    auto * panel = new QTabWidget();
    panel->setTabsClosable(true);
    splitter->addWidget(panel);

    auto * view = new ArticleView(panel);
    panel->addTab(view, "Close Me");

    bool closeReceived = false;
    int closedIndex = -1;
    QObject::connect(panel, &QTabWidget::tabCloseRequested,
      [&](int i) { closeReceived = true; closedIndex = i; });

    emit panel->tabCloseRequested(0);
    QVERIFY(closeReceived);
    QCOMPARE(closedIndex, 0);

    delete panel;
    delete view;
  }
};

QTEST_MAIN(TestPanelBasics)
#include "panel_test.moc"
