/*
    This file is part of KMail

    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)
    Copyright (c) 2003 Zack Rusin <zack@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "undostack.h"

#include "kmmainwin.h"
#include "kmkernel.h"
#include <KJob>
#include <AkonadiCore/itemmovejob.h>

#include <kmessagebox.h>
#include <KLocalizedString>
#include "kmail_debug.h"

#include <QList>

using namespace KMail;

UndoStack::UndoStack(int size)
    : QObject(nullptr)
    , mSize(size)
{
}

UndoStack::~UndoStack()
{
    clear();
}

void UndoStack::clear()
{
    qDeleteAll(mStack);
    mStack.clear();
}

int UndoStack::size() const
{
    return mStack.count();
}

QString UndoStack::undoInfo() const
{
    if (!mStack.isEmpty()) {
        UndoInfo *info = mStack.first();
        return info->moveToTrash ? i18n("Move To Trash") : i18np("Move Message", "Move Messages", info->items.count());
    } else {
        return QString();
    }
}

int UndoStack::newUndoAction(const Akonadi::Collection &srcFolder, const Akonadi::Collection &destFolder)
{
    UndoInfo *info = new UndoInfo;
    info->id = ++mLastId;
    info->srcFolder = srcFolder;
    info->destFolder = destFolder;
    info->moveToTrash = (destFolder == CommonKernel->trashCollectionFolder());
    if (static_cast<int>(mStack.count()) == mSize) {
        delete mStack.last();
        mStack.removeLast();
    }
    mStack.prepend(info);
    Q_EMIT undoStackChanged();
    return info->id;
}

void UndoStack::addMsgToAction(int undoId, const Akonadi::Item &item)
{
    if (!mCachedInfo || mCachedInfo->id != undoId) {
        QList<UndoInfo *>::const_iterator itr = mStack.constBegin();
        while (itr != mStack.constEnd()) {
            if ((*itr)->id == undoId) {
                mCachedInfo = (*itr);
                break;
            }
            ++itr;
        }
    }

    Q_ASSERT(mCachedInfo);
    mCachedInfo->items.append(item);
}

bool UndoStack::isEmpty() const
{
    return mStack.isEmpty();
}

void UndoStack::undo()
{
    if (!mStack.isEmpty()) {
        UndoInfo *info = mStack.takeFirst();
        Q_EMIT undoStackChanged();
        Akonadi::ItemMoveJob *job = new Akonadi::ItemMoveJob(info->items, info->srcFolder, this);
        connect(job, &Akonadi::ItemMoveJob::result, this, &UndoStack::slotMoveResult);
        delete info;
    } else {
        // Sorry.. stack is empty..
        KMessageBox::sorry(kmkernel->mainWin(), i18n("There is nothing to undo."));
    }
}

void UndoStack::slotMoveResult(KJob *job)
{
    if (job->error()) {
        KMessageBox::sorry(kmkernel->mainWin(), i18n("Cannot move message. %1", job->errorString()));
    }
}

void UndoStack::pushSingleAction(const Akonadi::Item &item, const Akonadi::Collection &folder, const Akonadi::Collection &destFolder)
{
    const int id = newUndoAction(folder, destFolder);
    addMsgToAction(id, item);
}

void UndoStack::folderDestroyed(const Akonadi::Collection &folder)
{
    QList<UndoInfo *>::iterator it = mStack.begin();
    while (it != mStack.end()) {
        UndoInfo *info = *it;
        if (info
            && ((info->srcFolder == folder)
                || (info->destFolder == folder))) {
            delete info;
            it = mStack.erase(it);
        } else {
            ++it;
        }
    }
    Q_EMIT undoStackChanged();
}
