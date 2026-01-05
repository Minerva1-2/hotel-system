#ifndef ROOMSTATUSPAGE_H
#define ROOMSTATUSPAGE_H

#include <QWidget>
#include <QSplitter>
#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>
#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QSqlTableModel>
#include <QHeaderView>
#include <QColor>
#include <QBrush>

class RoomStatusTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit RoomStatusTableModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase())
        : QSqlTableModel(parent, db) {}

    // 重写 data 函数：这是实现“主题适配”和“颜色区分”的关键
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {

        // 1. 处理背景颜色 (房态主题适配)
        // 当渲染背景时，检查"状态"列，返回对应的颜色
        if (role == Qt::BackgroundRole) {
            // 假设数据库中状态列名为 "status"，如果不是请在cpp中修改索引获取方式
            // 这里为了通用，先获取当前行所有数据
            QModelIndex statusIndex = this->index(index.row(), fieldIndex("status"));
            QString status = statusIndex.data().toString();

            if (status == "入住" || status == "Occupied") {
                return QColor(255, 235, 238); // 浅红色 (入住 - 警示色)
            } else if (status == "空闲" || status == "Available") {
                return QColor(232, 245, 233); // 浅绿色 (空闲 - 安全色)
            } else if (status == "打扫" || status == "Cleaning") {
                return QColor(255, 248, 225); // 浅黄色 (打扫 - 警告色)
            } else if (status == "维修" || status == "Maintenance") {
                return QColor(245, 245, 245); // 灰色 (维修)
            }
        }

        // 2. 确保所有文字居中显示 (为了美观)
        if (role == Qt::TextAlignmentRole) {
            return Qt::AlignCenter;
        }

        return QSqlTableModel::data(index, role);
    }
};

class RoomStatusPage : public QWidget
{
    Q_OBJECT

public:
    explicit RoomStatusPage(QWidget *parent = nullptr);
    ~RoomStatusPage();
    void refreshRooms(); // 公共刷新接口

signals:
    void backClicked(); // 返回信号

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onRefreshRooms();
    void onSearchQuery();
    void onRoomClicked(int roomId);

private:
    void setupUi();

    // --- 【关键修改点】函数名已更新为 Top/Bottom ---
    void setupTopPanel();    // 原 setupLeftPanel
    void setupBottomPanel(); // 原 setupRightPanel

    void loadRoomCards();

    // UI 组件
    QSplitter *mainSplitter;

    // 注意：为了兼容刚才的 .cpp 代码，这里变量名我暂时没改
    // leftContainer 实际上现在是 Top 面板
    // rightContainer 实际上现在是 Bottom 面板
    QWidget *leftContainer;
    QWidget *rightContainer;

    QScrollArea *scrollArea;
    QWidget *gridContainer;
    QGridLayout *roomGridLayout;

    // 查询组件
    QLineEdit *searchEdit;
    QComboBox *statusFilter;
    QTableView *infoTable;
    QSqlTableModel *queryModel;
};

#endif // ROOMSTATUSPAGE_H
