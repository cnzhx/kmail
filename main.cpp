// KMail startup and initialize code
// Author: Stefan Taferner <taferner@alpin.or.at>

#include <qstring.h>
#include <qdir.h>
#include "kmglobal.h"
#include "kmmainwin.h"
#include "kmacctmgr.h"
#include "kmfoldermgr.h"
#include "kmsender.h"
#include "kbusyptr.h"
#include <kapp.h>
#include <stdio.h>
#include <stdlib.h>
#include <kmsgbox.h>
#include <klocale.h>

KBusyPtr* kbp = NULL;
KApplication* app = NULL;
KMAcctMgr* acctMgr = NULL;
KMFolderMgr* folderMgr = NULL;
KMSender* msgSender = NULL;
KLocale* nls = NULL;

static msg_handler oldMsgHandler = NULL;


//-----------------------------------------------------------------------------
// Message handler
static void kmailMsgHandler(QtMsgType aType, const char* aMsg)
{
  QString appName = app->appName();

  switch (aType)
  {
  case QtDebugMsg:
    fprintf(stderr, "%s: %s\n", (const char*)app->appName(), aMsg);
    break;

  case QtWarningMsg:
    KMsgBox::message(NULL, appName+" "+nls->translate("warning"), aMsg, 
		     KMsgBox::EXCLAMATION);
    break;

  case QtFatalMsg:
    fprintf(stderr, appName+" "+nls->translate("fatal")+": %s\n", aMsg);
    KMsgBox::message(NULL, appName+nls->translate("fatal"),
		     aMsg, KMsgBox::STOP);
    abort();
  }
}


//-----------------------------------------------------------------------------
static void init(int argc, char *argv[])
{
  QString  fname;
  KConfig* cfg;

  app = new KApplication(argc, argv, "kmail");
  nls = new KLocale;

  kbp = new KBusyPtr(app);
  cfg = app->getConfig();

  oldMsgHandler = qInstallMsgHandler(kmailMsgHandler);

  cfg->setGroup("General");

  fname = QDir::homeDirPath() + 
    cfg->readEntry("accounts", &QString("/.kde/mail-accounts"));
  acctMgr = new KMAcctMgr(fname);

  fname = QDir::homeDirPath() + 
    cfg->readEntry("folders", &QString("/.kde/mail-folders"));
  folderMgr = new KMFolderMgr(fname);

  msgSender = new KMSender(folderMgr);

  debug("init done");
}


//-----------------------------------------------------------------------------
static void cleanup(void)
{
  app->getConfig()->sync();

  qInstallMsgHandler(oldMsgHandler);

  if (msgSender) delete msgSender;
  if (folderMgr) delete folderMgr;
  if (acctMgr) delete acctMgr;
  if (kbp) delete kbp;
}


//-----------------------------------------------------------------------------
main(int argc, char *argv[])
{
  KMMainWin* mainWin;

  init(argc, argv);

  mainWin = new KMMainWin;
  mainWin->show();

  app->exec();

  cleanup();
}

