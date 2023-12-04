/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

//#include "desktopenvironment_p.cpp"
#include "xdgdesktopfile.h"
#include <stdlib.h>
#include <unistd.h>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QSharedData>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QList>
#include <QMimeDatabase>
#include <QMimeType>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>
#include <QtAlgorithms>
#define DEFAULT_ICON ":/resources/icon.png"

/**
 *  See: http://standards.freedesktop.org/desktop-entry-spec
 */

// A list of executables that can't be run with QProcess::startDetached(). They
// will be run with QProcess::start()
static const QStringList nonDetachExecs = QStringList()
    << QLatin1String("pkexec");


static const QLatin1String nameKey("Name");
static const QLatin1String typeKey("Type");
static const QLatin1String ApplicationStr("Application");
static const QLatin1String LinkStr("Link");
static const QLatin1String DirectoryStr("Directory");
static const QLatin1String execKey("Exec");
static const QLatin1String iconKey("Icon");
static const QString userDirectoryString[8] =
{
    QLatin1String("Desktop"),
    QLatin1String("Download"),
    QLatin1String("Templates"),
    QLatin1String("Publicshare"),
    QLatin1String("Documents"),
    QLatin1String("Music"),
    QLatin1String("Pictures"),
    QLatin1String("Videos")
};

// Helper functions prototypes
bool checkTryExec(const QString& progName);
QString &doEscape(QString& str, const QHash<QChar,QChar> &repl);
QString &doUnEscape(QString& str, const QHash<QChar,QChar> &repl);
QString &escape(QString& str);
QString &escapeExec(QString& str);
QString expandDynamicUrl(QString url);
QString expandEnvVariables(const QString str);
QStringList expandEnvVariables(const QStringList strs);
QString findDesktopFile(const QString& dirName, const QString& desktopName);
QString findDesktopFile(const QString& desktopName);
static QStringList parseCombinedArgString(const QString &program);
bool read(const QString &prefix);
void replaceVar(QString &str, const QString &varName, const QString &after);
QString &unEscape(QString& str);
QString &unEscapeExec(QString& str);
void loadMimeCacheDir(const QString& dirName, QHash<QString, QList<XdgDesktopFile*> > & cache);
QStringList dataDirs(const QString &postfix);


QString &doUnEscape(QString& str, const QHash<QChar,QChar> &repl)
{
    int n = 0;
    while (1)
    {
        n=str.indexOf(QLatin1String("\\"), n);
        if (n < 0 || n > str.length() - 2)
            break;

        if (repl.contains(str.at(n+1)))
        {
            str.replace(n, 2, repl.value(str.at(n+1)));
        }

        n++;
    }

    return str;
}


/************************************************
 The escape sequences \s, \n, \t, \r, and \\ are supported for values
 of type string and localestring, meaning ASCII space, newline, tab,
 carriage return, and backslash, respectively.
 ************************************************/
QString &unEscape(QString& str)
{
    QHash<QChar,QChar> repl;
    repl.insert(QLatin1Char('\\'), QLatin1Char('\\'));
    repl.insert(QLatin1Char('s'),  QLatin1Char(' '));
    repl.insert(QLatin1Char('n'),  QLatin1Char('\n'));
    repl.insert(QLatin1Char('t'),  QLatin1Char('\t'));
    repl.insert(QLatin1Char('r'),  QLatin1Char('\r'));

    return doUnEscape(str, repl);
}


/************************************************
 Quoting must be done by enclosing the argument between double quotes and
 escaping the
    double quote character,
    backtick character ("`"),
    dollar sign ("$") and
    backslash character ("\")
by preceding it with an additional backslash character.
Implementations must undo quoting before expanding field codes and before
passing the argument to the executable program.

Reserved characters are
    space (" "),
    tab,
    newline,
    double quote,
    single quote ("'"),
    backslash character ("\"),
    greater-than sign (">"),
    less-than sign ("<"),
    tilde ("~"),
    vertical bar ("|"),
    ampersand ("&"),
    semicolon (";"),
    dollar sign ("$"),
    asterisk ("*"),
    question mark ("?"),
    hash mark ("#"),
    parenthesis ("(") and (")")
    backtick character ("`").

Note that the general escape rule for values of type string states that the
backslash character can be escaped as ("\\") as well and that this escape
rule is applied before the quoting rule. As such, to unambiguously represent a
literal backslash character in a quoted argument in a desktop entry file
requires the use of four successive backslash characters ("\\\\").
Likewise, a literal dollar sign in a quoted argument in a desktop entry file
is unambiguously represented with ("\\$").
 ************************************************/
