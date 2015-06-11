/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#ifndef LOCALAPPLICATIONRUNCONTROL_H
#define LOCALAPPLICATIONRUNCONTROL_H

#include "runconfiguration.h"
#include "applicationlauncher.h"

namespace ProjectExplorer {
namespace Internal {

class LocalApplicationRunControlFactory : public IRunControlFactory
{
    Q_OBJECT
public:
    LocalApplicationRunControlFactory ();
    ~LocalApplicationRunControlFactory();
    bool canRun(RunConfiguration *runConfiguration, RunMode mode) const;
    RunControl* create(RunConfiguration *runConfiguration, RunMode mode, QString *errorMessage);
};

} // namespace Internal

class PROJECTEXPLORER_EXPORT LocalApplicationRunControl : public RunControl
{
    Q_OBJECT
public:
    LocalApplicationRunControl(RunConfiguration *runConfiguration, RunMode mode);
    ~LocalApplicationRunControl();
    void start();
    StopResult stop();
    bool isRunning() const;

    void setCommand(const QString &executable, const QString &commandLineArguments);
    void setApplicationLauncherMode(const ApplicationLauncher::Mode mode);
    void setWorkingDirectory(const QString &workingDirectory);

private slots:
    void processStarted();
    void processExited(int exitCode, QProcess::ExitStatus status);
    void slotAppendMessage(const QString &err, Utils::OutputFormat isError);
private:
    ApplicationLauncher m_applicationLauncher;
    QString m_executable;
    QString m_commandLineArguments;
    ApplicationLauncher::Mode m_runMode;
    bool m_running;
};

} // namespace ProjectExplorer

#endif // LOCALAPPLICATIONRUNCONTROL_H
