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

#include "qmltypenamescriptclass_p.h"

#include "qmlengine_p.h"
#include "qmltypenamecache_p.h"

QT_BEGIN_NAMESPACE

struct TypeNameData : public QScriptDeclarativeClass::Object {
    TypeNameData(QObject *o, QmlType *t, QmlTypeNameScriptClass::TypeNameMode m) : object(o), type(t), typeNamespace(0), mode(m) {}
    TypeNameData(QObject *o, QmlTypeNameCache *n, QmlTypeNameScriptClass::TypeNameMode m) : object(o), type(0), typeNamespace(n), mode(m) {
        if (typeNamespace) typeNamespace->addref();
    }
    ~TypeNameData() {
        if (typeNamespace) typeNamespace->release();
    }

    QObject *object;
    QmlType *type;
    QmlTypeNameCache *typeNamespace;
    QmlTypeNameScriptClass::TypeNameMode mode;
};

QmlTypeNameScriptClass::QmlTypeNameScriptClass(QmlEngine *bindEngine)
: QmlScriptClass(QmlEnginePrivate::getScriptEngine(bindEngine)), 
  engine(bindEngine), object(0), type(0)
{
}

QmlTypeNameScriptClass::~QmlTypeNameScriptClass()
{
}

QScriptValue QmlTypeNameScriptClass::newObject(QObject *object, QmlType *type, TypeNameMode mode)
{
    QScriptEngine *scriptEngine = QmlEnginePrivate::getScriptEngine(engine);

    return QScriptDeclarativeClass::newObject(scriptEngine, this, new TypeNameData(object, type, mode));
}

QScriptValue QmlTypeNameScriptClass::newObject(QObject *object, QmlTypeNameCache *ns, TypeNameMode mode)
{
    QScriptEngine *scriptEngine = QmlEnginePrivate::getScriptEngine(engine);

    return QScriptDeclarativeClass::newObject(scriptEngine, this, new TypeNameData(object, ns, mode));
}

QScriptClass::QueryFlags 
QmlTypeNameScriptClass::queryProperty(Object *obj, const Identifier &name, 
                                      QScriptClass::QueryFlags flags)
{
    Q_UNUSED(flags);

    TypeNameData *data = (TypeNameData *)obj;

    object = 0;
    type = 0;
    QmlEnginePrivate *ep = QmlEnginePrivate::get(engine);

    if (data->typeNamespace) {

        QmlTypeNameCache::Data *d = data->typeNamespace->data(name);
        if (d && d->type) {
            type = d->type;
            return QScriptClass::HandlesReadAccess;
        } else {
            return 0;
        }

    } else {
        Q_ASSERT(data->type);

        QString strName = toString(name);

        if (strName.at(0).isUpper()) {
            // Must be an enum
            if (data->mode == IncludeEnums) {
                // ### Optimize
                QByteArray enumName = strName.toUtf8();
                const QMetaObject *metaObject = data->type->baseMetaObject();
                for (int ii = metaObject->enumeratorCount() - 1; ii >= 0; --ii) {
                    QMetaEnum e = metaObject->enumerator(ii);
                    int value = e.keyToValue(enumName.constData());
                    if (value != -1) {
                        enumValue = value;
                        return QScriptClass::HandlesReadAccess;
                    }
                }
            }
            return 0;
        } else if (data->object) {
            // Must be an attached property
            object = qmlAttachedPropertiesObjectById(data->type->index(), data->object);
            if (!object) return 0;
            return ep->objectClass->queryProperty(object, name, flags, 0);
        }
    }

    return 0;
}

QmlTypeNameScriptClass::ScriptValue 
QmlTypeNameScriptClass::property(Object *obj, const Identifier &name)
{
    QmlEnginePrivate *ep = QmlEnginePrivate::get(engine);
    QScriptEngine *scriptEngine = QmlEnginePrivate::getScriptEngine(engine);
    if (type) {
        return Value(scriptEngine, newObject(((TypeNameData *)obj)->object, type, ((TypeNameData *)obj)->mode));
    } else if (object) {
        return ep->objectClass->property(object, name);
    } else {
        return Value(scriptEngine, enumValue);
    }
}

void QmlTypeNameScriptClass::setProperty(Object *o, const Identifier &n, const QScriptValue &v)
{
    Q_ASSERT(object);
    Q_ASSERT(!type);

    QmlEnginePrivate *ep = QmlEnginePrivate::get(engine);
    ep->objectClass->setProperty(((TypeNameData *)o)->object, n, v);
}

QT_END_NAMESPACE
