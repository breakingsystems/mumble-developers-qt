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

#include "qmlgraphicstext_p.h"
#include "qmlgraphicstext_p_p.h"
#include <qmlstyledtext_p.h>

#include <qfxperf_p_p.h>

#include <QTextLayout>
#include <QTextLine>
#include <QTextDocument>
#include <QTextCursor>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <qmath.h>

QT_BEGIN_NAMESPACE

/*!
    \qmlclass Text QmlGraphicsText
    \brief The Text item allows you to add formatted text to a scene.
    \inherits Item

    It can display both plain and rich text. For example:

    \qml
    Text { text: "Hello World!"; font.family: "Helvetica"; font.pointSize: 24; color: "red" }
    Text { text: "<b>Hello</b> <i>World!</i>" }
    \endqml

    \image declarative-text.png

    If height and width are not explicitly set, Text will attempt to determine how
    much room is needed and set it accordingly. Unless \c wrap is set, it will always
    prefer width to height (all text will be placed on a single line).

    The \c elide property can alternatively be used to fit a single line of
    plain text to a set width.

    Text provides read-only text. For editable text, see \l TextEdit.
*/

/*!
    \internal
    \class QmlGraphicsText
    \qmlclass Text
    \ingroup group_coreitems

    \brief The QmlGraphicsText class provides a formatted text item that you can add to a QmlView.

    Text was designed for read-only text; it does not allow for any text editing.
    It can display both plain and rich text. For example:

    \qml
    Text { text: "Hello World!"; font.family: "Helvetica"; font.pointSize: 24; color: "red" }
    Text { text: "<b>Hello</b> <i>World!</i>" }
    \endqml

    \image text.png

    If height and width are not explicitly set, Text will attempt to determine how
    much room is needed and set it accordingly. Unless \c wrap is set, it will always
    prefer width to height (all text will be placed on a single line).

    The \c elide property can alternatively be used to fit a line of plain text to a set width.

    A QmlGraphicsText object can be instantiated in Qml using the tag \c Text.
*/
QmlGraphicsText::QmlGraphicsText(QmlGraphicsItem *parent)
  : QmlGraphicsItem(*(new QmlGraphicsTextPrivate), parent)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(QGraphicsItem::ItemHasNoContents, false);
}

QmlGraphicsText::~QmlGraphicsText()
{
}


QmlGraphicsTextPrivate::~QmlGraphicsTextPrivate()
{
}

/*!
    \qmlproperty string Text::font.family
    \qmlproperty bool Text::font.bold
    \qmlproperty bool Text::font.italic
    \qmlproperty bool Text::font.underline
    \qmlproperty real Text::font.pointSize
    \qmlproperty int Text::font.pixelSize

    Set the Text's font attributes.
*/
QFont QmlGraphicsText::font() const
{
    Q_D(const QmlGraphicsText);
    return d->font;
}

void QmlGraphicsText::setFont(const QFont &font)
{
    Q_D(QmlGraphicsText);
    if (d->font == font)
        return;

    d->font = font;

    d->updateLayout();
    d->markImgDirty();
    emit fontChanged(d->font);
}

void QmlGraphicsText::setText(const QString &n)
{
#ifdef Q_ENABLE_PERFORMANCE_LOG
    QmlPerfTimer<QmlPerf::QmlGraphicsText_setText> st;
#endif
    Q_D(QmlGraphicsText);
    if (d->text == n)
        return;

    d->richText = d->format == RichText || (d->format == AutoText && Qt::mightBeRichText(n));
    if (d->richText) {
        if (!d->doc) {
            d->doc = new QTextDocument(this);
            d->doc->setDocumentMargin(0);
        }
        d->doc->setHtml(n);
    }

    d->text = n;
    d->updateLayout();
    d->markImgDirty();
    emit textChanged(d->text);
}

/*!
    \qmlproperty string Text::text

    The text to display.  Text supports both plain and rich text strings.

    The item will try to automatically determine whether the text should
    be treated as rich text. This determination is made using Qt::mightBeRichText().
*/
QString QmlGraphicsText::text() const
{
    Q_D(const QmlGraphicsText);
    return d->text;
}

void QmlGraphicsText::setColor(const QColor &color)
{
    Q_D(QmlGraphicsText);
    if (d->color == color)
        return;

    d->color = color;
    d->markImgDirty();
    emit colorChanged(d->color);
}

