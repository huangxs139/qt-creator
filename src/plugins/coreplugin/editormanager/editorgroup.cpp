/***************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact:  Qt Software Information (qt-info@nokia.com)
**
**
** Non-Open Source Usage
**
** Licensees may use this file in accordance with the Qt Beta Version
** License Agreement, Agreement version 2.2 provided with the Software or,
** alternatively, in accordance with the terms contained in a written
** agreement between you and Nokia.
**
** GNU General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU General
** Public License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the packaging
** of this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
**
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt GPL Exception
** version 1.3, included in the file GPL_EXCEPTION.txt in this package.
**
***************************************************************************/

#include "editorgroup.h"

#include "editormanager.h"

#include <coreplugin/coreconstants.h>
#include <utils/qtcassert.h>

#include <QtCore/QDir>
#include <QtCore/QDebug>

#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>
#ifdef Q_WS_MAC
#include <QtGui/QMacStyle>
#endif

Q_DECLARE_METATYPE(Core::IEditor*)

using namespace Core;
using namespace Core::Internal;

namespace Core {
namespace Internal {
class EditorList;
}
}

QDataStream &operator<<(QDataStream &out, const Core::Internal::EditorList &list);
QDataStream &operator>>(QDataStream &in, Core::Internal::EditorList &list);

namespace Core {
namespace Internal {

class EditorList
{
public:
    quint32 currentEditorIndex;
    void append(Core::IEditor *editor);
    QString fileNameAt(int index);
    QString editorKindAt(int index);
    int count();

private:
    QList<QPair<QString,QString> > editorinfo;

    friend QDataStream &::operator<<(QDataStream &out, const EditorList &list);
    friend QDataStream &::operator>>(QDataStream &in, EditorList &list);
};

} // namespace Internal
} // namespace Core

//================EditorModel====================
int EditorModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

int EditorModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_editors.count();
    return 0;
}

void EditorModel::insertEditor(int index, IEditor *editor)
{
    beginInsertRows(QModelIndex(), index, index);
    m_editors.insert(index, editor);
    connect(editor, SIGNAL(changed()), this, SLOT(itemChanged()));
    endInsertRows();
}

void EditorModel::removeEditor(IEditor *editor)
{
    int idx = m_editors.indexOf(editor);
    if (idx < 0)
        return;
    beginRemoveRows(QModelIndex(), idx, idx);
    m_editors.removeAt(idx);
    endRemoveRows();
    disconnect(editor, SIGNAL(changed()), this, SLOT(itemChanged()));
}


void EditorModel::emitDataChanged(IEditor *editor)
{
    int idx = m_editors.indexOf(editor);
    if (idx < 0)
        return;
    QModelIndex mindex = index(idx, 0);
    emit dataChanged(mindex, mindex);
}

QModelIndex EditorModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (column != 0 || row < 0 || row >= m_editors.count())
        return QModelIndex();
    return createIndex(row, column);
}

QVariant EditorModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    IEditor *editor = m_editors.at(index.row());
    QTC_ASSERT(editor, return QVariant());
    switch (role) {
    case Qt::DisplayRole:
        return editor->file()->isModified()
                ? editor->displayName() + QLatin1String("*")
                : editor->displayName();
    case Qt::DecorationRole:
        return editor->file()->isReadOnly()
                ? QIcon(QLatin1String(":/core/images/locked.png"))
                : QIcon();
    case Qt::ToolTipRole:
        return editor->file()->fileName().isEmpty()
                ? editor->displayName()
                : QDir::toNativeSeparators(editor->file()->fileName());
    case Qt::UserRole:
        return qVariantFromValue(editor);
    default:
        return QVariant();
    }
    return QVariant();
}

QModelIndex EditorModel::indexOf(IEditor *editor) const
{
    int idx = m_editors.indexOf(editor);
    if (idx < 0)
        return indexOf(editor->file()->fileName());
    return createIndex(idx, 0);
}

QModelIndex EditorModel::indexOf(const QString &fileName) const
{
    for (int i = 0; i < m_editors.count(); ++i)
        if (m_editors.at(i)->file()->fileName() == fileName)
            return createIndex(i, 0);
    return QModelIndex();
}


void EditorModel::itemChanged()
{
    emitDataChanged(qobject_cast<IEditor*>(sender()));
}

//================EditorGroupContext===============

EditorGroupContext::EditorGroupContext(EditorGroup *editorGroup)
 : IContext(editorGroup),
     m_context(QList<int>() << Constants::C_GLOBAL_ID),
     m_editorGroup(editorGroup)
{
}

QList<int> EditorGroupContext::context() const
{
    return m_context;
}

