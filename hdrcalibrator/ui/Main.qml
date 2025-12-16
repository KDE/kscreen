/*
    SPDX-FileCopyrightText: 2025 Xaver Hugl <xaver.hugl@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.hdrcalibrator

Window {
    id: hdrCalibration
    title: i18nc("@title:window", "HDR Calibration")
    // on the first show(FullScreen), Qt sets the screen of a window to the one matching its geometry
    // so setting the screen before first show on Wayland doesn't work, as geometry is always at position 0
    // as a workaround, show the window first and then set the screen afterwards
    visible: true
    color: "black"

    readonly property real tenPercentSize: Math.sqrt(hdrCalibration.width * hdrCalibration.height * 0.1);
    readonly property bool horizontal: hdrCalibration.width >= hdrCalibration.height
    property real oldPeakBrightness: 0
    property real oldMaxAverageBrightness: 0
    property real oldSdrBrightness: 0

    Component.onCompleted: {
        // we have to reset the values, otherwise
        // - KWin will clamp peak luminance to the previous value
        // - we need to show this calibration page at full brightness
        // TODO add some sort of calibration protocol, to take care of this in a better way?

        hdrCalibration.oldPeakBrightness = HdrCalibrator.peakBrightnessOverride == 0 ? HdrCalibrator.peakBrightness : HdrCalibrator.peakBrightnessOverride;
        hdrCalibration.oldMaxAverageBrightness = HdrCalibrator.maxAverageBrightnessOverride == 0 ? HdrCalibrator.maxAverageBrightness : HdrCalibrator.maxAverageBrightnessOverride;
        hdrCalibration.oldSdrBrightness = HdrCalibrator.sdrBrightness;
        // TODO what if the screen can go brighter than this?
        // Add a checkbox for extended range, or make the slider logarithmic or something?
        HdrCalibrator.peakBrightnessOverride = 2000;
        HdrCalibrator.brightness = 1.0;
        HdrCalibrator.applyConfig();
        // restore the old value as a better starting point
        if (hdrCalibration.oldPeakBrightness != 0) {
            HdrCalibrator.peakBrightnessOverride = hdrCalibration.oldPeakBrightness;
        }

        for (var output of Qt.application.screens) {
            if (output.name === HdrCalibrator.outputName) {
                hdrCalibration.screen = output;
                break;
            }
        }
        hdrCalibration.showFullScreen();
    }

    function resetConfigAndQuit() {
        HdrCalibrator.peakBrightnessOverride = hdrCalibration.oldPeakBrightness;
        HdrCalibrator.maxAverageBrightnessOverride = hdrCalibration.oldMaxAverageBrightness;
        HdrCalibrator.sdrBrightness = hdrCalibration.oldSdrBrightness;
        HdrCalibrator.applyConfig();
        hdrCalibration.close();
    }

    QQC2.StackView {
        id: stack
        initialItem: peakLuminanceConfiguration
        anchors.fill: parent

        // NOTE that WindowContainers break most transition animations
        pushEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: Kirigami.Units.shortDuration
            }
        }
        pushExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: Kirigami.Units.shortDuration
            }
        }
        popEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: Kirigami.Units.shortDuration
            }
        }
        popExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: Kirigami.Units.shortDuration
            }
        }
    }

    onClosing: {
        HdrCalibrator.safeQuit();
    }

    Component {
        id: peakLuminanceConfiguration

        Rectangle {
            color: "black"
            ColumnLayout {
                Layout.fillWidth: true
                anchors.centerIn: parent
                WindowContainer {
                    Layout.alignment: Qt.AlignHCenter
                    window: Window {
                        id: peakLuminanceWindow
                        visible: true
                        onVisibleChanged: HdrHelper.setHdrParameters(peakLuminanceWindow, HdrHelper.Colorspace.BT709Linear, HdrCalibrator.sdrBrightness, 10000, HdrHelper.RenderIntent.RelativeColorimetricBPC)
                        width: hdrCalibration.tenPercentSize
                        height: hdrCalibration.tenPercentSize
                        Rectangle {
                            id: peakLuminanceRect
                            color: "white"
                            width: hdrCalibration.tenPercentSize
                            height: hdrCalibration.tenPercentSize

                            WindowContainer {
                                width: hdrCalibration.tenPercentSize
                                height: hdrCalibration.tenPercentSize
                                window: Window {
                                    id: hdrIconWindow
                                    visible: true
                                    onVisibleChanged: HdrHelper.setHdrParameters(hdrIconWindow, HdrHelper.Colorspace.BT709Linear, HdrCalibrator.sdrBrightness, 2550.0, HdrHelper.RenderIntent.RelativeColorimetricBPC)
                                    flags: Qt.WA_TranslucentBackground
                                    color: "#00000000"
                                    width: hdrCalibration.tenPercentSize
                                    height: hdrCalibration.tenPercentSize
                                    Kirigami.Icon {
                                        id: hdrIcon
                                        source: Qt.resolvedUrl("images/plasma-symbolic.svg")
                                        // TODO once QTBUG-135232 is fixed, switch back to color: "white"
                                        // and setting the color management parameters instead
                                        color: Qt.rgba(HdrCalibrator.peakBrightnessOverride / 2550, HdrCalibrator.peakBrightnessOverride / 2550, HdrCalibrator.peakBrightnessOverride / 2550, 1.0)
                                        width: hdrCalibration.tenPercentSize
                                        height: hdrCalibration.tenPercentSize
                                        anchors.centerIn: parent
                                    }
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.maximumWidth: peakLuminanceWindow.width
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.Slider {
                        id: peakBrightnessSlider
                        Layout.fillWidth: true
                        from: 100
                        to: 2000
                        stepSize: 10
                        live: true
                        value: HdrCalibrator.peakBrightnessOverride
                        onMoved: {
                            if (value != HdrCalibrator.peakBrightnessOverride) {
                                HdrCalibrator.peakBrightnessOverride = value;
                                hdrIcon.color = Qt.rgba(HdrCalibrator.peakBrightnessOverride / 2550, HdrCalibrator.peakBrightnessOverride / 2550, HdrCalibrator.peakBrightnessOverride / 2550, 1.0);
                            }
                        }
                    }
                    QQC2.SpinBox {
                        from: 100
                        to: 2000
                        stepSize: 10
                        value: HdrCalibrator.peakBrightnessOverride
                        onValueModified: {
                            HdrCalibrator.peakBrightnessOverride = value;
                            hdrIcon.color = Qt.rgba(HdrCalibrator.peakBrightnessOverride / 2550, HdrCalibrator.peakBrightnessOverride / 2550, HdrCalibrator.peakBrightnessOverride / 2550, 1.0);
                        }
                    }
                    QQC2.Button {
                        icon.name: "go-next"
                        text: i18nc("@action:button", "Next")
                        onClicked: {
                            HdrCalibrator.sdrBrightness = Math.min(HdrCalibrator.peakBrightnessOverride, HdrCalibrator.sdrBrightness);
                            HdrCalibrator.applyConfig();
                            stack.push(maxFallLuminanceConfiguration);
                        }

                        Accessible.description: i18nc("@info accessible description of a push button", "Switches to the next page")
                    }
                    QQC2.Button {
                        icon.name: "dialog-cancel"
                        text: i18nc("@action:button", "Cancel")
                        onClicked: hdrCalibration.resetConfigAndQuit();

                        Accessible.description: i18nc("@info accessible description of a push button", "Cancels the calibration process")
                    }
                }
                QQC2.Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: i18n("To determine the maximum brightness of the screen, adjust the slider until the logo is barely visible")
                    color: "white"
                }
            }
        }
    }

    Component {
        id: maxFallLuminanceConfiguration
        Rectangle {
            color: "white"
            ColumnLayout {
                Layout.fillWidth: true
                anchors.centerIn: parent

                WindowContainer {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: hdrCalibration.tenPercentSize
                    Layout.preferredHeight: hdrCalibration.tenPercentSize
                    window: Window {
                        id: hdrIconWindow2
                        visible: true
                        onVisibleChanged: HdrHelper.setHdrParameters(hdrIconWindow2, HdrHelper.Colorspace.BT709Linear, HdrCalibrator.sdrBrightness, 10000.0, HdrHelper.RenderIntent.RelativeColorimetricBPC)
                        flags: Qt.WA_TranslucentBackground
                        color: "#00000000"
                        width: hdrCalibration.tenPercentSize
                        height: hdrCalibration.tenPercentSize
                        Kirigami.Icon {
                            source: Qt.resolvedUrl("images/plasma-symbolic.svg")
                            color: "white"
                            width: hdrCalibration.tenPercentSize
                            height: hdrCalibration.tenPercentSize
                            anchors.centerIn: parent
                        }
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.maximumWidth: hdrCalibration.tenPercentSize
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.Slider {
                        Layout.fillWidth: true
                        from: 100
                        to: HdrCalibrator.peakBrightnessOverride
                        stepSize: 10
                        live: true
                        value: HdrCalibrator.sdrBrightness
                        onMoved: {
                            if (value != HdrCalibrator.sdrBrightness) {
                                HdrCalibrator.sdrBrightness = value;
                                HdrCalibrator.maxAverageBrightnessOverride = value;
                                HdrCalibrator.applyConfig();
                            }
                        }
                    }
                    QQC2.SpinBox {
                        from: 100
                        to: HdrCalibrator.peakBrightnessOverride
                        stepSize: 10
                        value: HdrCalibrator.sdrBrightness
                        onValueModified: {
                            HdrCalibrator.sdrBrightness = value;
                            HdrCalibrator.maxAverageBrightnessOverride = value;
                            HdrCalibrator.applyConfig();
                        }
                    }
                    QQC2.Button {
                        icon.name: "go-next"
                        text: i18nc("@action:button", "Next")
                        onClicked: {
                            HdrCalibrator.applyConfig();
                            stack.push(sdrLuminanceConfiguration);
                        }

                        Accessible.description: i18nc("@info accessible description of a push button", "Switches to the next page")
                    }
                    QQC2.Button {
                        icon.name: "dialog-cancel"
                        text: i18nc("@action:button", "Cancel")
                        onClicked: hdrCalibration.resetConfigAndQuit();

                        Accessible.description: i18nc("@info accessible description of a push button", "Cancels the calibration process")
                    }
                }
                QQC2.Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: i18n("To determine the maximum average brightness of the screen, adjust the slider until the logo is barely visible")
                    color: "black"
                }
            }
        }
    }

    Component {
        id: sdrLuminanceConfiguration

        Rectangle {
            color: "black"
            ColumnLayout {
                id: sdrLuminanceLayout
                Layout.fillWidth: true
                anchors.centerIn: parent

                // To give an "SDR white" background to compare the images to
                readonly property int whiteMargins: 30

                GridLayout {
                    id: testImages
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    columnSpacing: Kirigami.Units.smallSpacing
                    rowSpacing: Kirigami.Units.smallSpacing
                    columns: hdrCalibration.horizontal ? 2 : 1
                    rows: hdrCalibration.horizontal ? 1 : 2

                    readonly property real elementWidth: Math.min(hdrCalibration.tenPercentSize * 16/9.0 + 2 * sdrLuminanceLayout.whiteMargins, hdrCalibration.width)
                    readonly property real elementHeight: (elementWidth - 2 * sdrLuminanceLayout.whiteMargins) * 9.0/16.0 + 2 * sdrLuminanceLayout.whiteMargins

                    Rectangle {
                        Layout.preferredWidth: testImages.elementWidth
                        Layout.preferredHeight: testImages.elementHeight
                        color: "white"
                        WindowContainer {
                            id: hdrTestImageContainer
                            anchors.fill: parent
                            anchors.margins: sdrLuminanceLayout.whiteMargins
                            window: Window {
                                id: hdrTestImageSurface
                                visible: true
                                width: hdrTestImageContainer.width
                                height: hdrTestImageContainer.height
                                onVisibleChanged: HdrHelper.setHdrParameters(hdrTestImageSurface, HdrHelper.Colorspace.BT2020PQ, 203, 1000, HdrHelper.RenderIntent.Perceptual)

                                Rectangle {
                                    width: parent.width
                                    height: parent.height
                                    color: "black"
                                    Image {
                                        width: parent.width
                                        height: parent.height
                                        source: "images/graz.avif"
                                        fillMode: Image.PreserveAspectFit
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.preferredWidth: testImages.elementWidth
                        Layout.preferredHeight: testImages.elementHeight
                        color: "white"

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: sdrLuminanceLayout.whiteMargins
                            color: "black"

                            // the SDR "image" is just some gradients (for now)
                            // to make it easy to see when the display applies clipping
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: Kirigami.Units.smallSpacing

                                Rectangle {
                                    Layout.preferredWidth: parent.width
                                    Layout.preferredHeight: 100
                                    gradient: Gradient {
                                        orientation: Gradient.Horizontal
                                        GradientStop { position: 0.0; color: "black"}
                                        GradientStop { position: 1.0; color: "white"}
                                    }
                                }

                                Rectangle {
                                    Layout.preferredWidth: parent.width
                                    Layout.preferredHeight: 100
                                    gradient: Gradient {
                                        orientation: Gradient.Horizontal
                                        GradientStop { position: 0.0; color: "black"}
                                        GradientStop { position: 1.0; color: "red"}
                                    }
                                }

                                Rectangle {
                                    Layout.preferredWidth: parent.width
                                    Layout.preferredHeight: 100
                                    gradient: Gradient {
                                        orientation: Gradient.Horizontal
                                        GradientStop { position: 0.0; color: "black"}
                                        GradientStop { position: 1.0; color: "green"}
                                    }
                                }

                                Rectangle {
                                    Layout.preferredWidth: parent.width
                                    Layout.preferredHeight: 100
                                    gradient: Gradient {
                                        orientation: Gradient.Horizontal
                                        GradientStop { position: 0.0; color: "black"}
                                        GradientStop { position: 1.0; color: "blue"}
                                    }
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.maximumWidth: hdrCalibration.tenPercentSize
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.Slider {
                        id: sdrBrightnessSlider
                        Layout.fillWidth: true
                        from: 50
                        to: HdrCalibrator.peakBrightnessOverride
                        stepSize: 10
                        live: true
                        value: HdrCalibrator.sdrBrightness
                        onMoved: {
                            if (HdrCalibrator.sdrBrightness != value) {
                                HdrCalibrator.sdrBrightness = value;
                                HdrCalibrator.applyConfig();
                            }
                        }
                    }
                    QQC2.SpinBox {
                        from: 50
                        to: HdrCalibrator.peakBrightnessOverride
                        stepSize: 10
                        value: HdrCalibrator.sdrBrightness
                        onValueModified: {
                            HdrCalibrator.sdrBrightness = value
                            HdrCalibrator.applyConfig();
                        }
                    }
                    QQC2.Button {
                        icon.name: "go-next"
                        text: i18nc("@action:button", "Next")
                        onClicked: {
                            HdrCalibrator.applyConfig();
                            stack.push(windowsHdrConfiguration);
                        }

                        Accessible.description: i18nc("@info accessible description of a push button", "Switches to the next page")
                    }
                    QQC2.Button {
                        icon.name: "dialog-cancel"
                        text: i18nc("@action:button", "Cancel")
                        onClicked: hdrCalibration.resetConfigAndQuit();

                        Accessible.description: i18nc("@info accessible description of a push button", "Cancels the calibration process")
                    }
                }
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.maximumWidth: parent.width - 2 * Kirigami.Units.smallSpacing
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap

                    text: i18n("Configure how bright \"100\%\" on the normal brightness slider should be. Make it as bright as you'd ever use it, as long as the HDR image still looks good and the gradients are smooth.\nTo avoid brightness fluctuations, it's recommended to not exceed the display's maximum average brightness of %1cd/m²", HdrCalibrator.maxAverageBrightnessOverride)
                    color: "white"
                }
            }
        }
    }

    Component {
        id: windowsHdrConfiguration

        Rectangle {
            color: "black"

            ColumnLayout {
                anchors.centerIn: parent
                RowLayout {
                    WindowContainer {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: hdrCalibration.tenPercentSize * 0.5
                        Layout.preferredHeight: hdrCalibration.tenPercentSize * 0.5
                        window: Window {
                            visible: true
                            onVisibleChanged: HdrHelper.setHdrParameters(this, HdrHelper.Colorspace.BT709Linear, HdrCalibrator.sdrBrightness, HdrCalibrator.peakBrightnessOverride, HdrHelper.RenderIntent.RelativeColorimetricBPC)
                            color: "white"
                            width: hdrCalibration.tenPercentSize
                            height: hdrCalibration.tenPercentSize
                            // NOTE that the text requires a separate window, as it looks terrible with linear brightness values
                            WindowContainer {
                                anchors.fill: parent
                                window: Window {
                                    visible: true
                                    width: hdrCalibration.tenPercentSize
                                    height: hdrCalibration.tenPercentSize
                                    flags: Qt.WA_TranslucentBackground
                                    color: "#00000000"
                                    QQC2.Label {
                                        anchors.centerIn: parent
                                        color: "black"
                                        text: i18n("Maximum luminance: %1cd/m²", HdrCalibrator.peakBrightnessOverride)
                                    }
                                }
                            }
                        }
                    }

                    WindowContainer {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: hdrCalibration.tenPercentSize * 0.5
                        Layout.preferredHeight: hdrCalibration.tenPercentSize * 0.5
                        window: Window {
                            visible: true
                            onVisibleChanged: HdrHelper.setHdrParameters(this, HdrHelper.Colorspace.BT709Linear, HdrCalibrator.sdrBrightness, HdrCalibrator.maxAverageBrightnessOverride, HdrHelper.RenderIntent.RelativeColorimetricBPC)
                            color: "white"
                            width: hdrCalibration.tenPercentSize
                            height: hdrCalibration.tenPercentSize
                            // NOTE that the text requires a separate window, as it looks terrible with linear brightness values
                            WindowContainer {
                                anchors.fill: parent
                                window: Window {
                                    visible: true
                                    width: hdrCalibration.tenPercentSize
                                    height: hdrCalibration.tenPercentSize
                                    flags: Qt.WA_TranslucentBackground
                                    color: "#00000000"
                                    QQC2.Label {
                                        anchors.centerIn: parent
                                        color: "black"
                                        text: i18n("Maximum fullscreen luminance: %1cd/m²", HdrCalibrator.maxAverageBrightnessOverride)
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: hdrCalibration.tenPercentSize * 0.5
                        Layout.preferredHeight: hdrCalibration.tenPercentSize * 0.5
                        color: "white"
                        QQC2.Label {
                            anchors.centerIn: parent
                            color: "black"
                            text: i18n("Reference luminance / paper white: %1cd/m²", HdrCalibrator.sdrBrightness)
                        }
                    }
                }
                RowLayout {
                    id: windowsHdrRow

                    Layout.alignment: Qt.AlignHCenter
                    QQC2.CheckBox {
                        id: applyToWindowsApps
                        checked: true
                        text: i18nc("@action:checkbox", "Assume Windows HDR applications are calibrated for this screen")
                        Kirigami.Theme.textColor: "white"
                    }
                    QQC2.Button {
                        icon.name: "dialog-cancel"
                        text: i18nc("@action:button", "Cancel")
                        onClicked: hdrCalibration.resetConfigAndQuit();

                        Accessible.description: i18nc("@info accessible description of a push button", "Cancels the calibration process")
                    }
                    QQC2.Button {
                        icon.name: "dialog-ok-apply"
                        text: i18nc("@action:button", "Done")
                        onClicked: {
                            HdrCalibrator.applyConfig();
                            if (applyToWindowsApps.checked) {
                                HdrCalibrator.applyConfigForWindowsApps();
                            }
                            hdrCalibration.close();
                        }

                        Accessible.description: i18nc("@info accessible description of a push button", "Finish the calibration process")
                    }
                }
                QQC2.Label {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.maximumWidth: windowsHdrRow.width
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: i18n("Many Windows applications come with their own calibration settings, which this system wide configuration conflicts with. If this option is set, their calibration settings will work as expected on this specific display.")
                    color: "white"
                }
            }
        }
    }
}