/*!
    \qmlproperty color Text::color

    The text color.

    \qml
    //green text using hexadecimal notation
    Text { color: "#00FF00"; ... }

    //steelblue text using SVG color name
    Text { color: "steelblue"; ... }
    \endqml
*/

QColor QmlGraphicsText::color() const
{
    Q_D(const QmlGraphicsText);
    return d->color;
}

/*!
    \qmlproperty enumeration Text::style

    Set an additional text style.

    Supported text styles are \c Normal, \c Outline, \c Raised and \c Sunken.

    \qml
    Row {
        Text { font.pointSize: 24; text: "Normal" }
        Text { font.pointSize: 24; text: "Raised";  style: Text.Raised;  styleColor: "#AAAAAA" }
        Text { font.pointSize: 24; text: "Outline"; style: Text.Outline; styleColor: "red" }
        Text { font.pointSize: 24; text: "Sunken";  style: Text.Sunken;  styleColor: "#AAAAAA" }
    }
    \endqml

    \image declarative-textstyle.png
*/
QmlGraphicsText::TextStyle QmlGraphicsText::style() const
{
    Q_D(const QmlGraphicsText);
    return d->style;
}

void QmlGraphicsText::setStyle(QmlGraphicsText::TextStyle style)
{
    Q_D(QmlGraphicsText);
    if (d->style == style)
        return;

    d->style = style;
    d->markImgDirty();
    emit styleChanged(d->style);
}

void QmlGraphicsText::setStyleColor(const QColor &color)
{
    Q_D(QmlGraphicsText);
    if (d->styleColor == color)
        return;

    d->styleColor = color;
    d->markImgDirty();
    emit styleColorChanged(d->styleColor);
}

/*!
    \qmlproperty color Text::styleColor

    Defines the secondary color used by text styles.

    \c styleColor is used as the outline color for outlined text, and as the
    shadow color for raised or sunken text. If no style has been set, it is not
    used at all.
 */
QColor QmlGraphicsText::styleColor() const
{
    Q_D(const QmlGraphicsText);
    return d->styleColor;
}

/*!
    \qmlproperty enumeration Text::horizontalAlignment
    \qmlproperty enumeration Text::verticalAlignment

    Sets the horizontal and vertical alignment of the text within the Text items
    width and height.  By default, the text is top-left aligned.

    The valid values for \c horizontalAlignment are \c AlignLeft, \c AlignRight and
    \c AlignHCenter.  The valid values for \c verticalAlignment are \c AlignTop, \c AlignBottom
    and \c AlignVCenter.
*/
QmlGraphicsText::HAlignment QmlGraphicsText::hAlign() const
{
    Q_D(const QmlGraphicsText);
    return d->hAlign;
}

void QmlGraphicsText::setHAlign(HAlignment align)
{
    Q_D(QmlGraphicsText);
    if (d->hAlign == align)
        return;

    d->hAlign = align;
    emit horizontalAlignmentChanged(align);
}

QmlGraphicsText::VAlignment QmlGraphicsText::vAlign() const
{
    Q_D(const QmlGraphicsText);
    return d->vAlign;
}

void QmlGraphicsText::setVAlign(VAlignment align)
{
    Q_D(QmlGraphicsText);
    if (d->vAlign == align)
        return;

    d->vAlign = align;
    emit verticalAlignmentChanged(align);
}

/*!
    \qmlproperty bool Text::wrap

    Set this property to wrap the text to the Text item's width.  The text will only
    wrap if an explicit width has been set.

    Wrapping is done on word boundaries (i.e. it is a "word-wrap"). If the text cannot be
    word-wrapped to the specified width it will be partially drawn outside of the item's bounds.
    If this is undesirable then enable clipping on the item (Item::clip).

    Wrapping is off by default.
*/
//### Future may provide choice of wrap modes, such as QTextOption::WrapAtWordBoundaryOrAnywhere
bool QmlGraphicsText::wrap() const
{
    Q_D(const QmlGraphicsText);
    return d->wrap;
}

void QmlGraphicsText::setWrap(bool w)
{
    Q_D(QmlGraphicsText);
    if (w == d->wrap)
        return;

    d->wrap = w;

    d->updateLayout();
    d->markImgDirty();
    emit wrapChanged(d->wrap);
}