QWidget *EditorGroupContext::widget()
{
    return m_editorGroup;
}

EditorGroup *EditorGroupContext::editorGroup()
{
    return m_editorGroup;
}

//================EditorGroup=================

EditorGroup::EditorGroup(QWidget *parent)
    : QFrame(parent),
      m_contextObject(new EditorGroupContext(this))
{
    setFocusPolicy(Qt::StrongFocus);

    m_model = new EditorModel(this);
}

QSize EditorGroup::minimumSizeHint() const
{
    return QSize(10, 10);
}

void EditorGroup::focusInEvent(QFocusEvent *)
{
    update();
}

void EditorGroup::focusOutEvent(QFocusEvent *)
{
    update();
}

void EditorGroup::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);
    if (editorCount() == 0) {
        QPainter painter(this);

        // Discreet indication where an editor would be
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        QColor shadeBrush(Qt::black);
        shadeBrush.setAlpha(10);
        painter.setBrush(shadeBrush);
        const int r = 3;
        painter.drawRoundedRect(rect().adjusted(r, r, -r, -r), r * 2, r * 2);

        if (hasFocus()) {
#ifdef Q_WS_MAC
            // With QMacStyle, we have to draw our own focus rect, since I didn't find
            // a way to draw the nice mac focus rect _inside_ this widget
            if (qobject_cast<QMacStyle *>(style())) {
                painter.setPen(Qt::DotLine);
                painter.setBrush(Qt::NoBrush);
                painter.setOpacity(0.75);
                painter.drawRect(rect());
            } else {
#endif
                QStyleOptionFocusRect option;
                option.initFrom(this);
                option.backgroundColor = palette().color(QPalette::Background);

                // Some styles require a certain state flag in order to draw the focus rect
                option.state |= QStyle::State_KeyboardFocusChange;

                style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter);
#ifdef Q_WS_MAC
            }
#endif
        }
    }
}

void EditorGroup::moveEditorsFromGroup(EditorGroup *group)
{
    foreach (IEditor *editor, group->editors()) {
        group->removeEditor(editor);
        addEditor(editor);
    }
}

void EditorGroup::moveEditorFromGroup(EditorGroup *group, IEditor *editor)
{
    group->removeEditor(editor);
    addEditor(editor);
}

QByteArray EditorGroup::saveState() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    EditorList editorinfo;
    IEditor *curr = currentEditor();
    QList<IEditor *> editors = editorsInNaturalOrder();
    for (int j = 0; j < editors.count(); ++j) {
        IEditor *editor = editors.at(j);
        if (editor == curr)
            editorinfo.currentEditorIndex = j;
        editorinfo.append(editor);
    }
    stream << editorinfo;
    return bytes;
}

bool EditorGroup::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    EditorManager *em = EditorManager::instance();
    EditorList editors;
    in >> editors;
    int savedIndex = editors.currentEditorIndex;
    if (savedIndex >= 0 && savedIndex < editors.count())
        em->restoreEditor(editors.fileNameAt(savedIndex), editors.editorKindAt(savedIndex), this);
    for (int j = 0; j < editors.count(); ++j) {
        if (j == savedIndex)
            continue;
        em->restoreEditor(editors.fileNameAt(j), editors.editorKindAt(j), this);
    }
    return true;
}

void EditorGroup::addEditor(IEditor *editor)
{
    m_model->addEditor(editor);
}

void EditorGroup::insertEditor(int i, IEditor *editor)
{
    m_model->insertEditor(i, editor);
}

void EditorGroup::removeEditor(IEditor *editor)
{
    m_model->removeEditor(editor);
}

void EditorGroup::showEditorInfoBar(const QString &, const QString &, const QString &, QObject *, const char *)
{
}

void EditorGroup::hideEditorInfoBar(const QString &)
{
}

void EditorList::append(IEditor *editor)
{
    if (editor->file()->fileName().isEmpty())
        return;
    editorinfo << qMakePair(editor->file()->fileName(), QString(editor->kind()));
}

QDataStream &operator<<(QDataStream &out, const EditorList &list)
{
    //todo: versioning
    out << list.currentEditorIndex << list.editorinfo;
    return out;
}

QDataStream &operator>>(QDataStream &in, EditorList &list)
{
    //todo: versioning
    in >> list.currentEditorIndex;
    in >> list.editorinfo;
    return in;
}

QString EditorList::fileNameAt(int index)
{
    return editorinfo.at(index).first;
}

QString EditorList::editorKindAt(int index)
{
    return editorinfo.at(index).second;
}

int EditorList::count()
{
    return editorinfo.count();
}
