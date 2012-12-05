
#ifndef LOOP2
#define LOOP2

#include <QObject>

namespace KScreen
{
class Config;
}
class Loop : public QObject
{
    Q_OBJECT
    public:
        explicit Loop(QObject* parent = 0);
        virtual ~Loop();

    public Q_SLOTS:
        void start();
        void printConfig();

    private:
        KScreen::Config *m_config;
};

#endif