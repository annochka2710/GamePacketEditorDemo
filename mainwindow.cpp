#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "packetmodel.h"

#include <QFile>
#include <QTimer>

#include <QDebug>

constexpr int transparency = 127;

QColor changedColor = QColor(
            QColor(Qt::red).red(),
            QColor(Qt::red).green(),
            QColor(Qt::red).blue(),
            transparency);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , count(0)
{
    ui->setupUi(this);

    const QStringList headers({"Field", "Value", "First Bit", "Last Bit"});

    QFile file(":/packets.txt");
    file.open(QIODevice::ReadOnly);
    PacketModel *model = new PacketModel(headers, file.readAll());
    file.close();

    ui->treeView->setModel(model);
    for(int column = 0; column < model->columnCount(); ++column)
        ui->treeView->resizeColumnToContents(column);

    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setHorizontalHeaderLabels({"Packet", "Value"});

    const int rows = model->rowCount();
    ui->tableWidget->setRowCount(rows);

    for(int i = 0; i < rows; i++) {
        QModelIndex index = model->index(i, 0);
        QTableWidgetItem *item = new QTableWidgetItem(model->data(index).toString());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(i, 0, item);

        index = model->index(i, 1);
        item = new QTableWidgetItem(model->data(index).toString());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(i, 1, item);
    }

    QTimer *timer = new QTimer(this);
    timer->start(1000);

    connect(timer, &QTimer::timeout, this, &MainWindow::processTimer);
    connect(model, &PacketModel::packetChanged, this, &MainWindow::processPacket);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processTimer()
{
    for(int i = 0; i < ui->tableWidget->rowCount(); i++) {
        QString s;
        QTableWidgetItem *item = ui->tableWidget->item(i, 1);
        if(item && item->background() != Qt::white) {
            s.append(QString("%1: ").arg(++count));
            s.append(ui->tableWidget->item(i, 0)->text() + " ");
            s.append(item->text());
            item->setBackground(Qt::white);
        }
        if(s.count()) {
            ui->listWidget->insertItem(0, s);
        }
    }
}

void MainWindow::processPacket(const QModelIndex &index)
{
    int row = index.row();
    QString text = index.data().toString();
    qDebug() << "New:" << text;
    QTableWidgetItem *item = ui->tableWidget->item(row, 1);
    qDebug() << "Old:" << item->text();
    if(text != item->text()) {
        item->setText(text);
        item->setBackground(changedColor);
    }
}

