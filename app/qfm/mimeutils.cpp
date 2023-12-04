#include "mimeutils.h"
#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QDebug>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QMimeType>

#define MIME_APPS1 "/.local/share/applications/mimeapps.list"
#define MIME_APPS2 "/usr/share/applications/mimeapps.list"
/**
 * @brief Creates mime utils
 * @param parent
 */
MimeUtils::MimeUtils(QObject *parent) : QObject(parent) {
    QString path(QDir::homePath() + MIME_APPS1);
    QFileInfo fi(path);
    if(fi.exists()){
        defaultsFileName = path;
    } else {
        QFileInfo ff(MIME_APPS2);
        if(ff.exists())
            defaultsFileName = MIME_APPS2;
    }

    getProperties();
    load(defaultsFileName, "Default Applications");
    defaultsChanged = false;
}

MimeUtils::~MimeUtils() {
}
//---------------------------------------------------------------------------

/**
 * @brief Loads property file
 * @param fileName
 * @param group
 * @return true if load was successful
 */
bool MimeUtils::load(const QString &fileName, const QString &group) {

  // NOTE: This class is used for reading of property files instead of QSettings
  // class, which considers separator ';' as comment

  // Try open file
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }

  // Clear old data
  data.clear();

  // Indicator whether group was found or not, if name of group was not
  // specified, groupFound is always true
  bool groupFound = group.isEmpty();

  // Read propeties
  QTextStream in(&file);
  while (!in.atEnd()) {

    // Read new line
    QString line = in.readLine();

    // Skip empty line or line with invalid format
    if (line.trimmed().isEmpty()) {
      continue;
    }

    // Read group
    // NOTE: symbols '[' and ']' can be found not only in group names, but
    // only group can start with '['
    if (!group.isEmpty() && line.trimmed().startsWith("[")) {
      QString tmp = line.trimmed().replace("[", "").replace("]", "");
      groupFound = group.trimmed().compare(tmp) == 0;
    }

    // If we are in correct group and line contains assignment then read data
    if (groupFound && line.contains("=")) {
      QStringList tmp = line.split("=");
      data.insert(tmp.at(0), tmp.at(1));
    }
  }
  file.close();
  return true;
}

QVariant MimeUtils::value(const QString &key, const QVariant &defaultValue) {
  return data.value(key, defaultValue);
}

void MimeUtils::getProperties(const QString &fileName, const QString &group) {
  if (!fileName.isEmpty()) {
    load(fileName, group);
  }
}

/**
 * @brief Returns mime type of given file
 * @note This operation is slow, prevent its mass application
 * @param path path to file
 * @return mime type
 */
QString MimeUtils::getMimeType(const QString &path) {
    QMimeDatabase db;
    QMimeType type = db.mimeTypeForFile(path);
    //qDebug() << "mime type" << type.name() << path;
    return type.name();
}


void MimeUtils::getDesktopFile(const QString &fileName) {

  // Store file name
  this->fileName = fileName;

  // File validity
  if (!QFile::exists(fileName)) {
    return;
  }

  // Loads .desktop file (read from 'Desktop Entry' group)
  getProperties(fileName, "Desktop Entry");
  name = value("Name", "").toString();
  genericName = value("GenericName", "").toString();
  exec = value("Exec", "").toString();
  icon = value("Icon", "").toString();
  type = value("Type", "Application").toString();
  no_display = value("NoDisplay", false).toBool();
  terminal = value("Terminal", false).toBool();
  categories = value("Categories").toString().remove(" ").split(";");
  mimeType = value("MimeType").toString().remove(" ").split(";");

  // Fix categories
  if (categories.first().compare("") == 0) {
    categories.removeFirst();
  }
}
//---------------------------------------------------------------------------

QStringList MimeUtils::applicationLocations(QString appPath)
{
    QStringList result;
    result << QString("%1/.local/share/applications").arg(QDir::homePath());
    result << QString("%1/../share/applications").arg(appPath);
    result << "/usr/share/applications" << "/usr/local/share/applications";
    return result;
}

QString MimeUtils::findApplication(QString appPath, QString desktopFile)
{
    QString result;
    if (desktopFile.isEmpty()) { return result; }
    QStringList apps = applicationLocations(appPath);
    for (int i=0;i<apps.size();++i) {
        QDirIterator it(apps.at(i), QStringList("*.desktop"), QDir::Files|QDir::NoDotAndDotDot);
        while (it.hasNext()) {
            QString found = it.next();
            if (found.split("/").takeLast()==desktopFile) {
                //qDebug() << "found app" << found;
                return found;
            }
        }
    }
    return result;
}

void MimeUtils::checkAndKillRunningApp(QString &appName)
{
    QProcess p;
    QString s;

    s = "ps -aux";
    p.start(s);
    p.waitForStarted();
    p.waitForFinished();
    QByteArray b = p.readAll();
    QString out = QString::fromLocal8Bit(b);
    if(out.contains(appName)){
        QString str = appName.mid(appName.lastIndexOf('/') + 1, s.size());
        s = "killall " + str;
        qDebug() << "killing existing app: " + str;
        p.start(s);
        p.waitForFinished();
    }
}

