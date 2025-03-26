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

    // FIXME deal with preconfigured parameters. Specifically
    // - reference luminance, which changes the brightness mapping
    // - peak luminance, which KWin clamps brightness levels to
    //
    // A quick but bad solution could be to just set the values as desired
    // through the output management protocol.
    //
    // A better solution could be a small calibration / profiling protocol, which
    // - makes KWin assume 10k as the maximum luminance, only while this window is visible
    // - removes reference luminance mapping, or adjusts it to the window while it's visible
    // - places the window on the correct screen
    //
    //
    // TODO:
    // - add a second page for the max average / max reference luminance
    // - check out what exactly the Windows tool for this is doing
    // - remove the max SDR level slider from the main page
    // - make the button on the main page look ok
    // - add next, reset, cancel and (on last page) apply buttons
    // - change peak luminance rect to be 10% of screen area

    ColumnLayout {
        Layout.fillWidth: true
        anchors.centerIn: parent

        WindowContainer {
            Layout.alignment: Qt.AlignHCenter
            window: Window {
                id: hdrTest
                visible: true
                onVisibleChanged: kcm.setHdrParameters(hdrTest, element.sdrBrightness, 10000)
                width: peakLuminanceRect.width
                height: peakLuminanceRect.height
                Rectangle {
                    id: peakLuminanceRect
                    color: "white"
                    width: Math.sqrt(hdrCalibration.width * hdrCalibration.height * 0.1)
                    height: width

                    WindowContainer {
                        width: peakLuminanceRect.width
                        height: peakLuminanceRect.height
                        window: Window {
                            id: hdrIcon
                            visible: true
                            onVisibleChanged: kcm.setHdrParameters(hdrIcon, element.sdrBrightness, element.peakBrightnessOverride)
                            flags: Qt.WA_TranslucentBackground
                            color: "#00000000"
                            Kirigami.Icon {
                                source: "plasma-symbolic"
                                color: "white"
                                width: peakLuminanceRect.width
                                height: peakLuminanceRect.height
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
            Layout.maximumWidth: peakLuminanceRect.width
            spacing: Kirigami.Units.smallSpacing

            QQC2.Slider {
                id: sdrBrightnessSlider
                Layout.fillWidth: true
                from: 100
                to: 2000
                live: true
                value: element.peakBrightnessOverride
                onMoved: {
                    element.peakBrightnessOverride = value;
                    kcm.setHdrParameters(hdrIcon, element.sdrBrightness, value);
                }
            }
            QQC2.SpinBox {
                from: 100
                to: 2000
                stepSize: 10
                value: element.peakBrightnessOverride
                onValueModified: {
                    element.peakBrightnessOverride = value;
                    kcm.setHdrParameters(hdrIcon, element.sdrBrightness, element.peakBrightnessOverride);
                }
            }
        }
        QQC2.Label {
            Layout.alignment: Qt.AlignHCenter
            text: i18n("To determine the maximum brightness of the screen, move the slider until you can just barely not see the logo anymore")
            color: "white"
        }
    }
}
