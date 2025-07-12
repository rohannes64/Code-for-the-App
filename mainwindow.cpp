#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <unordered_set>
#include <queue>
#include <QGraphicsTextItem>
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();

    QLabel *label = new QLabel("Enter Contact (Person A, Person B):");
    QHBoxLayout *inputLayout = new QHBoxLayout();
    nameA = new QLineEdit();
    nameB = new QLineEdit();
    QPushButton *addBtn = new QPushButton("Add Contact");
    inputLayout->addWidget(nameA);
    inputLayout->addWidget(nameB);
    inputLayout->addWidget(addBtn);

    QLabel *traceLabel = new QLabel("Enter Infected Person:");
    infectedName = new QLineEdit();
    QPushButton *traceBtn = new QPushButton("Trace Exposure");

    exposedList = new QListWidget();

    layout->addWidget(label);
    layout->addLayout(inputLayout);
    layout->addWidget(traceLabel);
    layout->addWidget(infectedName);
    layout->addWidget(traceBtn);
    layout->addWidget(new QLabel("Exposed Individuals:"));
    layout->addWidget(exposedList);
    layout->addWidget(new QLabel("Listed Persons:"));
    allPeopleList = new QListWidget();
    layout->addWidget(allPeopleList);
    layout->addWidget(new QLabel("Contact Graph:"));
    graphScene = new QGraphicsScene(this);
    graphView = new QGraphicsView(graphScene);
    graphView->setMinimumHeight(300); // adjust height as needed
    layout->addWidget(graphView);


    connect(addBtn, &QPushButton::clicked, this, &MainWindow::addContact);
    connect(traceBtn, &QPushButton::clicked, this, &MainWindow::traceExposure);

    central->setLayout(layout);
    setCentralWidget(central);
    setWindowTitle("Dynamic Contact Tracer App");
    resize(700, 700);
}

MainWindow::~MainWindow() {}
void MainWindow::updateAllPeopleList() {
    allPeopleList->clear();
    QStringList people = tracer.getAllPeople();
    sort(people.begin(), people.end(), [](const QString &a, const QString &b) {
        return a.toLower() < b.toLower();
    });

    for (const auto& name : people) {
        allPeopleList->addItem(name);
    }
}

void MainWindow::updateGraphView() {
    graphScene->clear();

    const auto& graphData = tracer.getGraph(); // ← we’ll define this getter next
    const int radius = 120;
    const int nodeRadius = 30;
    const QPointF center(250, 150);
    int n = graphData.size();
    if (n == 0) return;

    vector<QString> names;
    for (const auto& p : graphData) names.push_back(p.first);

    unordered_map<QString, QPointF> positions;
    for (int i = 0; i < n; ++i) {
        double angle = (2 * M_PI * i) / n;
        double x = center.x() + radius * cos(angle);
        double y = center.y() + radius * sin(angle);
        positions[names[i]] = QPointF(x, y);

        graphScene->addEllipse(x - nodeRadius/2, y - nodeRadius/2, nodeRadius, nodeRadius, QPen(), QBrush(Qt::lightGray));
        QGraphicsTextItem *label = graphScene->addText(names[i]);
        label->setPos(x - label->boundingRect().width()/2, y - label->boundingRect().height()/2);
    }

    for (const auto& p : graphData) {
        for (const auto& neighbor : p.second) {
            if (p.first < neighbor) { // avoid double-drawing
                graphScene->addLine(QLineF(positions[p.first], positions[neighbor]), QPen(Qt::darkBlue));
            }
        }
    }
}

void MainWindow::addContact() {
    QString a = nameA->text().trimmed();
    QString b = nameB->text().trimmed();
    if (a.isEmpty() || b.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter both names.");
        return;
    }
    tracer.addContact(a, b);
    QMessageBox::information(this, "Success", "Contact added successfully!");
    nameA->clear();
    nameB->clear();
    updateAllPeopleList();
    updateGraphView();


}
void MainWindow::traceExposure() {
    exposedList->clear();
    QString input = infectedName->text().trimmed();
    if (input.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter at least one name.");
        return;
    }

    QStringList infectedList = input.split(',', Qt::SkipEmptyParts);
    vector<QString> infectedPeople;
    for (QString name : infectedList) {
        name = name.trimmed();
        if (!name.isEmpty()) {
            infectedPeople.push_back(name);
        }
    }

    if (infectedPeople.empty()) {
        QMessageBox::warning(this, "Error", "No valid names entered.");
        return;
    }

    auto exposed = tracer.traceExposureMultiple(infectedPeople);

    if (exposed.empty()) {
        QMessageBox::information(this, "No Exposure", "No contacts found for given names.");
    }

    for (const auto& person : exposed) {
        exposedList->addItem(person);
    }
}



void ContactTracer::addContact(const QString &a, const QString &b) {
    graph[a].insert(b);
    graph[b].insert(a);
}

QStringList ContactTracer::getAllPeople() {
    QStringList people;
    for (const auto& p : graph) {
        people << p.first;
    }
    return people;
}
const unordered_map<QString, unordered_set<QString>>& ContactTracer::getGraph() const {
    return graph;
}
vector<QString> ContactTracer::traceExposure(const QString &infected) {
    unordered_set<QString> visited;
    queue<QString> q;
    vector<QString> exposed;

    q.push(infected);
    visited.insert(infected);

    while (!q.empty()) {
        QString current = q.front(); q.pop();
        exposed.push_back(current);

        for (const auto& contact : graph[current]) {
            if (!visited.count(contact)) {
                visited.insert(contact);
                q.push(contact);
            }
        }
    }
    return exposed;
}
vector<QString> ContactTracer::traceExposureMultiple(const vector<QString>& infectedPeople) {
    unordered_set<QString> visited;
    queue<QString> q;
    vector<QString> exposed;

    for (const auto& person : infectedPeople) {
        if (graph.find(person) != graph.end()) {
            visited.insert(person);
            q.push(person);
        }
    }

    while (!q.empty()) {
        QString current = q.front(); q.pop();
        exposed.push_back(current);

        for (const auto& neighbor : graph[current]) {
            if (!visited.count(neighbor)) {
                visited.insert(neighbor);
                q.push(neighbor);
            }
        }
    }

    return exposed;
}


