#include "roomstatuspage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QScroller>
#include <QPainter>
#include <QLabel>
#include <QStyleOption>
#include <QDebug>
#include <QSqlDatabase> // 确保包含

RoomStatusPage::RoomStatusPage(QWidget *parent) : QWidget(parent)
{
    this->setObjectName("roomPage");

    // 1. 全局深色主题 (深海蓝渐变)
    this->setStyleSheet(
                "#roomPage { "
                "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0f2027, stop:0.5 #203a43, stop:1 #2c5364); "
                "}"
                "QWidget { font-family: 'Microsoft YaHei', 'Segoe UI'; font-size: 14px; }"
                "QSplitter::handle { background-color: rgba(255,255,255,0.1); height: 2px; }" // 分割线样式
                );

    setupUi();

    // 初始化数据
    loadRoomCards();
    onSearchQuery();
}

RoomStatusPage::~RoomStatusPage() {}

void RoomStatusPage::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void RoomStatusPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 1. 创建垂直分割器 (上下布局)
    mainSplitter = new QSplitter(Qt::Vertical, this);
    mainSplitter->setHandleWidth(2); // 细致的分割线

    // 2. 初始化面板
    setupTopPanel();    // 原左侧 -> 现顶部
    setupBottomPanel(); // 原右侧 -> 现底部

    // 3. 添加到分割器
    mainSplitter->addWidget(leftContainer);  // top
    mainSplitter->addWidget(rightContainer); // bottom

    // 4. 设置比例 (上 50%, 下 50% - 稍微调整以便表格显示更多)
    mainSplitter->setStretchFactor(0, 75);
    mainSplitter->setStretchFactor(1, 25);

    mainLayout->addWidget(mainSplitter);
}

// 顶部：房态可视化
void RoomStatusPage::setupTopPanel()
{
    leftContainer = new QWidget();
    leftContainer->setStyleSheet("background: transparent;"); // 透明以显示主窗口渐变

    QVBoxLayout *layout = new QVBoxLayout(leftContainer);
    layout->setContentsMargins(20, 20, 20, 10);
    layout->setSpacing(10);

    // --- 顶部导航栏 ---
    QHBoxLayout *topBar = new QHBoxLayout();

    QPushButton *btnBack = new QPushButton("← 返回", this);
    btnBack->setFixedSize(90, 32);
    btnBack->setCursor(Qt::PointingHandCursor);
    btnBack->setStyleSheet(
                "QPushButton { background: rgba(255,255,255,0.1); color: #ecf0f1; border: 1px solid rgba(255,255,255,0.2); border-radius: 16px; }"
                "QPushButton:hover { background: rgba(255,255,255,0.2); border-color: white; }"
                );
    connect(btnBack, &QPushButton::clicked, this, &RoomStatusPage::backClicked);

    QLabel *title = new QLabel("房态全景视图", this);
    title->setStyleSheet("color: #ecf0f1; font-size: 20px; font-weight: bold;");

    QFont font = title->font();
    font.setLetterSpacing(QFont::AbsoluteSpacing, 5.0); // 设置字间距为 5 像素
    title->setFont(font);

    QLabel *legend = new QLabel(this);
    legend->setText(
                "<span style='color:#2ecc71'>■</span> 空闲 &nbsp;&nbsp;"
                "<span style='color:#e74c3c'>■</span> 入住 &nbsp;&nbsp;"
                "<span style='color:#f39c12'>■</span> 维护"
                );
    legend->setStyleSheet("color: #bdc3c7; font-weight: bold;");

    topBar->addWidget(btnBack);
    topBar->addSpacing(20);
    topBar->addWidget(title);
    topBar->addStretch();
    topBar->addWidget(legend);

    // --- 滚动区域 ---
    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; }");

    gridContainer = new QWidget();
    gridContainer->setStyleSheet("background: transparent;");

    roomGridLayout = new QGridLayout(gridContainer);
    roomGridLayout->setSpacing(15);
    roomGridLayout->setContentsMargins(0, 10, 0, 10);
    roomGridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft); // 左上对齐

    scrollArea->setWidget(gridContainer);
    QScroller::grabGesture(scrollArea->viewport(), QScroller::LeftMouseButtonGesture);

    layout->addLayout(topBar);
    layout->addWidget(scrollArea);
}

