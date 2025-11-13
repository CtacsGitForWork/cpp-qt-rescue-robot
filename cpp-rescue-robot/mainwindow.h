#ifndef MAINWINDOW_H
#define MAINWINDOW_H



#include "controller.h"
#include "game.h"

#include <QMainWindow>
#include <QPainter>
#include <QKeyEvent>
#include <QPaintEvent>
#include <prac/QTimer>


QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Game& game, Controller& controller, QWidget* parent = nullptr);
    ~MainWindow();

public:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    void paintEvent(QPaintEvent* event) override;

    void DrawInventory(Painter& painter) ;
private slots:
    void on_timerTimeout();

private:
    Ui::MainWindow* ui;
    Game& game_;
    Controller& controller_;
    prac::QTimer* timer_for_40ms_;
};
#endif // MAINWINDOW_H
