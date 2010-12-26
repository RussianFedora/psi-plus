#include <QString>
#include <QDir>
#include <QFile>

#ifdef Q_WS_X11
#include <sys/stat.h> // chmod
#endif

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#ifdef Q_WS_MAC
#include <sys/stat.h> // chmod
#include <CoreServices/CoreServices.h>
#endif

#include "psiapplication.h"
#include "applicationinfo.h"
#include "profiles.h"
#ifdef HAVE_CONFIG
#include "config.h"
#endif

// Constants. These should be moved to a more 'dynamically changeable'
// place (like an external file loaded through the resources system)
// Should also be overridable through an optional file.

#define PROG_NAME "Psi+"
#ifdef WEBKIT
#define PROG_VERSION "0.15.3411-webkit Beta" " (" __DATE__ ")" //CVS Builds are dated
#else
#define PROG_VERSION "0.15.3411 Beta" " (" __DATE__ ")" //CVS Builds are dated
#endif
//#define PROG_VERSION "0.15";
#define PROG_CAPS_NODE "http://psi-dev.googlecode.com/caps"
#define PROG_CAPS_VERSION "0.15-beta"
#define PROG_IPC_NAME "org.psi-im.Psi"	// must not contain '\\' character on Windows
#define PROG_OPTIONS_NS "http://psi-im.org/options"
#define PROG_STORAGE_NS "http://psi-im.org/storage"
#define PROG_FILECACHE_NS "http://psi-im.org/filecache"
#ifdef Q_WS_MAC
#define PROG_APPCAST_URL "http://psi-im.org/appcast/psi-mac.xml"
#else
#define PROG_APPCAST_URL ""
#endif

#if defined(Q_WS_X11) && !defined(PSI_DATADIR)
#define PSI_DATADIR "/usr/local/share/psi"
#endif


QString ApplicationInfo::name()
{
	return PROG_NAME;
}


QString ApplicationInfo::version()
{
	return PROG_VERSION;
}

QString ApplicationInfo::capsNode()
{
	return PROG_CAPS_NODE;
}

QString ApplicationInfo::capsVersion()
{
	return PROG_CAPS_VERSION;
}

QString ApplicationInfo::IPCName()
{
	return PROG_IPC_NAME;
}

QString ApplicationInfo::getAppCastURL()
{
	return PROG_APPCAST_URL;
}

QString ApplicationInfo::optionsNS()
{
	return PROG_OPTIONS_NS;
}

QString ApplicationInfo::storageNS()
{
	return PROG_STORAGE_NS;
}	

QString ApplicationInfo::fileCacheNS()
{
	return PROG_FILECACHE_NS;
}

QStringList ApplicationInfo::getCertificateStoreDirs()
{
	QStringList l;
	l += ApplicationInfo::resourcesDir() + "/certs";
	l += ApplicationInfo::homeDir() + "/certs";
	return l;
}

QString ApplicationInfo::getCertificateStoreSaveDir()
{
	QDir certsave(homeDir() + "/certs");
	if(!certsave.exists()) {
		QDir home(homeDir());
		home.mkdir("certs");
	}

	return certsave.path();
}

QString ApplicationInfo::resourcesDir()
{
#if defined(Q_WS_X11)
	return PSI_DATADIR;
#elif defined(Q_WS_WIN)
	return qApp->applicationDirPath();
#elif defined(Q_WS_MAC)
	// FIXME: Clean this up (remko)
	// System routine locates resource files. We "know" that Psi.icns is
	// in the Resources directory.
	QString resourcePath;
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFStringRef resourceCFStringRef = CFStringCreateWithCString(NULL,
									"application.icns", kCFStringEncodingASCII);
	CFURLRef resourceURLRef = CFBundleCopyResourceURL(mainBundle,
											resourceCFStringRef, NULL, NULL);
	if (resourceURLRef) {
		CFStringRef resourcePathStringRef = CFURLCopyFileSystemPath(
					resourceURLRef, kCFURLPOSIXPathStyle);
		const char* resourcePathCString = CFStringGetCStringPtr(
					resourcePathStringRef, kCFStringEncodingASCII);
		if (resourcePathCString) {
			resourcePath = resourcePathCString;
		}
		else { // CFStringGetCStringPtr failed; use fallback conversion
			CFIndex bufferLength = CFStringGetLength(resourcePathStringRef) + 1;
			char* resourcePathCString = new char[ bufferLength ];
			Boolean conversionSuccess = CFStringGetCString(
						resourcePathStringRef, resourcePathCString,
						bufferLength, kCFStringEncodingASCII);
			if (conversionSuccess) {
				resourcePath = resourcePathCString;
			}
			delete [] resourcePathCString;  // I own this
		}
		CFRelease( resourcePathStringRef ); // I own this
	}
	// Remove the tail component of the path
	if (! resourcePath.isNull()) {
		QFileInfo fileInfo(resourcePath);
		resourcePath = fileInfo.absolutePath();
	}
	return resourcePath;
#endif
}

QString ApplicationInfo::libDir()
{
#if defined(Q_OS_UNIX)
	return PSI_LIBDIR;
#else
	return QString();
#endif
}

/** \brief return psi's private read write data directory
  * unix+mac: $HOME/.psi
  * environment variable "PSIDATADIR" overrides
  */
QString ApplicationInfo::homeDir()
{
	// Try the environment override first
	char *p = getenv("PSIDATADIR");
	if(p) {
		return p;
	}

#if defined(Q_WS_X11)
	QDir proghome(QDir::homePath() + "/.psi");
	if(!proghome.exists()) {
		QDir home = QDir::home();
		home.mkdir(".psi");
		chmod(QFile::encodeName(proghome.path()), 0700);
	}
	return proghome.path();
#elif defined(Q_WS_WIN)
	QString base = ((PsiApplication *)qApp)->portableBase();

	if (base.isEmpty()) {
		// Windows 9x
		if(QDir::homePath() == QDir::rootPath()) {
			base = ".";
		}
		// Windows NT/2K/XP variant
		else {
			base = QDir::homePath();
		}
	}
	// no trailing slash
	if(base.at(base.length()-1) == '/') {
		base.truncate(base.length()-1);
	}

	QDir proghome(base + "/PsiData");
	if(!proghome.exists()) {
		QDir home(base);
		home.mkdir("PsiData");
	}

	return proghome.path();
#elif defined(Q_WS_MAC)
	QDir proghome(QDir::homePath() + "/.psi");
	if(!proghome.exists()) {
		QDir home = QDir::home();
		home.mkdir(".psi");
		chmod(QFile::encodeName(proghome.path()), 0700);
	}

	return proghome.path();
#endif
}

QString ApplicationInfo::makeSubprofilePath(const QString &path)
{
	if (path.indexOf("..") == -1) { // ensure its in profile dir
		QDir dir(pathToProfile(activeProfile) + "/" + path);
		if (dir.exists() || dir.mkpath(".")) {
			return dir.path();
		}
	}
	return QString();
}

QString ApplicationInfo::historyDir()
{
	return makeSubprofilePath("history");
}

QString ApplicationInfo::vCardDir()
{
	return makeSubprofilePath("vcard");
}

QString ApplicationInfo::bobDir()
{
	return makeSubprofilePath("bob");
}

QString ApplicationInfo::profilesDir()
{
	QString profiles_dir(homeDir() + "/profiles");
	QDir d(profiles_dir);
	if(!d.exists()) {
		QDir d(homeDir());
		d.mkdir("profiles");
	}
	return profiles_dir;
}
