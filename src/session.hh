#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QString>
#include <QVector>

/// Data model for session persistence. Fork feature.
struct SessionData
{
  struct TabInfo
  {
    QString            word;
    unsigned           group = 0;
    bool               alwaysQuery = false;
    QSet< QString >    collapsedDicts;
  };

  struct PanelInfo
  {
    QVector< TabInfo > tabs;
    int                activeTab = 0;
  };

  QVector< PanelInfo > panels;
  int                  activePanel       = 0;
  Qt::Orientation      orientation       = Qt::Horizontal;
  QString              searchBarText;
};

/// JSON read/write for session state. Fork feature.
class Session
{
public:
  /// Returns true if the data was written successfully.
  static bool save( const SessionData & data, const QString & filePath );
  /// Returns a default-constructed SessionData (isEmpty() == true) on failure.
  static SessionData load( const QString & filePath );
};
