
#ifndef LOOP2
#define LOOP2

#include <QObject>
class Loop : public QObject
{
    Q_OBJECT
    public:
        explicit Loop(QObject* parent = 0);
        virtual ~Loop();

    public Q_SLOTS:
        void start();
        void printConfig();
};

#endif