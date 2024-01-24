#ifndef MIMEUTILS_H
#define MIMEUTILS_H

#include <QFileInfo>
#include <QVariant>
/**
 * @class MimeUtils
 * @brief Helps with mime type management
 * @author Michal Rost
 * @date 29.4.2013
 */
class MimeUtils : public QObject {
  Q_OBJECT
public:
  explicit MimeUtils(QObject* parent = Q_NULLPTR);
  virtual ~MimeUtils();
  bool load(const QString &fileName, const QString &group = "");
  QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
  void getProperties(const QString &fileName = "", const QString &group = "");
  QString getMimeType(const QString &path);
  void getDesktopFile(const QString &fileName);
  QStringList applicationLocations(QString appPath);
  QString findApplication(QString appPath, QString desktopFile);
  void checkAndKillRunningApp(QString &appName);
  void openInApp(QString exe, const QFileInfo &file, QString termCmd = QString());
  void openInApp(const QFileInfo &file, QString termCmd = QString());
  void openFiles(const QStringList &files);
private:
  bool defaultsChanged;
  QString defaultsFileName;
  QString fileName;
  QString name;
  QString genericName;
  QString exec;
  QString icon;
  QString type;
  bool no_display;
  bool terminal;
  QStringList categories;
  QStringList mimeType;
  QMap<QString, QVariant> data;
};

#endif // MIMEUTILS_H
