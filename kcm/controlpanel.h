/*
    Copyright (C) 2012  Dan Vratil <dvratil@redhat.com>

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


#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>

class QMLOutput;
class QGridLayout;
class QCheckBox;
class QLabel;
class KComboBox;

class ModesProxyModel;
class ResolutionSortModel;

class ControlPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ControlPanel(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~ControlPanel();


public Q_SLOTS:
    void setOutput(QMLOutput *output);

private Q_SLOTS:
    void primaryChanged(bool);
    void enabledChanged(bool);
    void refreshRateChanged();
    void resolutionChanged();

private:
    QMLOutput *m_output;

    QGridLayout *m_layout;

    ModesProxyModel *m_modesProxyModel;
    ResolutionSortModel *m_resolutionSortModel;

    QLabel *m_outputName;
    QCheckBox *m_enabled;
    QCheckBox *m_primary;
    QLabel *m_resolutionsLabel;
    KComboBox *m_resolutions;
    QLabel *m_refreshRatesLabel;
    KComboBox *m_refreshRates;
};

#endif // CONTROLPANEL_H