/*!
    \qmlproperty enumeration Text::textFormat

    The way the text property should be displayed.

    Supported text formats are \c AutoText, \c PlainText, \c RichText and \c StyledText

    The default is AutoText.  If the text format is AutoText the text element
    will automatically determine whether the text should be treated as
    rich text.  This determination is made using Qt::mightBeRichText().

    StyledText is an optimized format supporting some basic text
    styling markup, in the style of html 3.2:

    \code
    <font size="4" color="#ff0000">font size and color</font>
    <b>bold</b>
    <i>italic</i>
    <br>
    &gt; &lt; &amp;
    \endcode

    \c StyledText parser is strict, requiring tags to be correctly nested.

    \table
    \row
    \o
    \qml
Column {
    TextEdit {
        font.pointSize: 24
        text: "<b>Hello</b> <i>World!</i>"
    }
    TextEdit {
        font.pointSize: 24
        textFormat: "RichText"
        text: "<b>Hello</b> <i>World!</i>"
    }
    TextEdit {
        font.pointSize: 24
        textFormat: "PlainText"
        text: "<b>Hello</b> <i>World!</i>"
    }
}
    \endqml
    \o \image declarative-textformat.png
    \endtable
*/

QmlGraphicsText::TextFormat QmlGraphicsText::textFormat() const
{
    Q_D(const QmlGraphicsText);
    return d->format;
}

void QmlGraphicsText::setTextFormat(TextFormat format)
{
    Q_D(QmlGraphicsText);
    if (format == d->format)
        return;
    d->format = format;
    bool wasRich = d->richText;
    d->richText = format == RichText || (format == AutoText && Qt::mightBeRichText(d->text));

    if (wasRich && !d->richText) {
        //### delete control? (and vice-versa below)
        d->updateLayout();
        d->markImgDirty();
    } else if (!wasRich && d->richText) {
        if (!d->doc) {
            d->doc = new QTextDocument(this);
            d->doc->setDocumentMargin(0);
        }
        d->doc->setHtml(d->text);
        d->updateLayout();
        d->markImgDirty();
    }

    emit textFormatChanged(d->format);
}

/*!
    \qmlproperty enumeration Text::elide

    Set this property to elide parts of the text fit to the Text item's width.
    The text will only elide if an explicit width has been set.

    This property cannot be used with wrap enabled or with rich text.

    Eliding can be \c ElideNone (the default), \c ElideLeft, \c ElideMiddle, or \c ElideRight.

    If the text is a multi-length string, and the mode is not \c ElideNone,
    the first string that fits will be used, otherwise the last will be elided.

    Multi-length strings are ordered from longest to shortest, separated by the
    Unicode "String Terminator" character \c U009C (write this in QML with \c{"\u009C"} or \c{"\x9C"}).
*/
QmlGraphicsText::TextElideMode QmlGraphicsText::elideMode() const
{
    Q_D(const QmlGraphicsText);
    return d->elideMode;
}

void QmlGraphicsText::setElideMode(QmlGraphicsText::TextElideMode mode)
{
    Q_D(QmlGraphicsText);
    if (mode == d->elideMode)
        return;

    d->elideMode = mode;

    d->updateLayout();
    d->markImgDirty();
    emit elideModeChanged(d->elideMode);
}

void QmlGraphicsText::geometryChanged(const QRectF &newGeometry,
                              const QRectF &oldGeometry)
{
    Q_D(QmlGraphicsText);
    if (newGeometry.width() != oldGeometry.width()) {
        if (d->wrap || d->elideMode != QmlGraphicsText::ElideNone) {
            //re-elide if needed
            if (d->singleline && d->elideMode != QmlGraphicsText::ElideNone &&
                isComponentComplete() && widthValid()) {

                QFontMetrics fm(d->font);
                QString tmp = fm.elidedText(d->text,(Qt::TextElideMode)d->elideMode,width()); // XXX still worth layout...?
                d->layout.setText(tmp);
            }

            d->imgDirty = true;
            d->updateSize();
        }
    }
    QmlGraphicsItem::geometryChanged(newGeometry, oldGeometry);
}

