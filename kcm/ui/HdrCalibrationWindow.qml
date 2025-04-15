import QtCore
import QtQuick
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami

import org.kde.private.kcm.kscreen 1.0 as KScreen

Window {
    id: hdrCalibration
    title: i18nc("@title:window", "HDR calibration")
    visible: false
    color: "black"

    readonly property var tenPercentSize: Math.sqrt(hdrCalibration.width * hdrCalibration.height * 0.1);

    onVisibleChanged: {
        if (!visible) {
            return;
        }
        // we have to reset the values, otherwise
        // - KWin will clamp peak luminance to the previous value
        // - we need to show this calibration page at full brightness
        // TODO add some sort of calibration protocol, to take care of this in a better way?

        var oldPeakBrightness = element.peakBrightnessOverride == 0 ? element.peakBrightness : element.peakBrightnessOverride;
        element.peakBrightnessOverride = 2000;
        element.brightness = 1.0;
        kcm.doSave();
        element.peakBrightnessOverride = oldPeakBrightness;
    }

    ColumnLayout {
        id: peakLuminanceConfiguration
        Layout.fillWidth: true
        anchors.centerIn: parent

        WindowContainer {
            Layout.alignment: Qt.AlignHCenter
            window: Window {
                id: peakLuminanceWindow
                visible: true
                onVisibleChanged: kcm.setHdrParameters(peakLuminanceWindow, KScreen.KCMKScreen.Colorspace.BT709Linear, element.sdrBrightness, 10000, KScreen.KCMKScreen.RenderIntent.RelativeColorimetricBPC)
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
                            id: hdrIcon
                            visible: true
                            onVisibleChanged: kcm.setHdrParameters(hdrIcon, KScreen.KCMKScreen.Colorspace.BT709Linear, element.sdrBrightness, element.peakBrightnessOverride, KScreen.KCMKScreen.RenderIntent.RelativeColorimetricBPC)
                            flags: Qt.WA_TranslucentBackground
                            color: "#00000000"
                            width: hdrCalibration.tenPercentSize
                            height: hdrCalibration.tenPercentSize
                            Kirigami.Icon {
                                source: "plasma-symbolic"
                                color: "white"
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
                live: true
                value: element.peakBrightnessOverride
                onMoved: {
                    element.peakBrightnessOverride = value;
                    kcm.setHdrParameters(hdrIcon, KScreen.KCMKScreen.Colorspace.BT709Linear, element.sdrBrightness, value, KScreen.KCMKScreen.RenderIntent.RelativeColorimetricBPC);
                }
            }
            QQC2.SpinBox {
                from: 100
                to: 2000
                stepSize: 10
                value: element.peakBrightnessOverride
                onValueModified: {
                    element.peakBrightnessOverride = value;
                    kcm.setHdrParameters(hdrIcon, KScreen.KCMKScreen.Colorspace.BT709Linear, element.sdrBrightness, element.peakBrightnessOverride, KScreen.KCMKScreen.RenderIntent.RelativeColorimetricBPC);
                }
            }
            QQC2.Button {
                icon.name: "arrow-right"
                text: i18nc("@action:button", "Next")
                onClicked: {
                    peakLuminanceConfiguration.visible = false;
                    sdrLuminanceConfiguration.visible = true;
                    kcm.doSave();
                }

                Accessible.role: Accessible.Button
                Accessible.name: text
                Accessible.description: i18n("Switches to the next page")
                Accessible.onPressAction: onClicked();
            }
        }
        QQC2.Label {
            Layout.alignment: Qt.AlignHCenter
            text: i18n("To determine the maximum brightness of the screen, adjust the slider until the logo is barely visible")
            color: "white"
        }
    }

    ColumnLayout {
        id: sdrLuminanceConfiguration
        Layout.fillWidth: true
        anchors.centerIn: parent
        visible: false

        RowLayout {
            id: testImages
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Rectangle {
                // To give an "SDR white" background to compare the images to
                Layout.preferredWidth: hdrCalibration.tenPercentSize * 16/9.0 * 1.05625
                Layout.preferredHeight: hdrCalibration.tenPercentSize * 1.1
                color: "white"
                WindowContainer {
                    width: hdrCalibration.tenPercentSize * 16/9.0
                    height: hdrCalibration.tenPercentSize
                    anchors.centerIn: parent
                    window: Window {
                        id: hdrTestImageSurface
                        visible: true
                        width: parent.width
                        height: parent.height
                        onVisibleChanged: kcm.setHdrParameters(hdrTestImageSurface, KScreen.KCMKScreen.Colorspace.BT2020PQ, 203, 1000, KScreen.KCMKScreen.RenderIntent.Perceptual)

                        Rectangle {
                            width: parent.width
                            height: parent.height
                            color: "black"
                            Image {
                                width: parent.width
                                height: parent.height
                                source: "images/HDR outside.png"
                                fillMode: Image.PreserveAspectFit
                            }
                        }
                    }
                }
            }

            Rectangle {
                // To give an "SDR white" background to compare the images to
                Layout.preferredWidth: hdrCalibration.tenPercentSize * 16/9.0 * 1.05625
                Layout.preferredHeight: hdrCalibration.tenPercentSize * 1.1
                color: "white"

                // TODO replace this with some SDR image
                // either a picture, or a screenshot of some game perhaps?
                Rectangle {
                    width: hdrCalibration.tenPercentSize * 16/9.0
                    height: hdrCalibration.tenPercentSize
                    anchors.centerIn: parent
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "black"}
                        GradientStop { position: 1.0; color: "blue"}
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
                to: element.peakBrightnessOverride
                stepSize: 10
                live: true
                value: element.sdrBrightness
                onMoved: {
                    element.sdrBrightness = value
                    kcm.doSave();
                }
            }
            QQC2.SpinBox {
                from: 50
                to: element.peakBrightnessOverride
                stepSize: 10
                value: element.sdrBrightness
                onValueModified: {
                    element.sdrBrightness = value
                    kcm.doSave();
                }
            }
            QQC2.Button {
                icon.name: "dialog-ok-apply"
                text: i18nc("@action:button", "Done")
                onClicked: {
                    kcm.doSave();
                    hdrCalibration.visible = false;
                    sdrLuminanceConfiguration.visible = false;
                    peakLuminanceConfiguration.visible = true;
                }

                Accessible.role: Accessible.Button
                Accessible.name: text
                Accessible.description: i18n("Switches to the next page")
                Accessible.onPressAction: onClicked();
            }
        }
        QQC2.Label {
            Layout.alignment: Qt.AlignHCenter
            text: xi18nc("@label", "Configure how bright \"100\%\" on the normal brightness slider makes the screen.")
            color: "white"
        }
    }

}
