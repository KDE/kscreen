/*
    Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef QMLOUTPUT_H
#define QMLOUTPUT_H

#include <QQuickItem>
#include <kscreen/mode.h>

class QStandardItemModel;
class QAbstractItemModel;

class ModesProxyModel;
class QMLScreen;


class QMLOutput : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(KScreen::Output* output
               READ output
               NOTIFY outputChanged)

    Q_PROPERTY(KScreen::OutputPtr outputPtr
               READ outputPtr
               WRITE setOutputPtr
               NOTIFY outputChanged)

    Q_PROPERTY(bool isCloneMode
               READ isCloneMode
               WRITE setIsCloneMode
               NOTIFY isCloneModeChanged)

    Q_PROPERTY(QMLScreen* screen
               READ screen
               WRITE setScreen
               NOTIFY screenChanged)

    Q_PROPERTY(QMLOutput* cloneOf
               READ cloneOf
               WRITE setCloneOf
               NOTIFY cloneOfChanged)

    Q_PROPERTY(QMLOutput* leftDockedTo
               READ leftDockedTo
               WRITE setLeftDockedTo
               RESET undockLeft
               NOTIFY leftDockedToChanged)

    Q_PROPERTY(QMLOutput* topDockedTo
               READ topDockedTo
               WRITE setTopDockedTo
               RESET undockTop
               NOTIFY topDockedToChanged)

    Q_PROPERTY(QMLOutput* rightDockedTo
               READ rightDockedTo
               WRITE setRightDockedTo
               RESET undockRight
               NOTIFY rightDockedToChanged)

    Q_PROPERTY(QMLOutput* bottomDockedTo
               READ bottomDockedTo
               WRITE setBottomDockedTo
               RESET undockBottom
               NOTIFY bottomDockedToChanged)

    Q_PROPERTY(int currentOutputHeight
               READ currentOutputHeight
               NOTIFY currentOutputSizeChanged)

    Q_PROPERTY(int currentOutputWidth
               READ currentOutputWidth
               NOTIFY currentOutputSizeChanged)

    /* Workaround for possible QML bug when calling output.pos.y = VALUE works,
     * but output.pos.x = VALUE has no effect */
    Q_PROPERTY(int outputX
               READ outputX
               WRITE setOutputX
               NOTIFY outputXChanged)

    Q_PROPERTY(int outputY
               READ outputY
               WRITE setOutputY
               NOTIFY outputYChanged)

    Q_PROPERTY(QQmlListProperty<KScreen::Mode> modes READ modes NOTIFY modesChanged)


  public:
    enum {
      ModeRole = Qt::UserRole,
      ModeIdRole,
      SizeRole,
      RefreshRateRole
    };

    explicit QMLOutput(QQuickItem *parent = 0);
    virtual ~QMLOutput();

    KScreen::Output* output() const; // For QML

    KScreen::OutputPtr outputPtr() const;
    void setOutputPtr(const KScreen::OutputPtr &output);

    QMLScreen* screen() const;
    void setScreen(QMLScreen *screen);

    QMLOutput* leftDockedTo() const;
    void setLeftDockedTo(QMLOutput *output);
    void undockLeft();

    QMLOutput* topDockedTo() const;
    void setTopDockedTo(QMLOutput *output);
    void undockTop();

    QMLOutput* rightDockedTo() const;
    void setRightDockedTo(QMLOutput *output);
    void undockRight();

    QMLOutput* bottomDockedTo() const;
    void setBottomDockedTo(QMLOutput *output);
    void undockBottom();

    Q_INVOKABLE bool collidesWithOutput(QObject *other);
    Q_INVOKABLE bool maybeSnapTo(QMLOutput *other);

    void setCloneOf(QMLOutput *other);
    QMLOutput* cloneOf() const;

    int currentOutputHeight() const;
    int currentOutputWidth() const;

    int outputX() const;
    void setOutputX(int x);

    int outputY() const;
    void setOutputY(int y);

    void setIsCloneMode(bool isCloneMode);
    bool isCloneMode() const;

    void dockToNeighbours();

    QQmlListProperty<KScreen::Mode> modes();

public Q_SLOTS:
    void updateRootProperties();

  Q_SIGNALS:
    void changed();

    void moved(const QString &self);

    /* Property notifications */
    void outputChanged();
    void screenChanged();
    void cloneOfChanged();
    void currentOutputSizeChanged();

    void leftDockedToChanged();
    void topDockedToChanged();
    void rightDockedToChanged();
    void bottomDockedToChanged();

    void outputYChanged();
    void outputXChanged();

    void isCloneModeChanged();

    void modesChanged();

  private Q_SLOTS:
    void moved();
    void currentModeIdChanged();

  private:
    /**
     * Returns the biggest resolution available assuming it's the preferred one
     */
    KScreen::ModePtr bestMode() const;

    KScreen::OutputPtr m_output;
    QMLScreen *m_screen;

    QMLOutput *m_cloneOf;
    QMLOutput *m_leftDock;
    QMLOutput *m_topDock;
    QMLOutput *m_rightDock;
    QMLOutput *m_bottomDock;

    bool m_isCloneMode;
};

#endif // QMLOUTPUT_H

