#include "session.hh"
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

bool Session::save( const SessionData & data, const QString & filePath )
{
  QJsonArray panelsJson;
  for ( const auto & panel : data.panels ) {
    QJsonArray tabsJson;
    for ( const auto & tab : panel.tabs ) {
      QJsonObject tabJson;
      tabJson[ "word" ]        = tab.word;
      tabJson[ "group" ]       = (int)tab.group;
      tabJson[ "alwaysQuery" ] = tab.alwaysQuery;
      if ( !tab.collapsedDicts.isEmpty() ) {
        QStringList ids( tab.collapsedDicts.begin(), tab.collapsedDicts.end() );
        tabJson[ "collapsed" ] = ids.join( ',' );
      }
      tabsJson.append( tabJson );
    }
    if ( tabsJson.isEmpty() )
      continue;
    QJsonObject panelJson;
    panelJson[ "tabs" ]      = tabsJson;
    panelJson[ "activeTab" ] = panel.activeTab;
    panelsJson.append( panelJson );
  }

  QJsonObject root;
  root[ "panels" ]        = panelsJson;
  root[ "activePanel" ]   = data.activePanel;
  root[ "orientation" ]   = ( data.orientation == Qt::Horizontal ) ? QStringLiteral( "Horizontal" ) :
                                                                      QStringLiteral( "Vertical" );
  root[ "searchBarText" ] = data.searchBarText;

  QJsonDocument doc( root );
  QFile file( filePath );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    return false;
  file.write( doc.toJson( QJsonDocument::Compact ) );
  file.close();
  if ( file.error() != QFile::NoError ) {
    qDebug() << "saveSession: write error for" << filePath << ":" << file.errorString();
    return false;
  }
  return true;
}

SessionData Session::load( const QString & filePath )
{
  SessionData data;

  QFile file( filePath );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    qDebug() << "loadSession: cannot open session file:" << filePath;
    return data;
  }

  QByteArray raw = file.readAll();
  file.close();
  if ( raw.isEmpty() ) {
    qDebug() << "loadSession: empty session file:" << filePath;
    return data;
  }

  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson( raw, &err );
  if ( err.error != QJsonParseError::NoError || !doc.isObject() ) {
    qDebug() << "loadSession: JSON parse error in" << filePath << ":" << err.errorString();
    return data;
  }

  QJsonObject root = doc.object();

  data.searchBarText = root[ "searchBarText" ].toString();

  QString orient = root[ "orientation" ].toString();
  data.orientation = ( orient == QStringLiteral( "Vertical" ) ) ? Qt::Vertical : Qt::Horizontal;

  data.activePanel = root[ "activePanel" ].toInt( 0 );

  QJsonArray panelsJson = root[ "panels" ].toArray();
  for ( int p = 0; p < panelsJson.size(); p++ ) {
    QJsonObject panelJson = panelsJson[ p ].toObject();
    QJsonArray tabsJson   = panelJson[ "tabs" ].toArray();
    if ( tabsJson.isEmpty() )
      continue;

    SessionData::PanelInfo panel;
    panel.activeTab = panelJson[ "activeTab" ].toInt( 0 );

    for ( int t = 0; t < tabsJson.size(); t++ ) {
      QJsonObject tabJson = tabsJson[ t ].toObject();
      SessionData::TabInfo tab;
      tab.word        = tabJson[ "word" ].toString();
      tab.group       = (unsigned)tabJson[ "group" ].toInt();
      tab.alwaysQuery = tabJson[ "alwaysQuery" ].toBool();
      if ( tabJson.contains( "collapsed" ) ) {
        for ( const auto & id : tabJson[ "collapsed" ].toString().split( ',' ) ) {
          if ( !id.isEmpty() )
            tab.collapsedDicts.insert( id );
        }
      }
      panel.tabs.append( tab );
    }
    data.panels.append( panel );
  }

  return data;
}