void RoomStatusPage::loadRoomCards()
{
    // 清除旧组件
    QLayoutItem *child;
    while ((child = roomGridLayout->takeAt(0)) != nullptr) {
        if(child->widget()) delete child->widget();
        delete child;
    }

    QSqlQuery query("SELECT room_id, status, guest FROM rooms ORDER BY room_id ASC");

    int row = 0;
    int col = 0;
    // 【调整】横向布局可以放更多列
    int maxCols = 8;

    while(query.next()) {
        QString roomName = query.value(0).toString();
        int roomId = roomName.toInt();
        int status = query.value(1).toInt();
        QString guestName = query.value(2).toString();

        QPushButton *btn = new QPushButton();
        btn->setFixedSize(110, 80);
        btn->setCursor(Qt::PointingHandCursor);

        // 卡片基础样式
        QString baseStyle = "QPushButton { border-radius: 6px; border: none; color: white; font-weight: bold; }";
        QString colorStyle;
        QString text;

        if(status == 0) { // 空闲 - 绿色
            colorStyle = "QPushButton { background-color: rgba(46, 204, 113, 0.8); } QPushButton:hover { background-color: #2ecc71; }";
            text = roomName + "\n(空闲)";
        } else if (status == 1) { // 入住 - 红色
            colorStyle = "QPushButton { background-color: rgba(231, 76, 60, 0.8); } QPushButton:hover { background-color: #e74c3c; }";
            if(guestName.length() > 3) guestName = guestName.left(2) + "..";
            text = roomName + "\n" + (guestName.isEmpty() ? "已入住" : guestName);
        } else { // 维护 - 橙色
            colorStyle = "QPushButton { background-color: rgba(243, 156, 18, 0.8); } QPushButton:hover { background-color: #f39c12; }";
            text = roomName + "\n(维护)";
        }

        btn->setStyleSheet(baseStyle + colorStyle);
        btn->setText(text);

        connect(btn, &QPushButton::clicked, this, [=](){
            this->onRoomClicked(roomId);
        });

        roomGridLayout->addWidget(btn, row, col);

        col++;
        if(col >= maxCols) {
            col = 0;
            row++;
        }
    }
}

