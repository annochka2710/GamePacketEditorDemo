#ifndef PACKETMODEL_H
#define PACKETMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class PacketItem;

class PacketModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    PacketModel(const QStringList &headers, const QString &data,
                QObject *parent = nullptr);
    ~PacketModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
    bool insertColumns(int position, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int position, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertRows(int position, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int position, int count, const QModelIndex &parent = QModelIndex()) override;

private:
    void setupModelData(const QStringList &list, PacketItem *parent);
    PacketItem *getItem(const QModelIndex &index) const;

    PacketItem *rootItem;

private slots:
    void encodePacket(const QModelIndex &index);

signals:
    void fieldChanged(const QModelIndex &index);
    void packetChanged(const QModelIndex &index);
};

namespace packets {
    QString encode(QModelIndex &index);
    uint64_t encodeStatePacket(const QModelIndex &packetIndex);
    uint64_t encodeAppearancePacket(const QModelIndex &packetIndex);
}

#endif // PACKETMODEL_H