void QmlGraphicsTextPrivate::updateLayout()
{
    Q_Q(QmlGraphicsText);
    if (q->isComponentComplete()) {
        //setup instance of QTextLayout for all cases other than richtext
        if (!richText) {
            layout.clearLayout();
            layout.setFont(font);
            if (format != QmlGraphicsText::StyledText) {
                QString tmp = text;
                tmp.replace(QLatin1Char('\n'), QChar::LineSeparator);
                singleline = !tmp.contains(QChar::LineSeparator);
                if (singleline && elideMode != QmlGraphicsText::ElideNone && q->widthValid()) {
                    QFontMetrics fm(font);
                    tmp = fm.elidedText(tmp,(Qt::TextElideMode)elideMode,q->width()); // XXX still worth layout...?
                }
                layout.setText(tmp);
            } else {
                singleline = false;
                QmlStyledText::parse(text, layout);
            }
        }
        updateSize();
    } else {
        dirty = true;
    }
}

void QmlGraphicsTextPrivate::updateSize()
{
    Q_Q(QmlGraphicsText);
    if (q->isComponentComplete()) {
        QFontMetrics fm(font);
        if (text.isEmpty()) {
            q->setImplicitHeight(fm.height());
            return;
        }

        int dy = q->height();
        QSize size(0, 0);

        //setup instance of QTextLayout for all cases other than richtext
        if (!richText) {
            size = setupTextLayout(&layout);
            cachedLayoutSize = size;
            dy -= size.height();
        } else {
            singleline = false; // richtext can't elide or be optimized for single-line case
            doc->setDefaultFont(font);
            QTextOption option((Qt::Alignment)int(hAlign | vAlign));
            if (wrap)
                option.setWrapMode(QTextOption::WordWrap);
            else
                option.setWrapMode(QTextOption::NoWrap);
            doc->setDefaultTextOption(option);
            if (wrap && !q->heightValid() && q->widthValid())
                doc->setTextWidth(q->width());
            else
                doc->setTextWidth(doc->idealWidth()); // ### Text does not align if width is not set (QTextDoc bug)
            dy -= (int)doc->size().height();
            cachedLayoutSize = doc->size().toSize();
        }
        int yoff = 0;

        if (q->heightValid()) {
            if (vAlign == QmlGraphicsText::AlignBottom)
                yoff = dy;
            else if (vAlign == QmlGraphicsText::AlignVCenter)
                yoff = dy/2;
        }
        q->setBaselineOffset(fm.ascent() + yoff);

        //### need to comfirm cost of always setting these for richText
        q->setImplicitWidth(richText ? (int)doc->idealWidth() : size.width());
        q->setImplicitHeight(richText ? (int)doc->size().height() : size.height());
    } else {
        dirty = true;
    }
}

// ### text layout handling should be profiled and optimized as needed
// what about QStackTextEngine engine(tmp, d->font.font()); QTextLayout textLayout(&engine);

void QmlGraphicsTextPrivate::drawOutline()
{
    QPixmap img = QPixmap(imgStyleCache.width()+2,imgStyleCache.height()+2);
    img.fill(Qt::transparent);

    QPainter ppm(&img);

    QPoint pos(imgCache.rect().topLeft());
    pos += QPoint(-1, 0);
    ppm.drawPixmap(pos, imgStyleCache);
    pos += QPoint(2, 0);
    ppm.drawPixmap(pos, imgStyleCache);
    pos += QPoint(-1, -1);
    ppm.drawPixmap(pos, imgStyleCache);
    pos += QPoint(0, 2);
    ppm.drawPixmap(pos, imgStyleCache);

    pos += QPoint(0, -1);
    ppm.drawPixmap(pos, imgCache);
    ppm.end();

    imgCache = img;
}

void QmlGraphicsTextPrivate::drawOutline(int yOffset)
{
    QPixmap img = QPixmap(imgStyleCache.width()+2,imgStyleCache.height()+2);
    img.fill(Qt::transparent);

    QPainter ppm(&img);

    QPoint pos(imgCache.rect().topLeft());
    pos += QPoint(0, yOffset);
    ppm.drawPixmap(pos, imgStyleCache);

    pos += QPoint(0, -yOffset);
    ppm.drawPixmap(pos, imgCache);
    ppm.end();

    imgCache = img;
}

