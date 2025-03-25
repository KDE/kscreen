import QtCore
import QtQuick 2.15
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
    // - make the peak luminance test work, and make it intuitive
    // - add a second page for the max average / max reference luminance
    // - min luminance setting? Probably isn't necessary
    // - check out what exactly the Windows tool for this is doing
    // - remove the max SDR level slider from the main page
    // - make the button on the main page look ok
    // - add next, reset, cancel and (on last page) apply buttons

    onVisibleChanged: kcm.setHdrParameters(hdrCalibration, 203, element.peakBrightnessOverride)

    ColumnLayout {
        Layout.fillWidth: true
        anchors.centerIn: parent

        // boo, the Window parent doesn't work!
        // Item {
        //     id: brightnessPresentation
        //     Window {
        //         visible: true
        //         parent: brightnessPresentation
                Rectangle {
                    id: peakLuminanceRect
                    Layout.alignment: Qt.AlignHCenter
                    color: "white"
                    // TODO change the size to be about 10% of the fullscreen window area
                    // so that it matches the common definition of "peak" luminance
                    width: 200
                    height: 200

                    Kirigami.Icon {
                        // TODO this should be extremely bright white / 10k nits
                        source: "plasma-symbolic"
                        color: "blue"
                        width: 200
                        height: 200
                        anchors.centerIn: parent
                    }
                }
        //     }
        // }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.maximumWidth: peakLuminanceRect.width
            spacing: Kirigami.Units.smallSpacing

            QQC2.Slider {
                id: sdrBrightnessSlider
                Layout.fillWidth: true
                from: 100
                to: 10000
                live: true
                value: element.peakBrightnessOverride
                onMoved: {
                    element.peakBrightnessOverride = value
                    kcm.setHdrParameters(hdrCalibration, 203, value)
                }
            }
            QQC2.SpinBox {
                from: 100
                to: 10000
                stepSize: 10
                value: element.peakBrightnessOverride
                onValueModified: {
                    element.peakBrightnessOverride = value
                    kcm.setHdrParameters(hdrCalibration, 203, value)
                }
            }
        }
        QQC2.Label {
            Layout.alignment: Qt.AlignHCenter
            text: i18n("Set brightness so that you can barely not see the logo anymore")
        }
    }
}