// ==========================================
// 底部：数据表格 (关键修改：颜色适配与中文显示)
// ==========================================
void RoomStatusPage::setupBottomPanel()
{
    rightContainer = new QWidget();
    // 底部背景：半透明黑
    rightContainer->setStyleSheet("background-color: rgba(0, 0, 0, 0.3); border-top: 1px solid rgba(255,255,255,0.1);");

    QVBoxLayout *layout = new QVBoxLayout(rightContainer);
    layout->setContentsMargins(20, 15, 20, 15);
    layout->setSpacing(10);

    // 1. 搜索控制栏
    QHBoxLayout *searchLayout = new QHBoxLayout();

    QLabel *lblSearch = new QLabel("信息检索:");
    lblSearch->setStyleSheet("color: #ecf0f1; font-weight: bold;");

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText(" 输入房号 / 姓名");
    searchEdit->setFixedWidth(200);
    searchEdit->setStyleSheet(
                "QLineEdit { background: rgba(0,0,0,0.4); border: 1px solid #546e7a; border-radius: 4px; color: white; padding: 5px; }"
                "QLineEdit:focus { border: 1px solid #3498db; }"
                );

    statusFilter = new QComboBox();
    statusFilter->addItems({"所有状态", "空闲", "入住", "维护"});
    statusFilter->setFixedWidth(120);
    statusFilter->setStyleSheet(
                "QComboBox { background: rgba(0,0,0,0.4); border: 1px solid #546e7a; border-radius: 4px; color: white; padding: 5px; }"
                "QComboBox QAbstractItemView { background: #2c3e50; color: white; selection-background-color: #3498db; }"
                );

    QPushButton *btnSearch = new QPushButton(" 查询");
    btnSearch->setCursor(Qt::PointingHandCursor);
    btnSearch->setStyleSheet("QPushButton { background-color: #3498db; color: white; border-radius: 4px; padding: 5px 15px; font-weight: bold; } QPushButton:hover { background-color: #2980b9; }");
    connect(btnSearch, &QPushButton::clicked, this, &RoomStatusPage::onSearchQuery);

    QPushButton *btnRefresh = new QPushButton(" 刷新");
    btnRefresh->setCursor(Qt::PointingHandCursor);
    btnRefresh->setStyleSheet("QPushButton { background-color: #27ae60; color: white; border-radius: 4px; padding: 5px 15px; font-weight: bold; } QPushButton:hover { background-color: #219150; }");
    connect(btnRefresh, &QPushButton::clicked, this, &RoomStatusPage::onRefreshRooms);

    searchLayout->addWidget(lblSearch);
    searchLayout->addWidget(searchEdit);
    searchLayout->addSpacing(10);
    searchLayout->addWidget(statusFilter);
    searchLayout->addSpacing(10);
    searchLayout->addWidget(btnSearch);
    searchLayout->addStretch();
    searchLayout->addWidget(btnRefresh);

    // 2. 表格配置
    infoTable = new QTableView();
    infoTable->setFrameShape(QFrame::NoFrame);
    infoTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    infoTable->verticalHeader()->setVisible(false);

    // 信息显示齐全：设置列宽自动铺满
    infoTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    infoTable->verticalHeader()->setDefaultSectionSize(30);

    // QSS 调整：
    infoTable->setStyleSheet(
                // 核心表格区
                "QTableView { "
                "   background-color: #f5f5f5; "       // 默认底色（未填充数据的部分）
                "   color: #333333; "                  // 【重点】字体改为深色，以便在彩色背景上阅读
                "   gridline-color: #dcdcdc; "         // 网格线
                "   selection-background-color: #2980b9; " // 选中时变为深蓝
                "   selection-color: white; "
                "   border: none;"
                "}"
                // 表头区 (保持酷炫的深色风格)
                "QHeaderView::section { "
                "   background-color: rgba(30, 40, 50, 0.9); " // 深色表头
                "   color: #ecf0f1; "                          // 表头白字
                "   padding: 8px; "
                "   border: none; "
                "   border-bottom: 2px solid #3498db; "
                "   font-weight: bold; "
                "}"
                "QTableCornerButton::section { background-color: rgba(30,40,50,0.9); border: none; }"
                );

    // 使用自定义模型 RoomStatusTableModel
    queryModel = new RoomStatusTableModel(this, QSqlDatabase::database());
    queryModel->setTable("rooms");
    queryModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    infoTable->setModel(queryModel);

    layout->addLayout(searchLayout);
    layout->addWidget(infoTable);
}

void RoomStatusPage::onSearchQuery()
{
    QString filter = "1=1";
    QString keyword = searchEdit->text().trimmed();
    if (!keyword.isEmpty()) {
        filter += QString(" AND (room_id LIKE '%%1%' OR guest LIKE '%%1%')").arg(keyword);
    }
    int idx = statusFilter->currentIndex();
    if (idx > 0) {
        filter += QString(" AND status = %1").arg(idx - 1);
    }

    queryModel->setFilter(filter);
    queryModel->setSort(0, Qt::AscendingOrder); // 按 ID 排序
    queryModel->select();

    queryModel->setHeaderData(0, Qt::Horizontal, "房间编号");
    queryModel->setHeaderData(1, Qt::Horizontal, "房间类型");
    queryModel->setHeaderData(2, Qt::Horizontal, "房间价格");
    queryModel->setHeaderData(4, Qt::Horizontal, "客户信息");
    queryModel->setHeaderData(5, Qt::Horizontal, "证件号码");

    int status = queryModel->fieldIndex("status");   // 替换为你数据库真实的字段名
    int checkout_time = queryModel->fieldIndex("checkout_time"); // 替换为你数据库真实的字段名
    int rfid_id = queryModel->fieldIndex("rfid_id");
    int checkin_time = queryModel->fieldIndex("checkin_time");

    if(status != -1)   infoTable->setColumnHidden(status, true);
    if(checkout_time != -1)  infoTable->setColumnHidden(checkout_time, true);
    if(rfid_id != -1) infoTable->setColumnHidden(rfid_id, true);
    if(checkin_time != -1) infoTable->setColumnHidden(checkin_time, true);
}

void RoomStatusPage::onRefreshRooms()
{
    loadRoomCards();
    onSearchQuery();
}

void RoomStatusPage::onRoomClicked(int roomId)
{
    searchEdit->setText(QString::number(roomId));
    statusFilter->setCurrentIndex(0);
    onSearchQuery();
}

void RoomStatusPage::refreshRooms()
{
    onRefreshRooms();
}
