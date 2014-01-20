/****************************************************************************
**
** Copyright (C) 2014 Orgad Shaneh <orgads@gmail.com>.
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "cpptoolsplugin.h"
#include "cpptoolsreuse.h"

#include <utils/fileutils.h>

#include <QDir>
#include <QtTest>

static inline QString _(const QByteArray &ba) { return QString::fromLatin1(ba, ba.size()); }

static void createTempFile(const QString &fileName)
{
    QFile file(fileName);
    QDir(QFileInfo(fileName).absolutePath()).mkpath(_("."));
    file.open(QFile::WriteOnly);
    file.close();
}

static QString baseTestDir()
{
    return QDir::tempPath() + _("/qtc_cppheadersource/");
}

namespace CppTools {
namespace Internal {

void CppToolsPlugin::test_headersource()
{
    QFETCH(QString, sourceFileName);
    QFETCH(QString, headerFileName);

    bool wasHeader;
    const QString baseDir = baseTestDir();
    QDir path = QDir(baseDir + _(QTest::currentDataTag()));

    const QString sourcePath = path.absoluteFilePath(sourceFileName);
    const QString headerPath = path.absoluteFilePath(headerFileName);
    createTempFile(sourcePath);
    createTempFile(headerPath);

    clearHeaderSourceCache();
    QCOMPARE(correspondingHeaderOrSource(sourcePath, &wasHeader), headerPath);
    QVERIFY(!wasHeader);
    clearHeaderSourceCache();
    QCOMPARE(correspondingHeaderOrSource(headerPath, &wasHeader), sourcePath);
    QVERIFY(wasHeader);
}

void CppToolsPlugin::test_headersource_data()
{
    QTest::addColumn<QString>("sourceFileName");
    QTest::addColumn<QString>("headerFileName");
    QTest::newRow("samedir") << _("foo.cpp") << _("foo.h");
    QTest::newRow("includesub") << _("foo.cpp") << _("include/foo.h");
}

void CppToolsPlugin::initTestCase()
{
    QDir(baseTestDir()).mkpath(_("."));
}

void CppToolsPlugin::cleanupTestCase()
{
    Utils::FileUtils::removeRecursively(Utils::FileName::fromString(baseTestDir()));
}

} // namespace Internal
} // namespace CppTools
