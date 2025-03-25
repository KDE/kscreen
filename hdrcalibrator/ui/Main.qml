import QtCore
import QtQuick
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.hdrcalibrator

Window {
    id: hdrCalibration
    title: i18nc("@title:window", "HDR calibration")
    visible: false
    color: "black"

    readonly property var tenPercentSize: Math.sqrt(hdrCalibration.width * hdrCalibration.height * 0.1);

    Component.onCompleted: {
        // we have to reset the values, otherwise
        // - KWin will clamp peak luminance to the previous value
        // - we need to show this calibration page at full brightness
        // TODO add some sort of calibration protocol, to take care of this in a better way?

        var oldPeakBrightness = HdrCalibrator.peakBrightnessOverride == 0 ? HdrCalibrator.peakBrightness : HdrCalibrator.peakBrightnessOverride;
        // TODO what if the screen can go brighter than this?
        // Add a checkbox for extended range, or make the slider logarithmic or something?
        HdrCalibrator.peakBrightnessOverride = 2000;
        HdrCalibrator.brightness = 1.0;
        HdrCalibrator.applyConfig();
        if (oldPeakBrightness != 0) {
            // restore the old override, as a better starting point
            HdrCalibrator.peakBrightnessOverride = oldPeakBrightness;
        }

        for (var i = 0; i < Qt.application.screens.length; i++) {
            if (Qt.application.screens[i].name === HdrCalibrator.outputName) {
                hdrCalibration.screen = Qt.application.screens[i];
                break;
            }
        }
        hdrCalibration.showFullScreen();
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
                            id: hdrIcon
                            visible: true
                            onVisibleChanged: HdrHelper.setHdrParameters(hdrIcon, HdrHelper.Colorspace.BT709Linear, HdrCalibrator.sdrBrightness, HdrCalibrator.peakBrightnessOverride, HdrHelper.RenderIntent.RelativeColorimetricBPC)
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
                value: HdrCalibrator.peakBrightnessOverride
                onMoved: {
                    HdrCalibrator.peakBrightnessOverride = value;
                    HdrHelper.setHdrParameters(hdrIcon, HdrHelper.Colorspace.BT709Linear, HdrCalibrator.sdrBrightness, value, HdrHelper.RenderIntent.RelativeColorimetricBPC);
                }
            }
            QQC2.SpinBox {
                from: 100
                to: 2000
                stepSize: 10
                value: HdrCalibrator.peakBrightnessOverride
                onValueModified: {
                    HdrCalibrator.peakBrightnessOverride = value;
                    HdrHelper.setHdrParameters(hdrIcon, HdrHelper.Colorspace.BT709Linear, HdrCalibrator.sdrBrightness, HdrCalibrator.peakBrightnessOverride, HdrHelper.RenderIntent.RelativeColorimetricBPC);
                }
            }
            QQC2.Button {
                icon.name: "go-next"
                text: i18nc("@action:button", "Next")
                onClicked: {
                    peakLuminanceConfiguration.visible = false;
                    sdrLuminanceConfiguration.visible = true;
                    HdrCalibrator.sdrBrightness = HdrCalibrator.peakBrightnessOverride;
                    HdrCalibrator.applyConfig();
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
                    id: hdrTestImageContainer
                    width: hdrCalibration.tenPercentSize * 16/9.0
                    height: hdrCalibration.tenPercentSize
                    anchors.centerIn: parent
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
                                source: "images/graz.png"
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

                Rectangle {
                    width: hdrCalibration.tenPercentSize * 16/9.0
                    height: hdrCalibration.tenPercentSize
                    anchors.centerIn: parent
                    color: "black"

                    // the SDR "image" is just some gradients (for now)
                    // to make it easy to see when the display applies clipping
                    ColumnLayout {
                        spacing: Kirigami.Units.smallSpacing
                        anchors.centerIn: parent
                        width: hdrCalibration.tenPercentSize * 16/9.0
                        height: hdrCalibration.tenPercentSize

                        Rectangle {
                            Layout.preferredWidth: hdrCalibration.tenPercentSize * 16/9.0
                            Layout.preferredHeight: 100
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "black"}
                                GradientStop { position: 1.0; color: "white"}
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: hdrCalibration.tenPercentSize * 16/9.0
                            Layout.preferredHeight: 100
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "black"}
                                GradientStop { position: 1.0; color: "red"}
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: hdrCalibration.tenPercentSize * 16/9.0
                            Layout.preferredHeight: 100
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "black"}
                                GradientStop { position: 1.0; color: "green"}
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: hdrCalibration.tenPercentSize * 16/9.0
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
                    HdrCalibrator.sdrBrightness = value
                    HdrCalibrator.applyConfig();
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
                icon.name: "dialog-ok-apply"
                text: i18nc("@action:button", "Done")
                onClicked: {
                    HdrCalibrator.applyConfig();
                    hdrCalibration.close();
                }

                Accessible.role: Accessible.Button
                Accessible.name: text
                Accessible.description: i18n("Switches to the next page")
                Accessible.onPressAction: onClicked();
            }
        }
        QQC2.Label {
            Layout.alignment: Qt.AlignHCenter
            text: i18n("Configure how bright \"100\%\" on the normal brightness slider should be. Make it as bright as you'd ever use it, as long as the HDR image still looks good and the gradients are smooth.")
            color: "white"
        }
    }
}
