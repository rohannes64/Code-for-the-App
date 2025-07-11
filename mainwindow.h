#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QListWidget>
#include <QGraphicsView>
#include <QGraphicsScene>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ContactTracer {
public:
    void addContact(const QString& a, const QString& b);
    std::vector<QString> traceExposure(const QString& infected);
    std::vector<QString> traceExposureMultiple(const std::vector<QString>& infectedPeople);
    QStringList getAllPeople();
    const std::unordered_map<QString, std::unordered_set<QString>>& getGraph() const;

private:
    std::unordered_map<QString, std::unordered_set<QString>> graph;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void addContact();
    void traceExposure();
private:
    QLineEdit *nameA;
    QLineEdit *nameB;
    QLineEdit *infectedName;
    QListWidget *exposedList;
    QListWidget *allPeopleList;
    void updateAllPeopleList();
    QGraphicsView *graphView;
    QGraphicsScene *graphScene;
    void updateGraphView();

    ContactTracer tracer;
};
#endif // MAINWINDOW_H
