#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "darkeners.h"


MainWindow::MainWindow(Game& game, Controller& controller, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , game_(game)
    , controller_(controller)
    , timer_for_40ms_{ new prac::QTimer(this) }
{
    ui->setupUi(this);
    controller_.SetRedrawCallback([this]() {
        this->repaint();
    });

    connect(timer_for_40ms_, &prac::QTimer::timeout, this, &MainWindow::on_timerTimeout);
    timer_for_40ms_->start(40);
}

MainWindow::~MainWindow() {
    delete ui;
    delete timer_for_40ms_;
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) {
        return QWidget::keyPressEvent(event);
    }

    switch (event->key()) {
    case Qt::Key_Right:
        controller_.OnMoveKey(Direction::kRight);
        break;
    case Qt::Key_Left:
        controller_.OnMoveKey(Direction::kLeft);
        break;
    case Qt::Key_Up:
        controller_.OnMoveKey(Direction::kUp);
        break;
    case Qt::Key_Down:
        controller_.OnMoveKey(Direction::kDown);
        break;
    default:
        QWidget::keyPressEvent(event); // call base class implementation
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) {
        return QWidget::keyReleaseEvent(event);
    }

    switch (event->key()) {
    case Qt::Key_Right:
        controller_.OnReleaseMoveKey(Direction::kRight);
        break;
    case Qt::Key_Left:
        controller_.OnReleaseMoveKey(Direction::kLeft);
        break;
    case Qt::Key_Up:
        controller_.OnReleaseMoveKey(Direction::kUp);
        break;
    case Qt::Key_Down:
        controller_.OnReleaseMoveKey(Direction::kDown);
        break;
    default:
        QWidget::keyPressEvent(event); // call base class implementation
    }
}

void MainWindow::paintEvent(QPaintEvent* /*event*/) {
    QPainter q_painter{ this };
    Painter painter{ q_painter };

    CoordinateF player_pos = game_.GetPlayer().GetExactPos();
    Direction player_dir = game_.GetPlayer().GetDirection();
    FlashlightDarkener flash_darkener{ player_pos, 7, player_dir, 4 };
    DrawContext draw_context{ .painter = painter, .darkener = flash_darkener };

    Size field_size = game_.GetField().GetRect();
    Size window_size{ width(), height() };

    painter.PreparePaint(player_pos, field_size, window_size);
    game_.DrawFrame(draw_context);
}

void MainWindow::on_timerTimeout() {
    controller_.Tick();
}

void MainWindow::DrawInventory(Painter& painter) {
    const auto& inventory = game_.GetInventory();
    const auto& items = inventory.GetItems();

    int offset = 0;
    for (const auto& item : items) {
        if (!item) {
            qWarning() << "Null item in inventory";
            continue;
        }

        try {            
            const InventoryKey* key = item->AsKey();
            if (key) {
                painter.DrawInventoryItem(offset, key->GetAsset());
                offset += 50;
            } else {
                qWarning() << "Non-key item in inventory";
            }
        } catch (...) {
            qCritical() << "Failed to draw inventory item";
        }
    }
}