QString &unEscapeExec(QString& str)
{
    unEscape(str);
    QHash<QChar,QChar> repl;
    // The parseCombinedArgString() splits the string by the space symbols,
    // we temporarily replace them on the special characters.
    // Replacement will reverse after the splitting.
    repl.insert(QLatin1Char(' '),  01);    // space
    repl.insert(QLatin1Char('\t'), 02);    // tab
    repl.insert(QLatin1Char('\n'), 03);    // newline,

    repl.insert(QLatin1Char('"'), QLatin1Char('"'));    // double quote,
    repl.insert(QLatin1Char('\''), QLatin1Char('\''));  // single quote ("'"),
    repl.insert(QLatin1Char('\\'), QLatin1Char('\\'));  // backslash character ("\"),
    repl.insert(QLatin1Char('>'), QLatin1Char('>'));    // greater-than sign (">"),
    repl.insert(QLatin1Char('<'), QLatin1Char('<'));    // less-than sign ("<"),
    repl.insert(QLatin1Char('~'), QLatin1Char('~'));    // tilde ("~"),
    repl.insert(QLatin1Char('|'), QLatin1Char('|'));    // vertical bar ("|"),
    repl.insert(QLatin1Char('&'), QLatin1Char('&'));    // ampersand ("&"),
    repl.insert(QLatin1Char(';'), QLatin1Char(';'));    // semicolon (";"),
    repl.insert(QLatin1Char('$'), QLatin1Char('$'));    // dollar sign ("$"),
    repl.insert(QLatin1Char('*'), QLatin1Char('*'));    // asterisk ("*"),
    repl.insert(QLatin1Char('?'), QLatin1Char('?'));    // question mark ("?"),
    repl.insert(QLatin1Char('#'), QLatin1Char('#'));    // hash mark ("#"),
    repl.insert(QLatin1Char('('), QLatin1Char('('));    // parenthesis ("(")
    repl.insert(QLatin1Char(')'), QLatin1Char(')'));    // parenthesis (")")
    repl.insert(QLatin1Char('`'), QLatin1Char('`'));    // backtick character ("`").

    return doUnEscape(str, repl);
}

void fixBashShortcuts(QString &s)
{
    if (s.startsWith(QLatin1Char('~')))
        s = QFile::decodeName(qgetenv("HOME")) + (s).mid(1);
}

void removeEndingSlash(QString &s)
{
    // We don't check for empty strings. Caller must check it.

    // Remove the ending slash, except for root dirs.
    if (s.length() > 1 && s.endsWith(QLatin1Char('/')))
        s.chop(1);
}

void cleanAndAddPostfix(QStringList &dirs, const QString& postfix)
{
    const int N = dirs.count();
    for(int i = 0; i < N; ++i)
    {
        fixBashShortcuts(dirs[i]);
        removeEndingSlash(dirs[i]);
        dirs[i].append(postfix);
    }
}

namespace
{
    /*!
     * Helper class for getting the keys for "Additional applications actions"
     * ([Desktop Action %s] sections)
     */
    class XdgDesktopAction : public XdgDesktopFile
    {
    public:
        XdgDesktopAction(const XdgDesktopFile & parent, const QString & action)
            : XdgDesktopFile(parent)
            , m_prefix(QString{QLatin1String("Desktop Action %1")}.arg(action))
        {}

    protected:
        virtual QString prefix() const { return m_prefix; }

    private:
        const QString m_prefix;
    };
}

class XdgDesktopFileData: public QSharedData {
public:
    XdgDesktopFileData();

    inline void clear() {
        mFileName.clear();
        mIsValid = false;
        mValidIsChecked = false;
        mIsShow.clear();
        mItems.clear();
        mType = XdgDesktopFile::UnknownType;
    }
    bool read(const QString &prefix);
    XdgDesktopFile::Type detectType(XdgDesktopFile *q) const;
    bool startApplicationDetached(const XdgDesktopFile *q, const QString & action, const QStringList& urls) const;

