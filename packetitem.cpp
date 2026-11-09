/* packetitem.cpp
 *
 * A container of packet fields supplied by the simple tree model
 */

#include "packetitem.h"

PacketItem::PacketItem(const QVector<QVariant> &data, PacketItem *parent)
    : itemData(data)
    , parentItem(parent)
{

}

PacketItem::~PacketItem()
{
    qDeleteAll(childItems);
}

PacketItem *PacketItem::child(int number)
{
    if(number < 0 || number >= childItems.size())
        return nullptr;
    return childItems.at(number);
}

int PacketItem::childCount() const
{
    return childItems.count();
}

int PacketItem::columnCount() const
{
    return itemData.count();
}

QVariant PacketItem::data(int column) const
{
    if(column < 0 || column >= itemData.size())
        return QVariant();
    return itemData.at(column);
}

bool PacketItem::insertChildren(int position, int count, int columns)
{
    if(position < 0 || position > childItems.size())
        return false;

    for(int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        PacketItem *item = new PacketItem(data, this);
        childItems.insert(position, item);
    }
    return true;
}

bool PacketItem::insertColumns(int position, int columns)
{
    if(position < 0 || position > itemData.size() )
        return false;

    for(int column = 0; column < columns; ++column)
        itemData.insert(position, QVariant());

    for(PacketItem *child : qAsConst(childItems))
        child->insertColumns(position, columns);

    return true;
}

PacketItem *PacketItem::parent()
{
    return parentItem;
}

bool PacketItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;

}

bool PacketItem::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.remove(position);

    for (PacketItem *child : qAsConst(childItems))
        child->removeColumns(position, columns);

    return true;

}

int PacketItem::childNumber() const
{
    if(parentItem)
        return parentItem->childItems.indexOf(const_cast<PacketItem*>(this));
    return 0;
}

bool PacketItem::setData(int column, const QVariant &value)
{
    if(column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;
    return true;
}
