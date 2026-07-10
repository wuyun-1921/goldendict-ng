#pragma once
#include <QObject>
#include <QSplitter>
#include <QTabWidget>
#include <QVector>

class ArticleView;

/// Manages the multi-panel splitter layout. Fork feature.
/// Owns the QSplitter widget. Each panel is a QTabWidget inside it.
class Panels : public QObject
{
  Q_OBJECT

public:
  explicit Panels( QSplitter * splitter, QObject * parent = nullptr );

  QSplitter * widget() const { return m_splitter; }

  /// Panel count (0..N).
  int count() const;

  /// Return the panel containing focusView, or the first panel.
  QTabWidget * activePanel() const;

  /// Return the ArticleView for the focused tab, or nullptr.
  ArticleView * activeView() const;

  /// Return all ArticleViews across all panels.
  QVector< ArticleView * > allViews() const;

  /// Add av to the panel at targetPanelIdx. Creates a new panel if targetPanelIdx >= count().
  void add( ArticleView * av, int targetPanelIdx );

  /// Remove av from its panel. If the panel becomes empty, remove the panel.
  void remove( ArticleView * av );

  /// Distribute sizes evenly across panels.
  void distributeSizes();

  /// Move keyboard focus to the panel offset positions away.
  void focusAdjacent( int offset );

  /// Toggle Horizontal <-> Vertical.
  void toggleOrientation();

signals:
  /// Emitted when a panel is added or removed.
  void panelCountChanged( int count );

private:
  QSplitter * m_splitter;
};