    QString mFileName;
    bool mIsValid;
    mutable bool mValidIsChecked;
    mutable QHash<QString, bool> mIsShow;
    QMap<QString, QVariant> mItems;

    XdgDesktopFile::Type mType;
};


XdgDesktopFileData::XdgDesktopFileData():
    mFileName(),
    mIsValid(false),
    mValidIsChecked(false),
    mIsShow(),
    mItems(),
    mType(XdgDesktopFile::UnknownType)
{
}


bool XdgDesktopFileData::read(const QString &prefix)
{
    QFile file(mFileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QString section;
    QTextStream stream(&file);
    bool prefixExists = false;
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();

        // Skip comments ......................
        if (line.startsWith(QLatin1Char('#')))
            continue;


        // Section ..............................
        if (line.startsWith(QLatin1Char('[')) && line.endsWith(QLatin1Char(']')))
        {
            section = line.mid(1, line.length()-2);
            if (section == prefix)
                prefixExists = true;

            continue;
        }

        QString key = line.section(QLatin1Char('='), 0, 0).trimmed();
        QString value = line.section(QLatin1Char('='), 1).trimmed();

        if (key.isEmpty())
            continue;

        mItems[section + QLatin1Char('/') + key] = QVariant(value);
    }


    // Not check for empty prefix
    mIsValid = (prefix.isEmpty()) || prefixExists;
    return mIsValid;
}

XdgDesktopFile::Type XdgDesktopFileData::detectType(XdgDesktopFile *q) const
{
    QString typeStr = q->value(typeKey).toString();
    if (typeStr == ApplicationStr)
        return XdgDesktopFile::ApplicationType;

    if (typeStr == LinkStr)
        return XdgDesktopFile::LinkType;

    if (typeStr == DirectoryStr)
        return XdgDesktopFile::DirectoryType;

    if (!q->value(execKey).toString().isEmpty())
        return XdgDesktopFile::ApplicationType;

    return XdgDesktopFile::UnknownType;
}

bool XdgDesktopFileData::startApplicationDetached(const XdgDesktopFile *q, const QString & action, const QStringList& urls) const
{
    //DBusActivatable handling
    if (q->value(QLatin1String("DBusActivatable"), false).toBool()) {
        /* WARNING: We fallback to use Exec when the DBusActivatable fails.
         *
         * This is a violation of the standard and we know it!
         *
         * From the Standard:
         * DBusActivatable	A boolean value specifying if D-Bus activation is
         * supported for this application. If this key is missing, the default
         * value is false. If the value is true then implementations should
         * ignore the Exec key and send a D-Bus message to launch the
         * application. See D-Bus Activation for more information on how this
         * works. Applications should still include Exec= lines in their desktop
         * files for compatibility with implementations that do not understand
         * the DBusActivatable key.
         *
         * So, why are we doing it ? In the benefit of user experience.
         * We first ignore the Exec line and in use the D-Bus to lauch the
         * application. But if it fails, we try the Exec method.
         *
         * We consider that this violation is more acceptable than an failure
         * in launching an application.
         */

            return false;
    }
    QStringList args = action.isEmpty()
        ? q->expandExecString(urls)
        : XdgDesktopAction{*q, action}.expandExecString(urls);

    if (args.isEmpty())
        return false;

    if (q->value(QLatin1String("Terminal")).toBool())
    {
        QString term = QString::fromLocal8Bit(qgetenv("TERM"));
        if (term.isEmpty())
            term = QLatin1String("xterm");

        args.prepend(QLatin1String("-e"));
        args.prepend(term);
    }

    bool nonDetach = false;
    for (const QString &s : nonDetachExecs)
    {
        for (const QString &a : const_cast<const QStringList&>(args))
        {
            if (a.contains(s))
            {
                nonDetach = true;
            }
        }
    }

    QString cmd = args.takeFirst();
    QString workingDir = q->value(QLatin1String("Path")).toString();
    if (!workingDir.isEmpty() && !QDir(workingDir).exists())
	    workingDir = QString();

    if (nonDetach)
    {
        QScopedPointer<QProcess> p(new QProcess);
        p->setStandardInputFile(QProcess::nullDevice());
        p->setProcessChannelMode(QProcess::ForwardedChannels);
        if (!workingDir.isEmpty())
            p->setWorkingDirectory(workingDir);
        p->start(cmd, args);
        bool started = p->waitForStarted();
        if (started)
        {
            QProcess* proc = p.take(); //release the pointer(will be selfdestroyed upon finish)
            QObject::connect(proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                proc, &QProcess::deleteLater);
        }
        return started;
    }
    else
    {
        return QProcess::startDetached(cmd, args, workingDir);
    }
}

