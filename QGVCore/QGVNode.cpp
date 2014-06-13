/***************************************************************
QGVCore
Copyright (c) 2014, Bergont Nicolas, All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.
***************************************************************/
#include <QGVNode.h>
#include <QGVCore.h>
#include <QGVScene.h>
#include <QGVGraphPrivate.h>
#include <QGVNodePrivate.h>
#include <QDebug>
#include <QPainter>

#include <stdlib.h>

QGVNode::QGVNode(QGVNodePrivate *node, QGVScene *scene): _node(node), _scene(scene)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

QGVNode::~QGVNode()
{
    _scene->removeItem(this);
		delete _node;
}

QString QGVNode::label() const
{
    return getAttribute("label");
}

void QGVNode::setLabel(const QString &label)
{
    setAttribute("label", label);
}

QRectF QGVNode::boundingRect() const
{
    return _path.boundingRect().united(_shadowPath.boundingRect());
}

void QGVNode::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);

    // Shadow
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(7, 7, 7));
    painter->drawPath(_shadowPath);

    // Fill
    if(isSelected()) {
        QBrush tbrush(_brush);
        tbrush.setColor(tbrush.color().darker(160));
        painter->setBrush(tbrush);
        _pen.setColor(QGVCore::toColor(getAttribute("selectedcolor")));
    } else {
        painter->setBrush(_brush);
        _pen.setColor(QGVCore::toColor(getAttribute("color")));
    }
    painter->setPen(_pen);
    painter->drawPath(_path);

    // Text/Icon
    if(isSelected()) {
        painter->setPen(QGVCore::toColor(getAttribute("selectedlabelfontcolor")));
    } else {
        painter->setPen(QGVCore::toColor(getAttribute("labelfontcolor")));
    }
    const QString &fontAttr = getAttribute("labelfont");
    QFont font = painter->font();
    if (fontAttr != "") {
        font = QFont(font);
    }
    const QString &fontWeightAttr = getAttribute("labelfontweight");
    if (fontWeightAttr == "bold") {
        font.setBold(true);
    }
    painter->setFont(font);
    const QRectF rect = boundingRect().adjusted(2,2,-2,-2); //Margin
    if(_icon.isNull()) {
        painter->drawText(rect, Qt::AlignCenter , QGVNode::label());
    } else {
        painter->drawText(rect.adjusted(0,0,0, -rect.height()*2/3), Qt::AlignCenter , QGVNode::label());

        const QRectF img_rect = rect.adjusted(0, rect.height()/3,0, 0);
        QImage img = _icon.scaled(img_rect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter->drawImage(img_rect.topLeft() + QPointF((img_rect.width() - img.rect().width())/2, 0), img);
    }

    painter->restore();
}

void QGVNode::setAttribute(const QString &name, const QString &value)
{
		agsafeset(_node->node(), name.toLocal8Bit().data(), value.toLocal8Bit().data(), "");
}

QString QGVNode::getAttribute(const QString &name) const
{
		char* value = agget(_node->node(), name.toLocal8Bit().data());
    if(value)
        return value;
    return QString();
}

void QGVNode::setIcon(const QImage &icon)
{
    _icon = icon;
}

void QGVNode::updateLayout()
{
    prepareGeometryChange();
		qreal width = ND_width(_node->node())*DotDefaultDPI;
		qreal height = ND_height(_node->node())*DotDefaultDPI;

    //Node Position (center)
		qreal gheight = QGVCore::graphHeight(_scene->_graph->graph());
		setPos(QGVCore::centerToOrigin(QGVCore::toPoint(ND_coord(_node->node()), gheight), width, height));

    //Node on top
    setZValue(1);

    //Node path
    _path = QGVCore::toPath(ND_shape(_node->node())->name, (polygon_t*)ND_shape_info(_node->node()), width, height);
    _shadowPath = _path;
    int shadowOffset = atoi(getAttribute("shadowoffset").toStdString().c_str());
    _shadowPath.translate(shadowOffset, shadowOffset);
    _pen.setWidth(1);

    _brush.setStyle(QGVCore::toBrushStyle(getAttribute("style")));
    _brush.setColor(QGVCore::toColor(getAttribute("fillcolor")));

    setToolTip(getAttribute("tooltip"));
}
