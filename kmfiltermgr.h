/* Mail Filter Manager
 *
 * Author: Stefan Taferner <taferner@kde.org>
 * This code is under GPL
 */
#ifndef kmfiltermgr_h
#define kmfiltermgr_h

#include "kmfolder.h"

#include <qptrlist.h>
#include <qguardedptr.h>
#include <qobject.h>

class KMFilterDlg;
class KMFilter;

class KMFilterMgr: public QObject, public QPtrList<KMFilter>
{
  Q_OBJECT

public:
  KMFilterMgr(bool popFilter = false);
  virtual ~KMFilterMgr();

  enum FilterSet { NoSet = 0x0, Inbound = 0x1, Outbound = 0x2, Explicit = 0x4,
		   All = Inbound|Outbound|Explicit };

  /** Reload filter rules from config file. */
  void readConfig(void);

  /** Store filter rules in config file. */
  void writeConfig(bool withSync=TRUE);

  /** Open an edit dialog. */
  void openDialog( QWidget *parent );

  /** Open an edit dialog, create a new filter and preset the first
      rule with "field equals value" */
  void createFilter( const QCString & field, const QString & value );

  /** Process given message by applying the filter rules one by
      one. You can select which set of filters (incoming or outgoing)
      should be used.

      @param msg The message to process.
      @param aSet Select the filter set to use.
      @param id If set the id of an individual filter to apply
      @return 2 if a critical error occurred (eg out of disk space)
      1 if the caller is still owner of the message and
      0 otherwise. If the caller does not any longer own the message
      he *must* not delete the message or do similar stupid things. ;-)
  */
  int process( KMMessage * msg, FilterSet aSet=Inbound ) const;

  /** For ad-hoc filters. Applies @p filter to @p msg. Return codes
      are as with the above method. */
  int process( KMMessage * msg, const KMFilter * filter ) const;

  /** Call this method after processing messages with process().
    Shall be called after all messages are processed. This method
    closes all folders that have been temporarily opened with
    tempOpenFolder(). */
  void cleanup();

  /** Open given folder and mark it as temporarily open. The folder
    will be closed upon next call of cleanip(). This method is
    usually only called from within filter actions during process().
    Returns returncode from KMFolder::open() call. */
  int tempOpenFolder(KMFolder* aFolder);

  /** Called at the beginning of an filter list update. Currently a
      no-op */
  void beginUpdate() {}

  /** Called at the end of an filter list update. */
  void endUpdate();

  /** Output all rules to stdout */
#ifndef NDEBUG
  void dump();
#endif

  /** Called from the folder manager when a folder is removed.
    Tests if the folder aFolder is used in any action. Changes
    to aNewFolder folder in this case. Returns TRUE if a change
    occurred. */
  bool folderRemoved(KMFolder* aFolder, KMFolder* aNewFolder);

  /** Called from the folder manager when a new folder has been
      created. Forwards this to the filter dialog if that is open. */
  void folderCreated(KMFolder*) {}

  /** Set the global option 'Show Download Later Messages' */
  void setShowLaterMsgs( bool show ) {
    mShowLater = show;
  }

  /** Get the global option 'Show Download Later Messages' */
  bool showLaterMsgs() const {
    return mShowLater;
  }

signals:
  void filterListUpdated();

private:
  int processPop( KMMessage * msg ) const;

  QGuardedPtr<KMFilterDlg> mEditDialog;
  QPtrList<KMFolder> mOpenFolders;
  bool bPopFilter;
  bool mShowLater;
};

#endif /*kmfiltermgr_h*/