XdgDesktopFile::XdgDesktopFile():
    d(new XdgDesktopFileData)
{
}


XdgDesktopFile::XdgDesktopFile(const XdgDesktopFile& other):
    d(other.d)
{
}

XdgDesktopFile::~XdgDesktopFile()
{
}

bool XdgDesktopFile::load(const QString& fileName)
{
    d->clear();
    if (fileName.startsWith(QDir::separator())) { // absolute path
        QFileInfo f(fileName);
        if (f.exists())
            d->mFileName = f.canonicalFilePath();
        else
            return false;
    } else { // relative path
        const QString r = findDesktopFile(fileName);
        if (r.isEmpty())
            return false;
        else
            d->mFileName = r;
    }
    d->read(prefix());
    d->mIsValid = d->mIsValid && check();
    d->mType = d->detectType(this);
    return isValid();
}

QVariant XdgDesktopFile::value(const QString& key, const QVariant& defaultValue) const
{
    QString path = (!prefix().isEmpty()) ? prefix() + QLatin1Char('/') + key : key;
    QVariant res = d->mItems.value(path, defaultValue);
    if (res.type() == QVariant::String)
    {
        QString s = res.toString();
        return unEscape(s);
    }

    return res;
}


/************************************************
 LC_MESSAGES value      Possible keys in order of matching
 lang_COUNTRY@MODIFIER  lang_COUNTRY@MODIFIER, lang_COUNTRY, lang@MODIFIER, lang,
                        default value
 lang_COUNTRY           lang_COUNTRY, lang, default value
 lang@MODIFIER          lang@MODIFIER, lang, default value
 lang                   lang, default value
 ************************************************/
QString XdgDesktopFile::localizedKey(const QString& key) const
{
    QString lang = QString::fromLocal8Bit(qgetenv("LC_MESSAGES"));

    if (lang.isEmpty())
        lang = QString::fromLocal8Bit(qgetenv("LC_ALL"));

    if (lang.isEmpty())
         lang = QString::fromLocal8Bit(qgetenv("LANG"));

    QString modifier = lang.section(QLatin1Char('@'), 1);
    if (!modifier.isEmpty())
        lang.truncate(lang.length() - modifier.length() - 1);

    QString encoding = lang.section(QLatin1Char('.'), 1);
    if (!encoding.isEmpty())
        lang.truncate(lang.length() - encoding.length() - 1);


    QString country = lang.section(QLatin1Char('_'), 1);
    if (!country.isEmpty())
        lang.truncate(lang.length() - country.length() - 1);

    //qDebug() << "LC_MESSAGES: " << qgetenv("LC_MESSAGES");
    //qDebug() << "Lang:" << lang;
    //qDebug() << "Country:" << country;
    //qDebug() << "Encoding:" << encoding;
    //qDebug() << "Modifier:" << modifier;


    if (!modifier.isEmpty() && !country.isEmpty())
    {
        QString k = QString::fromLatin1("%1[%2_%3@%4]").arg(key, lang, country, modifier);
        //qDebug() << "\t try " << k << contains(k);
        if (contains(k))
            return k;
    }

    if (!country.isEmpty())
    {
        QString k = QString::fromLatin1("%1[%2_%3]").arg(key, lang, country);
        //qDebug() << "\t try " << k  << contains(k);
        if (contains(k))
            return k;
    }

    if (!modifier.isEmpty())
    {
        QString k = QString::fromLatin1("%1[%2@%3]").arg(key, lang, modifier);
        //qDebug() << "\t try " << k  << contains(k);
        if (contains(k))
            return k;
    }

    QString k = QString::fromLatin1("%1[%2]").arg(key, lang);
    //qDebug() << "\t try " << k  << contains(k);
    if (contains(k))
        return k;


    //qDebug() << "\t try " << key  << contains(key);
    return key;
}