QSize QmlGraphicsTextPrivate::setupTextLayout(QTextLayout *layout)
{
    Q_Q(QmlGraphicsText);
    layout->setCacheEnabled(true);

    int height = 0;
    qreal widthUsed = 0;
    qreal lineWidth = 0;

    //set manual width
    if ((wrap || elideMode != QmlGraphicsText::ElideNone) && q->widthValid())
        lineWidth = q->width();

    layout->beginLayout();

    while (1) {
        QTextLine line = layout->createLine();
        if (!line.isValid())
            break;

        if ((wrap || elideMode != QmlGraphicsText::ElideNone) && q->widthValid())
            line.setLineWidth(lineWidth);
    }
    layout->endLayout();

    int x = 0;
    for (int i = 0; i < layout->lineCount(); ++i) {
        QTextLine line = layout->lineAt(i);
        widthUsed = qMax(widthUsed, line.naturalTextWidth());
        line.setPosition(QPointF(0, height));
        height += int(line.height());

        if (!cache) {
            if (hAlign == QmlGraphicsText::AlignLeft) {
                x = 0;
            } else if (hAlign == QmlGraphicsText::AlignRight) {
                x = q->width() - (int)line.naturalTextWidth();
            } else if (hAlign == QmlGraphicsText::AlignHCenter) {
                x = (q->width() - (int)line.naturalTextWidth()) / 2;
            }
            line.setPosition(QPoint(x, (int)line.y()));
        }
    }

    return QSize(qCeil(widthUsed), height);
}

QPixmap QmlGraphicsTextPrivate::wrappedTextImage(bool drawStyle)
{
    //do layout
    QSize size = cachedLayoutSize;

    int x = 0;
    for (int i = 0; i < layout.lineCount(); ++i) {
        QTextLine line = layout.lineAt(i);
        if (hAlign == QmlGraphicsText::AlignLeft) {
            x = 0;
        } else if (hAlign == QmlGraphicsText::AlignRight) {
            x = size.width() - (int)line.naturalTextWidth();
        } else if (hAlign == QmlGraphicsText::AlignHCenter) {
            x = (size.width() - (int)line.naturalTextWidth()) / 2;
        }
        line.setPosition(QPoint(x, (int)line.y()));
    }

    //paint text
    QPixmap img(size);
    if (!size.isEmpty()) {
        img.fill(Qt::transparent);
        QPainter p(&img);
        drawWrappedText(&p, QPointF(0,0), drawStyle);
    }
    return img;
}

void QmlGraphicsTextPrivate::drawWrappedText(QPainter *p, const QPointF &pos, bool drawStyle)
{
    if (drawStyle)
        p->setPen(styleColor);
    else
        p->setPen(color);
    p->setFont(font);
    layout.draw(p, pos);
}

QPixmap QmlGraphicsTextPrivate::richTextImage(bool drawStyle)
{
    QSize size = doc->size().toSize();

    //paint text
    QPixmap img(size);
    img.fill(Qt::transparent);
    QPainter p(&img);

    QAbstractTextDocumentLayout::PaintContext context;

    if (drawStyle) {
        context.palette.setColor(QPalette::Text, styleColor);
        // ### Do we really want this?
        QTextOption colorOption;
        colorOption.setFlags(QTextOption::SuppressColors);
        doc->setDefaultTextOption(colorOption);
    } else {
        context.palette.setColor(QPalette::Text, color);
    }
    doc->documentLayout()->draw(&p, context);
    if (drawStyle)
        doc->setDefaultTextOption(QTextOption());
    return img;
}

void QmlGraphicsTextPrivate::checkImgCache()
{
    if (!imgDirty)
        return;

    bool empty = text.isEmpty();
    if (empty) {
        imgCache = QPixmap();
        imgStyleCache = QPixmap();
    } else if (richText) {
        imgCache = richTextImage(false);
        if (style != QmlGraphicsText::Normal)
            imgStyleCache = richTextImage(true); //### should use styleColor
    } else {
        imgCache = wrappedTextImage(false);
        if (style != QmlGraphicsText::Normal)
            imgStyleCache = wrappedTextImage(true); //### should use styleColor
    }
    if (!empty)
        switch (style) {
        case QmlGraphicsText::Outline:
            drawOutline();
            break;
        case QmlGraphicsText::Sunken:
            drawOutline(-1);
            break;
        case QmlGraphicsText::Raised:
            drawOutline(1);
            break;
        default:
            break;
        }

    imgDirty = false;
}

