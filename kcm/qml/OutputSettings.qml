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

import QtQuick 1.1
import org.kde.qtextracomponents 0.1
import org.kde.plasma.components 0.1 as PlasmaComponents;
import org.kde.plasma.core 0.1 as PlasmaCore
import KScreen 1.0

FocusScope {
	id: root;

	property QMLOutput output;

	ListModel {
		id: resolutionModel;
	}

	ListModel {
		id: refreshRatesModel;
	}

	Row {
		id: titleRow;
		spacing: 20;


		IconButton {
			id: primaryButton;
			enabledIcon: "bookmarks";
			enabled: (output != null && output.output.primary);
			anchors.verticalCenter: parent.verticalCenter;

			onClicked: {
				if (output == null) {
					return;
				}

				output.output.primary = !output.output.primary;
			}
		}

		Text {
			id: outputName;
			text: "";
			color: palette.text;
			font {
				family: theme.defaultFont.family;
				pointSize: 18;
				bold: true;
			}

			anchors.verticalCenter: parent.verticalCenter;
		}

		PlasmaComponents.Switch {
			id: outputSwitch;
			checked: false;

			anchors.verticalCenter: parent.verticalCenter;

			onCheckedChanged: output.output.enabled = checked;
		}
	}


	Row {
		anchors {
			top: titleRow.bottom;
			bottom: parent.bottom;
			left: parent.left;
			topMargin: 20;
		}

		spacing: 20;

		ModeListView {
			id: resolutionListView;
			model: resolutionModel;
			width: 150;
			focus: true;

			anchors {
				top: parent.top;
				bottom: parent.bottom;
			}

			onCurrentItemChanged: {
				refreshRatesModel.clear();
				if (resolutionListView.currentItem == null) {
					return;
				}

				var res = resolutionListView.currentItem.modelData.label;
				var rates = output.getRefreshRatesForResolution(res);
				var currentMode = output.output.mode(output.output.currentMode);
				var currentRate = (currentMode == null) ? -1 : currentMode.refreshRate;
				refreshRatesModel.append({
					"label": i18n("Auto"),
					"rate": 0
				});
				for (var i = 0; i < rates.length; i++) {
					refreshRatesModel.append({
						"label" : Math.round(rates[i], 1) + " Hz",
						"rate": rates[i]
					});

					if (rates[i] == currentRate) {
						refreshRatesListView.currentIndex = i + 1;
					}
				}

				output.setMode(res, 0);
			}
		}

		ModeListView {
			id: refreshRatesListView;
			model: refreshRatesModel;

			width: 150;
			anchors {
				top: parent.top;
				bottom: parent.bottom;
			}

			onCurrentItemChanged: {
				if ((refreshRatesListView.currentItem == null) ||
				    (resolutionListView.currentItem == null)) {
					return;
				}

				var res = resolutionListView.currentItem.modelData.label;
				var rate = refreshRatesListView.currentItem.modelData.rate;

				output.setMode(res, rate);
			}
		}
	}

	onOutputChanged: {
		if (output == null) {
			resolutionModel.clear();
			refreshRatesModel.clear();
			outputName.text = "";
			return;
		}

		output.output.isEnabledChanged.connect(function() {
			outputSwitch.checked = output.output.enabled;
		});

		output.output.currentModeChanged.connect(selectCurrentMode);

		outputName.text = output.output.name;
		outputSwitch.checked = output.output.enabled;

		resolutionModel.clear();
		var resolutions = output.getResolutions();
		var currentMode = output.output.mode(output.output.currentMode);
		var currentResolution = (currentMode == null) ? -1 : currentMode.name;
		for (var i = 0; i < resolutions.length; i++) {
			resolutionModel.append({ "label": resolutions[i] });

			if (resolutions[i] == currentResolution) {
				resolutionListView.currentIndex = i;
			}
		}
	}

	function selectCurrentMode()
	{
		var currentMode = output.output.mode(output.output.currentMode);
		var resolutions = output.getResolutions();
		var currentResolution = (currentMode == null) ? -1 : currentMode.name;
		for (var i = 0; i < resolutions.length; i++) {
			if (resolutions[i] == currentResolution) {
				resolutionListView.currentIndex = i;
			}
		}
	}
}