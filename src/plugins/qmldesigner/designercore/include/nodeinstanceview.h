/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#ifndef NODEINSTANCEVIEW_H
#define NODEINSTANCEVIEW_H

#include "corelib_global.h"
#include "abstractview.h"

#include <modelnode.h>
#include <nodeinstance.h>
#include <nodeinstanceclientinterface.h>

#include <QHash>
#include <QSet>
#include <QWeakPointer>
#include <QRectF>

QT_BEGIN_NAMESPACE
class QDeclarativeEngine;
class QGraphicsView;
class QFileSystemWatcher;
class QPainter;
QT_END_NAMESPACE

namespace QmlDesigner {

class NodeInstanceServerInterface;
class CreateSceneCommand;
class CreateInstancesCommand;
class ClearSceneCommand;
class ReparentInstancesCommand;
class ChangeFileUrlCommand;
class ChangeValuesCommand;
class ChangeBindingsCommand;
class ChangeIdsCommand;
class RemoveInstancesCommand;
class RemovePropertiesCommand;
class AddImportCommand;

class CORESHARED_EXPORT NodeInstanceView : public AbstractView, public NodeInstanceClientInterface
{
    Q_OBJECT

    friend class NodeInstance;

public:
    typedef QWeakPointer<NodeInstanceView> Pointer;

    NodeInstanceView(QObject *parent = 0);
    ~NodeInstanceView();

    void modelAttached(Model *model);
    void modelAboutToBeDetached(Model *model);
    void nodeCreated(const ModelNode &createdNode);
    void nodeAboutToBeRemoved(const ModelNode &removedNode);
    void nodeRemoved(const ModelNode &removedNode, const NodeAbstractProperty &parentProperty, PropertyChangeFlags propertyChange);
    void propertiesAboutToBeRemoved(const QList<AbstractProperty>& propertyList);
    void propertiesRemoved(const QList<AbstractProperty>& propertyList);
    void variantPropertiesChanged(const QList<VariantProperty>& propertyList, PropertyChangeFlags propertyChange);
    void bindingPropertiesChanged(const QList<BindingProperty>& propertyList, PropertyChangeFlags propertyChange);
    void nodeReparented(const ModelNode &node, const NodeAbstractProperty &newPropertyParent, const NodeAbstractProperty &oldPropertyParent, AbstractView::PropertyChangeFlags propertyChange);
    void rootNodeTypeChanged(const QString &type, int majorVersion, int minorVersion);
    void fileUrlChanged(const QUrl &oldUrl, const QUrl &newUrl);
    void nodeIdChanged(const ModelNode& node, const QString& newId, const QString& oldId);
    void nodeOrderChanged(const NodeListProperty &listProperty, const ModelNode &movedNode, int oldIndex);
    void selectedNodesChanged(const QList<ModelNode> &selectedNodeList, const QList<ModelNode> &lastSelectedNodeList);
    void scriptFunctionsChanged(const ModelNode &node, const QStringList &scriptFunctionList);
    void instancePropertyChange(const QList<QPair<ModelNode, QString> > &propertyList);
    void importAdded(const Import &import);
    void importRemoved(const Import &import);

    QList<NodeInstance> instances() const;
    NodeInstance instanceForNode(const ModelNode &node) const ;
    bool hasInstanceForNode(const ModelNode &node) const;

    NodeInstance instanceForId(qint32 id) const;
    bool hasInstanceForId(qint32 id) const;

    QRectF sceneRect() const;

    void setBlockUpdates(bool block);

    NodeInstance activeStateInstance() const;

    void activateState(const NodeInstance &instance);
    void activateBaseState();

    void valuesChanged(const ValuesChangedCommand &command);
    void pixmapChanged(const PixmapChangedCommand &command);
    void informationChanged(const InformationChangedCommand &command);
    void childrenChanged(const ChildrenChangedCommand &command);

private: // functions
    NodeInstance rootNodeInstance() const;

    NodeInstance loadNode(const ModelNode &node);

    void loadNodes(const QList<ModelNode> &nodeList);

    void removeAllInstanceNodeRelationships();

    void removeRecursiveChildRelationship(const ModelNode &removedNode);

    void insertInstanceRelationships(const NodeInstance &instance);
    void removeInstanceNodeRelationship(const ModelNode &node);

    void removeInstanceAndSubInstances(const ModelNode &node);

    void setStateInstance(const NodeInstance &stateInstance);
    void clearStateInstance();

    NodeInstanceServerInterface *nodeInstanceServer() const;

    CreateSceneCommand createCreateSceneCommand() const;
    ClearSceneCommand createClearSceneCommand() const;
    CreateInstancesCommand createCreateInstancesCommand(const QList<NodeInstance> &instanceList) const;
    ReparentInstancesCommand createReparentInstancesCommand(const QList<NodeInstance> &instanceList) const;
    ReparentInstancesCommand createReparentInstancesCommand(const ModelNode &node, const NodeAbstractProperty &newPropertyParent, const NodeAbstractProperty &oldPropertyParent) const;
    ChangeFileUrlCommand createChangeFileUrlCommand(const QUrl &fileUrl) const;
    ChangeValuesCommand createChangeValueCommand(const QList<VariantProperty> &propertyList) const;
    ChangeBindingsCommand createChangeBindingCommand(const QList<BindingProperty> &propertyList) const;
    ChangeIdsCommand createChangeIdsCommand(const QList<NodeInstance> &instanceList) const;
    RemoveInstancesCommand createRemoveInstancesCommand(const QList<ModelNode> &nodeList) const;
    RemoveInstancesCommand createRemoveInstancesCommand(const ModelNode &node) const;
    RemovePropertiesCommand createRemovePropertiesCommand(const QList<AbstractProperty> &propertyList) const;
    AddImportCommand createImportCommand(const Import &import);

    qint32 generateInstanceId();

    void resetHorizontalAnchors(const ModelNode &node);
    void resetVerticalAnchors(const ModelNode &node);

private slots:
    void restartProcess();

private: //variables
    NodeInstance m_rootNodeInstance;
    NodeInstance m_activeStateInstance;

    QHash<ModelNode, NodeInstance> m_nodeInstanceHash;
    QHash<qint32, NodeInstance> m_idInstanceHash; // This is purely internal. Might contain dangling pointers!

    uint m_blockUpdates;
    QWeakPointer<NodeInstanceServerInterface> m_nodeInstanceServer;
    qint32 m_instanceIdCounter;
;
};

} // namespace ProxyNodeInstanceView

#endif // NODEINSTANCEVIEW_H
