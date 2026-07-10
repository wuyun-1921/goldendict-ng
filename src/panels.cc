#include "panels.hh"
#include "ui/articleview.hh"
#include <QApplication>
#include <QStyle>
#include <QWidget>

Panels::Panels( QSplitter * splitter, QObject * parent )
  : QObject( parent )
  , m_splitter( splitter )
{
}

int Panels::count() const
{
  return m_splitter->count();
}

QTabWidget * Panels::activePanel() const
{
  ArticleView * fv = activeView();
  if ( !fv )
    return qobject_cast< QTabWidget * >( m_splitter->widget( 0 ) );
  for ( int i = 0; i < m_splitter->count(); i++ ) {
    auto * panel = qobject_cast< QTabWidget * >( m_splitter->widget( i ) );
    if ( panel && panel->indexOf( fv ) >= 0 )
      return panel;
  }
  return qobject_cast< QTabWidget * >( m_splitter->widget( 0 ) );
}

ArticleView * Panels::activeView() const
{
  QWidget * w = QApplication::focusWidget();
  while ( w ) {
    auto * av = qobject_cast< ArticleView * >( w );
    if ( av )
      return av;
    w = w->parentWidget();
  }
  auto * first = qobject_cast< QTabWidget * >( m_splitter->widget( 0 ) );
  if ( first )
    return qobject_cast< ArticleView * >( first->currentWidget() );
  return nullptr;
}

QVector< ArticleView * > Panels::allViews() const
{
  QVector< ArticleView * > result;
  for ( int i = 0; i < m_splitter->count(); i++ ) {
    auto * panel = qobject_cast< QTabWidget * >( m_splitter->widget( i ) );
    if ( !panel )
      continue;
    for ( int j = 0; j < panel->count(); j++ ) {
      auto * av = qobject_cast< ArticleView * >( panel->widget( j ) );
      if ( av )
        result.append( av );
    }
  }
  return result;
}

QVector< QTabWidget * > Panels::allPanels() const
{
  QVector< QTabWidget * > result;
  for ( int i = 0; i < m_splitter->count(); i++ ) {
    auto * panel = qobject_cast< QTabWidget * >( m_splitter->widget( i ) );
    if ( panel )
      result.append( panel );
  }
  return result;
}

void Panels::add( ArticleView * av, int targetPanelIdx )
{
  QString title = av->windowTitle();

  // Remove from current panel
  QTabWidget * currentPanel = nullptr;
  int currentPanelIdx       = -1;
  int prevCount             = m_splitter->count();
  for ( int i = 0; i < m_splitter->count(); i++ ) {
    auto * panel = qobject_cast< QTabWidget * >( m_splitter->widget( i ) );
    if ( !panel )
      continue;
    int idx = panel->indexOf( av );
    if ( idx >= 0 ) {
      if ( title.isEmpty() )
        title = panel->tabText( idx );
      panel->removeTab( idx );
      currentPanel    = panel;
      currentPanelIdx = i;
      break;
    }
  }
  if ( title.isEmpty() )
    title = tr( "(untitled)" );

  // Clean up empty side panel (not main)
  if ( currentPanel && currentPanel != m_splitter->widget( 0 ) && currentPanel->count() == 0 ) {
    delete currentPanel;
    prevCount--;
    if ( currentPanelIdx < targetPanelIdx )
      targetPanelIdx--;
  }

  // Target: main panel is index 0
  QTabWidget * target = nullptr;
  if ( targetPanelIdx >= m_splitter->count() ) {
    auto * panel = new QTabWidget( m_splitter );
    panel->setDocumentMode( true );
    m_splitter->addWidget( panel );
    target = panel;
  }
  else {
    target = qobject_cast< QTabWidget * >( m_splitter->widget( targetPanelIdx ) );
  }
  if ( !target )
    return;

  target->addTab( av, title );
  target->setCurrentWidget( av );
  av->setFocus();

  distributeSizes();

  if ( m_splitter->count() != prevCount )
    emit panelCountChanged( m_splitter->count() );
}

void Panels::remove( ArticleView * av )
{
  for ( int i = 0; i < m_splitter->count(); i++ ) {
    auto * panel = qobject_cast< QTabWidget * >( m_splitter->widget( i ) );
    if ( !panel )
      continue;
    int idx = panel->indexOf( av );
    if ( idx < 0 )
      continue;
    panel->removeTab( idx );
    if ( panel->count() == 0 && m_splitter->count() > 1 ) {
      panel->deleteLater();
      distributeSizes();
      emit panelCountChanged( m_splitter->count() );
    }
    return;
  }
}

void Panels::distributeSizes()
{
  int n = m_splitter->count();
  if ( n <= 1 )
    return;
  int total = m_splitter->orientation() == Qt::Horizontal ? m_splitter->width() : m_splitter->height();
  total = std::max( total - m_splitter->handleWidth() * ( n - 1 ), n * 100 );
  int each = total / n;
  QList< int > sizes;
  for ( int i = 0; i < n; i++ )
    sizes.append( each );
  m_splitter->setSizes( sizes );

  for ( int i = 0; i < n; i++ )
    m_splitter->setStretchFactor( i, 1 );

  auto centralWidget = qobject_cast< QWidget * >( m_splitter->parent() );
  if ( centralWidget )
    centralWidget->setMinimumWidth( n * 200 );
}

void Panels::focusAdjacent( int offset )
{
  ArticleView * curr = activeView();
  if ( !curr )
    return;

  int panelIdx = -1;
  for ( int i = 0; i < m_splitter->count(); i++ ) {
    auto * panel = qobject_cast< QTabWidget * >( m_splitter->widget( i ) );
    if ( panel && panel->indexOf( curr ) >= 0 ) {
      panelIdx = i;
      break;
    }
  }
  if ( panelIdx < 0 )
    return;

  int targetIdx = panelIdx + offset;
  if ( targetIdx < 0 )
    targetIdx = m_splitter->count() - 1;
  else if ( targetIdx >= m_splitter->count() )
    targetIdx = 0;

  auto * targetPanel = qobject_cast< QTabWidget * >( m_splitter->widget( targetIdx ) );
  if ( targetPanel && targetPanel->count() > 0 ) {
    auto * av = qobject_cast< ArticleView * >( targetPanel->currentWidget() );
    if ( av )
      av->setFocus();
  }
}

void Panels::toggleOrientation()
{
  m_splitter->setOrientation( m_splitter->orientation() == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal );
  distributeSizes();
}
