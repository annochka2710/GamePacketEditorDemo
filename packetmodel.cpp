#include "packetmodel.h"
#include "packetitem.h"

#include <QtWidgets>

#include <QDebug>

PacketModel::PacketModel(const QStringList &headers, const QString &data, QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
    for(const QString &header : headers)
        rootData << header;

    rootItem = new PacketItem(rootData);
    setupModelData(data.split('\n'), rootItem);

    setData(index(0, 1), QString("%1").arg(packets::encodeStatePacket(index(0, 0))));
    setData(index(1, 1), QString("%1").arg(packets::encodeAppearancePacket(index(1, 0))));

    connect(this, &PacketModel::fieldChanged, this, &PacketModel::encodePacket);
}

PacketModel::~PacketModel()
{
    delete rootItem;
}

QVariant PacketModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    PacketItem *item = getItem(index);

    return item->data(index.column());
}

QVariant PacketModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex PacketModel::index(int row, int column, const QModelIndex &parent) const
{
    if(parent.isValid() && parent.column() != 0)
        return QModelIndex();

    PacketItem *parentItem = getItem(parent);
    if(!parentItem)
        return QModelIndex();

    PacketItem *childItem = parentItem->child(row);
    if(childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex PacketModel::parent(const QModelIndex &child) const
{
    if(!child.isValid())
        return QModelIndex();

    PacketItem *childItem = getItem(child);
    PacketItem *parentItem = childItem ? childItem->parent() : nullptr;

    if(parentItem == rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

int PacketModel::rowCount(const QModelIndex &parent) const
{
    const PacketItem *parentItem = getItem(parent);

    return parentItem ? parentItem->childCount() : 0;
}

int PacketModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return rootItem->columnCount();
}

Qt::ItemFlags PacketModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

bool PacketModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role != Qt::EditRole)
        return false;

    // Do not change any column but 1
    if(index.column() != 1)
        return false;

    PacketItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if(result && parent(index) != QModelIndex())
        emit fieldChanged(index);

    if(result)
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

    return result;
}

bool PacketModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if(role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    const bool result = rootItem->setData(section, value);

    if(result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

bool PacketModel::insertColumns(int position, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, position, position + count - 1);
    const bool success = rootItem->insertColumns(position, count);
    endInsertColumns();

    return success;
}

bool PacketModel::removeColumns(int position, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, position, position + count - 1);
    const bool success = rootItem->removeColumns(position, count);
    endRemoveColumns();

    if(rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool PacketModel::insertRows(int position, int count, const QModelIndex &parent)
{
    PacketItem *parentItem = getItem(parent);
    if(!parentItem)
        return false;

    beginInsertRows(parent, position, position + count - 1);
    const bool success = parentItem->insertChildren(position,
                                                    count,
                                                    rootItem->columnCount());
    endInsertRows();

    return success;
}

bool PacketModel::removeRows(int position, int count, const QModelIndex &parent)
{
    PacketItem *parentItem = getItem(parent);
    if(!parentItem)
        return false;

    beginRemoveRows(parent, position, position + count - 1);
    const bool success = parentItem->removeChildren(position, count);
    endRemoveRows();

    return success;
}

void PacketModel::setupModelData(const QStringList &list, PacketItem *parent)
{
    QVector<PacketItem*> parents;
    QVector<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while(number < list.count()) {
        qDebug() << "Line" << number << ":" << list[number];
        int position = 0;
        while(position < list[number].length()) {
            if(list[number].at(position) != ' ')
                break;
            ++position;
        }
        qDebug() << "Position:" << position;
        const QString lineData = list[number].mid(position).trimmed();
        qDebug() << "Line data:" << lineData;
        if(!lineData.isEmpty()) {
            // Read the column data
            const QStringList columnStrings = lineData.split('\t', Qt::SkipEmptyParts);
            QVector<QVariant> columnData;
            columnData.reserve(columnStrings.size());
            for(const QString &columnString : columnStrings) {
                qDebug() << "Column string:" << columnString;
                columnData << columnString;
            }
            if(position > indentations.last()) {
                if(parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount() - 1);
                    indentations << position;
                }
            } else {
                while(position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item
            PacketItem *parent = parents.last();
            parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
            for(int column = 0; column < columnData.size(); ++column) {
                qDebug() << "Set data:" << columnData[column];
                parent->child(parent->childCount() - 1)->setData(column, columnData[column]);
            }
        }
        ++number;
    }
}

PacketItem *PacketModel::getItem(const QModelIndex &index) const
{
    if(index.isValid()) {
        PacketItem *item = static_cast<PacketItem*>(index.internalPointer());
        if(item)
            return item;
    }
    return rootItem;
}

void PacketModel::encodePacket(const QModelIndex &index)
{
    QModelIndex i(index);
    QString s = packets::encode(i);
    qDebug() << "encode packet: " << s;
    int row = i.row();
    QModelIndex ii = this->index(row, 1);
    setData(ii, s);
    emit packetChanged(ii);
}

uint64_t packets::encodeStatePacket(const QModelIndex &packetIndex)
{
    bool ok;
    const QAbstractItemModel *model = packetIndex.model();
    QModelIndex packetValueIndex = model->index(0, 1);
    uint64_t data = packetValueIndex.data().toString().toULongLong(&ok);
    if(!ok) {
        return 0;
    }
    QModelIndex parametersIndex = model->index(0, 0, packetIndex);
    // Object type
    QModelIndex typeIndex = model->index(0, 1, parametersIndex);
    uint8_t objectType = model->data(typeIndex).toString().toUInt(&ok);
    if(ok) {
        const uint64_t mask = (~((uint64_t)-1 << 5)) << 0; // bits from 0 to 4
        data &= ~mask; // reset bits
        data |= objectType & mask; // write object type
    }
    // Mode
    QModelIndex modeIndex = model->index(1, 1, parametersIndex);
    uint8_t mode = model->data(modeIndex).toString().toUInt(&ok);
    if(ok) {
        const uint64_t mask = (~((uint64_t)-1 << 3)) << 5; // bits from 5 to 7
        data &= ~mask; // reset bits
        data |= (mode << 5) & mask; // write mode
    }
    // Character state
    QModelIndex stateIndex = model->index(2, 1, parametersIndex);
    uint8_t state = model->data(stateIndex).toString().toUInt(&ok);
    if(ok) {
        const uint64_t mask = (~((uint64_t)-1 << 3)) << 8; // bits from 8 to 10
        data &= ~mask; // reset bits
        data |= (state << 8) & mask; // write character state
    }
    // Character ID
    QModelIndex characterIdIndex = model->index(3, 1, parametersIndex);
    uint16_t characterId = model->data(characterIdIndex).toString().toUInt(&ok);
    if(ok) {
        const uint64_t mask = (~((uint64_t)-1 << 13)) << 11; // bits from 11 to 23
        data &= ~mask; // reset bits
        data |= (characterId << 11) & mask; // write character ID
    }
    return data;
}

uint64_t packets::encodeAppearancePacket(const QModelIndex &packetIndex)
{
    bool ok;
    const QAbstractItemModel *model = packetIndex.model();
    int row = packetIndex.row();
    QModelIndex packetValueIndex = model->index(row, 1);
    QString s = packetValueIndex.data().toString();
    qDebug() << "encodeAppearancePacket:" << s;
    uint64_t data = packetValueIndex.data().toString().toULongLong(&ok);
    if(!ok) {
        return 0;
    }
    // Object type
    QModelIndex typeIndex = model->index(0, 1, packetIndex);
    uint8_t objectType = model->data(typeIndex).toString().toUInt(&ok);
    if(ok) {
        const uint64_t mask = (~((uint64_t)-1 << 5)) << 0; // bits from 0 to 4
        data &= ~mask; // reset bits
        data |= objectType & mask; // write object type
    }
    // Character class
    QModelIndex classIndex = model->index(1, 1, packetIndex);
    uint8_t characterClass = model->data(classIndex).toString().toUInt(&ok);
    if(ok) {
        const uint64_t mask = (~((uint64_t)-1 << 3)) << 5; // bits from 5 to 7
        data &= ~mask; // reset bits
        data |= (characterClass << 5) & mask; // write character class
    }
    const uint64_t equipmentBits = 6;
    const int equipmentCount = 8;
    const uint64_t equipmentPositions[equipmentCount] = { 8, 14, 20, 26, 32, 38, 44, 50 };
    // Equipment codes
    for(int i = 0; i < equipmentCount; i++) {
        QModelIndex equipmentIndex = model->index(2 + i, 1, packetIndex);
        uint64_t equipmentCode = model->data(equipmentIndex).toString().toUInt(&ok);
        if(ok) {
            const uint64_t mask = (~((uint64_t)-1 << equipmentBits)) << equipmentPositions[i];
            data &= ~mask; // reset bits
            data |= (equipmentCode << equipmentPositions[i]) & mask; // write equipment code
        }
    }
    return data;
}

QString packets::encode(QModelIndex &index)
{
    QString ret = QString();
    QString s = index.data().toString();
    qDebug() << "encode:" << s;
    QModelIndex parent = index.model()->parent(index);
    if(parent != QModelIndex()) {
        index = parent;
        ret = encode(index);
    } else {
        QString s = index.data().toString();
        qDebug() << s;
        if(s == "CharacterState") {
            ret = QString("%1").arg(packets::encodeStatePacket(index));
        } else if(s == "CharacterAppearance") {
            uint64_t value = packets::encodeAppearancePacket(index);
            ret = QString("%1").arg(value);
            qDebug() << "New packet:" << ret;
        } else {
            qDebug() << "Nothing to encode";
        }
    }
    return ret;
}