QVariant XdgDesktopFile::localizedValue(const QString& key, const QVariant& defaultValue) const
{
    return value(localizedKey(key), defaultValue);
}

bool XdgDesktopFile::contains(const QString& key) const
{
    QString path = (!prefix().isEmpty()) ? prefix() + QLatin1Char('/') + key : key;
    return d->mItems.contains(path);
}


bool XdgDesktopFile::isValid() const
{
    return d->mIsValid;
}

/************************************************
 Returns the QIcon corresponding to name in the current icon theme. If no such icon
 is found in the current theme fallback is return instead.
 ************************************************/
QIcon XdgDesktopFile::fromTheme(const QString& iconName, int size, const QIcon& fallback)
{
    return QIcon::fromTheme(iconName, fallback);
}

QIcon const XdgDesktopFile::icon(int size, const QIcon& fallback) const
{
    QIcon result = fromTheme(value(iconKey).toString(), size,  fallback);

    if (result.isNull() && type() == ApplicationType) {
        result = fromTheme(QLatin1String("application-x-executable.png"), size);
        // TODO Maybe defaults for other desktopfile types as well..
    }

    return result;
}


XdgDesktopFile::Type XdgDesktopFile::type() const
{
    return d->mType;
}


/************************************************
 Starts the program defined in this desktop file in a new process, and detaches
 from it. Returns true on success; otherwise returns false. If the calling process
 exits, the detached process will continue to live.

 Urls - the list of URLs or files to open, can be empty (app launched without
  argument)
 If the function is successful then *pid is set to the process identifier of the
 started process.
 ************************************************/
bool XdgDesktopFile::startDetached(const QStringList& urls) const
{
    switch(d->mType)
    {
    case ApplicationType:
        return d->startApplicationDetached(this, QString{}, urls);
        break;

    case LinkType:
        return false;
        break;

    default:
        return false;
    }
}

/************************************************
 This is an overloaded function.
 ************************************************/
bool XdgDesktopFile::startDetached(const QString& url) const
{
    if (url.isEmpty())
        return startDetached(QStringList());
    else
        return startDetached(QStringList(url));
}


static QStringList parseCombinedArgString(const QString &program)
{
    QStringList args;
    QString tmp;
    int quoteCount = 0;
    bool inQuote = false;

    // handle quoting. tokens can be surrounded by double quotes
    // "hello world". three consecutive double quotes represent
    // the quote character itself.
    for (int i = 0; i < program.size(); ++i) {
        if (program.at(i) == QLatin1Char('"')) {
            ++quoteCount;
            if (quoteCount == 3) {
                // third consecutive quote
                quoteCount = 0;
                tmp += program.at(i);
            }
            continue;
        }
        if (quoteCount) {
            if (quoteCount == 1)
                inQuote = !inQuote;
            quoteCount = 0;
        }
        if (!inQuote && program.at(i).isSpace()) {
            if (!tmp.isEmpty()) {
                args += tmp;
                tmp.clear();
            }
        } else {
            tmp += program.at(i);
        }
    }
    if (!tmp.isEmpty())
        args += tmp;

    return args;
}


void replaceVar(QString &str, const QString &varName, const QString &after)
{
    str.replace(QRegExp(QString::fromLatin1("\\$%1(?!\\w)").arg(varName)), after);
    str.replace(QRegExp(QString::fromLatin1("\\$\\{%1\\}").arg(varName)), after);
}

