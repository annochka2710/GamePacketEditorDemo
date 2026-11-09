#ifndef PACKETITEM_H
#define PACKETITEM_H

#include <QVariant>
#include <QVector>

class PacketItem
{
public:
    explicit PacketItem(const QVector<QVariant> &data, PacketItem *parent = nullptr);
    ~PacketItem();

    PacketItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);
    PacketItem *parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    bool setData(int column, const QVariant &value);

private:
    QVector<PacketItem*> childItems;
    QVector<QVariant> itemData;
    PacketItem *parentItem;
};

#endif // PACKETITEM_H
