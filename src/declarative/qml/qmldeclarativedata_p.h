/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QMLDECLARATIVEDATA_P_H
#define QMLDECLARATIVEDATA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtScript/qscriptvalue.h>
#include <private/qobject_p.h>
#include "qmlguard_p.h"

QT_BEGIN_NAMESPACE

class QmlCompiledData;
class QmlAbstractBinding;
class QmlContext;
class QmlPropertyCache;
class Q_AUTOTEST_EXPORT QmlDeclarativeData : public QDeclarativeData
{
public:
    QmlDeclarativeData(QmlContext *ctxt = 0)
        : context(ctxt), bindings(0), nextContextObject(0), prevContextObject(0),
          bindingBitsSize(0), bindingBits(0), outerContext(0), lineNumber(0), 
          columnNumber(0), deferredComponent(0), deferredIdx(0), attachedProperties(0), 
          propertyCache(0), guards(0) {}

    virtual void destroyed(QObject *);

    QmlContext *context;
    QmlAbstractBinding *bindings;

    // Linked list for QmlContext::contextObjects
    QmlDeclarativeData *nextContextObject;
    QmlDeclarativeData**prevContextObject;

    int bindingBitsSize;
    quint32 *bindingBits; 
    bool hasBindingBit(int) const;
    void clearBindingBit(int);
    void setBindingBit(QObject *obj, int);

    QmlContext *outerContext; // Can't this be found from context?
    ushort lineNumber;
    ushort columnNumber;

    QmlCompiledData *deferredComponent; // Can't this be found from the context?
    unsigned int deferredIdx;

    QHash<int, QObject *> *attachedProperties;

    QScriptValue scriptValue;
    QmlPropertyCache *propertyCache;

    QmlGuard<QObject> *guards;

    static QmlDeclarativeData *get(const QObject *object, bool create = false) {
        QObjectPrivate *priv = QObjectPrivate::get(const_cast<QObject *>(object));
        if (priv->declarativeData) {
            return static_cast<QmlDeclarativeData *>(priv->declarativeData);
        } else if (create) {
            priv->declarativeData = new QmlDeclarativeData;
            return static_cast<QmlDeclarativeData *>(priv->declarativeData);
        } else {
            return 0;
        }
    }
};

template<class T>
void QmlGuard<T>::addGuard()
{
    QmlDeclarativeData *data = QmlDeclarativeData::get(o, true);
    next = data->guards;
    if (next) reinterpret_cast<QmlGuard<T> *>(next)->prev = &next;
    data->guards = reinterpret_cast<QmlGuard<QObject> *>(this);
    prev = &data->guards;
}

template<class T>
void QmlGuard<T>::remGuard()
{
    if (next) reinterpret_cast<QmlGuard<T> *>(next)->prev = prev;
    *prev = next;
    next = 0;
    prev = 0;
}

QT_END_NAMESPACE

#endif // QMLDECLARATIVEDATA_P_H