QString userDir(UserDirectory dir)
{
    // possible values for UserDirectory
    Q_ASSERT(!(dir < Desktop || dir > Videos));
    if (dir < Desktop || dir > Videos)
        return QString();

    QString folderName = userDirectoryString[dir];
    QString fallback;
    const QString home = QFile::decodeName(qgetenv("HOME"));

    if (home.isEmpty())
        return QString::fromLatin1("/tmp");
    else if (dir == Desktop)
        fallback = QString::fromLatin1("%1/%2").arg(home, QLatin1String("Desktop"));
    else
        fallback = home;

    return fallback;

    QString s = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    fixBashShortcuts(s);
    QDir d(s);
    if (!d.exists())
    {
        if (!d.mkpath(QLatin1String(".")))
        {
            qWarning() << QString::fromLatin1("Can't create %1 directory.").arg(d.absolutePath());
        }
    }
    QString r = d.absolutePath();
    removeEndingSlash(r);
    QString configDir(s);
    QFile configFile(configDir + QLatin1String("/user-dirs.dirs"));
    if (!configFile.exists())
        return fallback;

    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return fallback;

    QString userDirVar(QLatin1String("XDG_") + folderName.toUpper() + QLatin1String("_DIR"));
    QTextStream in(&configFile);
    QString line;
    while (!in.atEnd())
    {
        line = in.readLine();
        if (line.contains(userDirVar))
        {
            configFile.close();

            // get path between quotes
            line = line.section(QLatin1Char('"'), 1, 1);
            if (line.isEmpty())
                return fallback;
            line.replace(QLatin1String("$HOME"), QLatin1String("~"));
            fixBashShortcuts(line);
            return line;
        }
    }

    configFile.close();
    return fallback;
}

QString expandEnvVariables(const QString str)
{
    QString scheme = QUrl(str).scheme();

    if (scheme == QLatin1String("http")   || scheme == QLatin1String("https") || scheme == QLatin1String("shttp") ||
        scheme == QLatin1String("ftp")    || scheme == QLatin1String("ftps")  ||
        scheme == QLatin1String("pop")    || scheme == QLatin1String("pops")  ||
        scheme == QLatin1String("imap")   || scheme == QLatin1String("imaps") ||
        scheme == QLatin1String("mailto") ||
        scheme == QLatin1String("nntp")   ||
        scheme == QLatin1String("irc")    ||
        scheme == QLatin1String("telnet") ||
        scheme == QLatin1String("xmpp")   ||
        scheme == QLatin1String("nfs")
      )
        return str;

    const QString homeDir = QFile::decodeName(qgetenv("HOME"));

    QString res = str;
    res.replace(QRegExp(QString::fromLatin1("~(?=$|/)")), homeDir);

    replaceVar(res, QLatin1String("HOME"), homeDir);
    replaceVar(res, QLatin1String("USER"), QString::fromLocal8Bit(qgetenv("USER")));

    replaceVar(res, QLatin1String("XDG_DESKTOP_DIR"),   userDir(Desktop));
    replaceVar(res, QLatin1String("XDG_TEMPLATES_DIR"), userDir(Templates));
    replaceVar(res, QLatin1String("XDG_DOCUMENTS_DIR"), userDir(Documents));
    replaceVar(res, QLatin1String("XDG_MUSIC_DIR"), userDir(Music));
    replaceVar(res, QLatin1String("XDG_PICTURES_DIR"), userDir(Pictures));
    replaceVar(res, QLatin1String("XDG_VIDEOS_DIR"), userDir(Videos));
    replaceVar(res, QLatin1String("XDG_PHOTOS_DIR"), userDir(Pictures));

    return res;
}