/**
 * @brief Opens file in a default application
 * @param file
 * @param processOwner
 */
void MimeUtils::openInApp(const QFileInfo &file, QString termCmd) {
  qDebug() << "openInApp without app";
  QString mime = getMimeType(file.absoluteFilePath());
  load(defaultsFileName, "Default Applications");
  QString app = value(mime).toString().split(";").first();
  if (app.isEmpty() && mime.startsWith("text/") && mime != "text/plain") {
      // fallback for text
      app = value("text/plain").toString().split(";").first();
  }
  QString desktop = findApplication(qApp->applicationFilePath(), app);
  qDebug() << "openInApp" << file.absoluteFilePath() << termCmd << mime << app << desktop;
  if (!desktop.isEmpty()) {
    getDesktopFile(desktop);
    if (!terminal) { termCmd.clear(); }
    else {
        if (termCmd.isEmpty()) { termCmd = "xterm"; }
    }
    openInApp(exec, file, termCmd);
  } else {
     QString title = tr("No default application");
     QString msg = tr("No default application for mime: %1!").arg(mime);
     QMessageBox::warning(Q_NULLPTR, title, msg);
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Opens file in a given application
 * @param exe name of application to be executed
 * @param file to be opened in executed application
 * @param processOwner process owner (default NULL)
 */
void MimeUtils::openInApp(QString exe, const QFileInfo &file,
                          QString termCmd) {

  qDebug() << "openInApp" << exe << file.absoluteFilePath() << termCmd;

  // This is not the right the solution, but qpdfview won't start otherwise
  // TODO: Repair it correctly
  if (exe.contains("qpdfview")) {
    exe = "qpdfview";
  }

  // Separate application name from its arguments
  QStringList split = exe.split(" ");
  QString name = split.takeAt(0);
  QString args = split.join(" ");

  // Get relative path
  //args = args.split(QDir::separator()).last();

  // Replace parameters with file name. If there are no parameters simply append
  // file name to the end of argument list
  if (args.toLower().contains("%f")) {
    args.replace("%f", "\"" + file.filePath() + "\"", Qt::CaseInsensitive);
  } else if (args.toLower().contains("%u")) {
    args.replace("%u", "\"" + file.filePath() + "\"", Qt::CaseInsensitive);
  } else {
    args.append(args.isEmpty() ? "" : " ");
    args.append("\"" + file.filePath() + "\"");
  }

  qDebug() << "qprocess start detached" << name << args;
  checkAndKillRunningApp(name);
  // Start application
 /* QProcess *myProcess = new QProcess(processOwner);
  myProcess->startDetached(name, QStringList() << args);
  myProcess->waitForFinished(1000);
  //myProcess->terminate();*/
  //Q_UNUSED(processOwner)
  QString cmd = name;
  if (termCmd.isEmpty()) {
    cmd.append(" ");
    cmd.append(args);
  } else {
    cmd = QString("%1 -e \"%2 %3\"").arg(termCmd).arg(name).arg(args);
  }
  qDebug() << "running:" << cmd;
  QProcess::startDetached(cmd);
}

void MimeUtils::openFiles(const QStringList &files) {
  qDebug() << "openInApp without app";

  QString mime = getMimeType(files[0]);
  load(defaultsFileName, "Default Applications");
  QString app = value(mime).toString().split(";").first();
  QString desktop = findApplication(qApp->applicationFilePath(), app);
  QString file;
  for(int i = 0; i < files.size(); i++){
      file.append(files[i]).append(" ");
  }
  qDebug() << "openInApp" << file << mime << app << desktop;
  if (!desktop.isEmpty()) {
    getDesktopFile(desktop);
//    openInApp(exec, file, termCmd);
    qDebug() << "openInApp" << exec << file;
    // Separate application name from its arguments
    QStringList split = exec.split(" ");
    QString name = split.takeAt(0);
    QString args = split.join(" ");

    if (args.toLower().contains("%f")) {
      args.replace("%f", file, Qt::CaseInsensitive);
    } else if (args.toLower().contains("%u")) {
      args.replace("%u", file, Qt::CaseInsensitive);
    } else {
      args.append(args.isEmpty() ? "" : " ");
      args.append(file);
    }
    qDebug() << "qprocess start detached" << name << args;
    checkAndKillRunningApp(name);
    QString cmd = name;
    cmd.append(" ");
    cmd.append(args);
    qDebug() << "running:" << cmd;
    QProcess::startDetached(cmd);
  } else {
     QString title = tr("No default application");
     QString msg = tr("No default application for mime: %1!").arg(mime);
     QMessageBox::warning(Q_NULLPTR, title, msg);
  }
}
