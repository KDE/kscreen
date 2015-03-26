/*
 * Copyright 2015  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PRIMARYOUTPUTCOMBO_H
#define PRIMARYOUTPUTCOMBO_H

#include <QComboBox>

#include <KScreen/Types>

class PrimaryOutputCombo : public QComboBox
{
    Q_OBJECT
public:
    explicit PrimaryOutputCombo(QWidget *parent = 0);
    virtual ~PrimaryOutputCombo();

    void setConfig(const KScreen::ConfigPtr &config);
    KScreen::OutputPtr primaryOutput() const;

private Q_SLOTS:
    void addOutput(const KScreen::OutputPtr &output);
    void removeOutput(int outputId);
    void setPrimaryOutput(const KScreen::OutputPtr &output);
    void outputChanged(const KScreen::OutputPtr &output);

    void onCurrentIndexChanged(int currentIndex);

private:
    void addOutputItem(const KScreen::OutputPtr &output);
    void removeOutputItem(int outputId);

private:
    KScreen::ConfigPtr mConfig;
};

#endif