QStringList XdgDesktopFile::expandExecString(const QStringList& urls) const
{
    if (d->mType != ApplicationType)
        return QStringList();

    QStringList result;

    QString execStr = value(execKey).toString();
    unEscapeExec(execStr);
    const QStringList tokens = parseCombinedArgString(execStr);

    for (QString token : tokens)
    {
        // The parseCombinedArgString() splits the string by the space symbols,
        // we temporarily replaced them on the special characters.
        // Now we reverse it.
        token.replace(01, QLatin1Char(' '));
        token.replace(02, QLatin1Char('\t'));
        token.replace(03, QLatin1Char('\n'));

        // ----------------------------------------------------------
        // A single file name, even if multiple files are selected.
        if (token == QLatin1String("%f"))
        {
            continue;
        }

        // ----------------------------------------------------------
        // A list of files. Use for apps that can open several local files at once.
        // Each file is passed as a separate argument to the executable program.
        if (token == QLatin1String("%F"))
        {
            continue;
        }

        // ----------------------------------------------------------
        // A single URL. Local files may either be passed as file: URLs or as file path.
        if (token == QLatin1String("%u"))
        {
            if (!urls.isEmpty())
            {
                QUrl url;
                url.setUrl(expandEnvVariables(urls.at(0)));
                result << ((!url.toLocalFile().isEmpty()) ? url.toLocalFile() : QString::fromUtf8(url.toEncoded()));
            }
            continue;
        }

        // ----------------------------------------------------------
        // A list of URLs. Each URL is passed as a separate argument to the executable
        // program. Local files may either be passed as file: URLs or as file path.
        if (token == QLatin1String("%U"))
        {
            for (const QString &s : urls)
            {
                QUrl url(expandEnvVariables(s));
                result << ((!url.toLocalFile().isEmpty()) ? url.toLocalFile() : QString::fromUtf8(url.toEncoded()));
            }
            continue;
        }

        // ----------------------------------------------------------
        // The Icon key of the desktop entry expanded as two arguments, first --icon
        // and then the value of the Icon key. Should not expand to any arguments if
        // the Icon key is empty or missing.
        if (token == QLatin1String("%i"))
        {
            QString icon = value(iconKey).toString();
            if (!icon.isEmpty())
                result << QLatin1String("-icon") << icon.replace(QLatin1Char('%'), QLatin1String("%%"));
            continue;
        }


        // ----------------------------------------------------------
        // The translated name of the application as listed in the appropriate Name key
        // in the desktop entry.
        if (token == QLatin1String("%c"))
        {
            result << localizedValue(nameKey).toString().replace(QLatin1Char('%'), QLatin1String("%%"));
            continue;
        }

        // ----------------------------------------------------------
        // The location of the desktop file as either a URI (if for example gotten from
        // the vfolder system) or a local filename or empty if no location is known.
        if (token == QLatin1String("%k"))
        {
            break;
        }

        // ----------------------------------------------------------
        // Deprecated.
        // Deprecated field codes should be removed from the command line and ignored.
        if (token == QLatin1String("%d") || token == QLatin1String("%D") ||
            token == QLatin1String("%n") || token == QLatin1String("%N") ||
            token == QLatin1String("%v") || token == QLatin1String("%m")
            )
        {
            continue;
        }

        // ----------------------------------------------------------
        result << expandEnvVariables(token);
    }

    return result;
}


QString findDesktopFile(const QString& dirName, const QString& desktopName)
{
    QDir dir(dirName);
    QFileInfo fi(dir, desktopName);

    if (fi.exists())
        return fi.canonicalFilePath();

    // Working recursively ............
    const QFileInfoList dirs = dir.entryInfoList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &d : dirs)
    {
        QString cn = d.canonicalFilePath();
        if (dirName != cn)
        {
            QString f = findDesktopFile(cn, desktopName);
            if (!f.isEmpty())
                return f;
        }
    }

    return QString();
}

QString findDesktopFile(const QString& desktopName)
{
    QString d = QFile::decodeName(qgetenv("XDG_DATA_DIRS"));
    QStringList dirs = d.split(QLatin1Char(':'), QString::SkipEmptyParts);

    if (dirs.isEmpty()) {
        dirs.append(QString::fromLatin1("/usr/local/share"));
        dirs.append(QString::fromLatin1("/usr/share"));
    } else {
        QMutableListIterator<QString> it(dirs);
        while (it.hasNext()) {
            const QString dir = it.next();
            if (!dir.startsWith(QLatin1Char('/')))
                it.remove();
        }
    }

    dirs.removeDuplicates();
    cleanAndAddPostfix(dirs, QString());

    QString s = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    fixBashShortcuts(s);
    removeEndingSlash(s);
    dirs.prepend(s);
    for (const QString &dirName : const_cast<const QStringList&>(dirs))
    {
        QString f = findDesktopFile(dirName + QLatin1String("/applications"), desktopName);
        if (!f.isEmpty())
            return f;
    }

    return QString();
}


    // First, we look in following places for a default in specified order:
    // ~/.config/mimeapps.list
    // /etc/xdg/mimeapps.list
    // ~/.local/share/applications/mimeapps.list
    // /usr/local/share/applications/mimeapps.list
    // /usr/share/applications/mimeapps.list