void QmlGraphicsText::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    Q_D(QmlGraphicsText);

    if (d->cache || d->style != Normal) {
        d->checkImgCache();
        if (d->imgCache.isNull())
            return;

        bool oldAA = p->testRenderHint(QPainter::Antialiasing);
        bool oldSmooth = p->testRenderHint(QPainter::SmoothPixmapTransform);
        if (d->smooth)
            p->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, d->smooth);

        int w = width();
        int h = height();

        int x = 0;
        int y = 0;

        switch (d->hAlign) {
        case AlignLeft:
            x = 0;
            break;
        case AlignRight:
            x = w - d->imgCache.width();
            break;
        case AlignHCenter:
            x = (w - d->imgCache.width()) / 2;
            break;
        }

        switch (d->vAlign) {
        case AlignTop:
            y = 0;
            break;
        case AlignBottom:
            y = h - d->imgCache.height();
            break;
        case AlignVCenter:
            y = (h - d->imgCache.height()) / 2;
            break;
        }

        bool needClip = !clip() && (d->imgCache.width() > width() ||
                                    d->imgCache.height() > height());

        if (needClip) {
            p->save();
            p->setClipRect(boundingRect(), Qt::IntersectClip);
        }
        p->drawPixmap(x, y, d->imgCache);
        if (needClip)
            p->restore();

        if (d->smooth) {
            p->setRenderHint(QPainter::Antialiasing, oldAA);
            p->setRenderHint(QPainter::SmoothPixmapTransform, oldSmooth);
        }
    } else {
        int h = height();
        int y = 0;

        switch (d->vAlign) {
        case AlignTop:
            y = 0;
            break;
        case AlignBottom:
            y = h - d->cachedLayoutSize.height();
            break;
        case AlignVCenter:
            y = (h - d->cachedLayoutSize.height()) / 2;
            break;
        }
        bool needClip = !clip() && (d->cachedLayoutSize.width() > width() ||
                                    d->cachedLayoutSize.height() > height());

        if (needClip) {
            p->save();
            p->setClipRect(boundingRect(), Qt::IntersectClip);
        }
        if (d->richText) {
            QAbstractTextDocumentLayout::PaintContext context;
            context.palette.setColor(QPalette::Text, d->color);
            p->translate(0, y);
            d->doc->documentLayout()->draw(p, context);
            p->translate(0, -y);
        } else {
            d->drawWrappedText(p, QPointF(0,y), false);
        }
        if (needClip)
            p->restore();
    }
}

/*!
    \qmlproperty bool Text::smooth

    Set this property if you want the text to be smoothly scaled or
    transformed.  Smooth filtering gives better visual quality, but is slower.  If
    the item is displayed at its natural size, this property has no visual or
    performance effect.

    \note Generally scaling artifacts are only visible if the item is stationary on
    the screen.  A common pattern when animating an item is to disable smooth
    filtering at the beginning of the animation and reenable it at the conclusion.
*/

void QmlGraphicsText::componentComplete()
{
    Q_D(QmlGraphicsText);
#ifdef Q_ENABLE_PERFORMANCE_LOG
    QmlPerfTimer<QmlPerf::TextComponentComplete> cc;
#endif
    QmlGraphicsItem::componentComplete();
    if (d->dirty) {
        d->updateLayout();
        d->dirty = false;
    }
}

/*!
  \overload
  Handles the given mouse \a event.
 */
void QmlGraphicsText::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(QmlGraphicsText);

    if (!d->richText || !d->doc || d->doc->documentLayout()->anchorAt(event->pos()).isEmpty()) {
        event->setAccepted(false);
        d->activeLink = QString();
    } else {
        d->activeLink = d->doc->documentLayout()->anchorAt(event->pos());
    }

    // ### may malfunction if two of the same links are clicked & dragged onto each other)

    if (!event->isAccepted())
        QmlGraphicsItem::mousePressEvent(event);

}

/*!
    \qmlsignal Text::linkActivated(link)

    This handler is called when the user clicks on a link embedded in the text.
*/

/*!
  \overload
  Handles the given mouse \a event.
 */
void QmlGraphicsText::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(QmlGraphicsText);

        // ### confirm the link, and send a signal out
    if (d->richText && d->doc && d->activeLink == d->doc->documentLayout()->anchorAt(event->pos()))
        emit linkActivated(d->activeLink);
    else
        event->setAccepted(false);

    if (!event->isAccepted())
        QmlGraphicsItem::mouseReleaseEvent(event);
}
QT_END_NAMESPACE