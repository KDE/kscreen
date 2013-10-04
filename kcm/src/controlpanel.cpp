/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "controlpanel.h"
#include "outputconfig.h"
#include "unifiedoutputconfig.h"

#include <QtGui/QVBoxLayout>

#include <kscreen/config.h>

ControlPanel::ControlPanel(KScreen::Config *config, QWidget *parent)
    : QScrollArea(parent)
    , mConfig(config)
    , mUnifiedOutputCfg(0)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);

    QWidget *widget = new QWidget(this);
    mLayout = new QVBoxLayout(widget);
    mLayout->setSizeConstraint(QLayout::QLayout::SetMinAndMaxSize);

    setWidget(widget);

    Q_FOREACH (KScreen::Output *output, mConfig->outputs()) {
        OutputConfig *outputCfg = new OutputConfig(output, widget);
        // Make sure laptop screen is always first - it somehow feels right :)
        if (output->type() == KScreen::Output::Panel) {
            mLayout->insertWidget(0, outputCfg);
        } else {
            mLayout->addWidget(outputCfg);
        }
        mOutputConfigs << outputCfg;
    }

    mLayout->addStretch(1);
}

ControlPanel::~ControlPanel()
{
}

void ControlPanel::activateOutput(KScreen::Output *output)
{
    Q_FOREACH (OutputConfig *cfg, mOutputConfigs) {
        if (cfg->output()->id() == output->id()) {
            cfg->expand();
        }
    }
}

void ControlPanel::setUnifiedOutput(KScreen::Output *output)
{
    Q_FOREACH (OutputConfig *config, mOutputConfigs) {
        if (!config->output()->isConnected()) {
            continue;
        }

        config->setVisible(output == 0);
    }

    if (output == 0) {
        mUnifiedOutputCfg->deleteLater();
    } else {
        mUnifiedOutputCfg = new UnifiedOutputConfig(mConfig, widget());
        mUnifiedOutputCfg->setOutput(output);
        mUnifiedOutputCfg->setVisible(true);
        mLayout->insertWidget(mLayout->count() - 2, mUnifiedOutputCfg);
    }
}

#include "controlpanel.moc"
