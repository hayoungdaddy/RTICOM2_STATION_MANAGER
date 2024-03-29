/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.6
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls 2.0 as C2

Row {
    id: containerRow

    property var mapSource
    property real fontSize : 14
    property color labelBackground : "transparent"
    property int edge: Qt.RightEdge

    function rightEdge() {
        return (containerRow.edge === Qt.RightEdge);
    }

    layoutDirection: rightEdge() ? Qt.LeftToRight : Qt.RightToLeft
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.right: rightEdge() ? parent.right : undefined
    anchors.left: rightEdge() ? undefined : parent.left

    Rectangle {
        id: sliderContainer
        height: parent.height
        width: sliderRow.width + 10
        color: Qt.rgba( 0, 191 / 255.0, 255 / 255.0, 0.07)

        property var labelBorderColor: "transparent"
        property var slidersHeight : sliderContainer.height
                                     - rowSliderValues.height
                                     - rowSliderLabels.height
                                     - sliderColumn.spacing * 2
                                     - sliderColumn.topPadding
                                     - sliderColumn.bottomPadding

        Column {
            id: sliderColumn
            spacing: 10
            topPadding: 16
            bottomPadding: 48
            anchors.centerIn: parent

            // the sliders value labels
            Row {
                id: rowSliderValues
                spacing: sliderRow.spacing
                width: sliderRow.width
                height: 32
                property real entryWidth: zoomSlider.width

                Rectangle{
                    color: labelBackground
                    height: parent.height
                    width: parent.entryWidth
                    border.color: sliderContainer.labelBorderColor
                    Label {
                        id: labelZoomValue
                        text: zoomSlider.value.toFixed(3)
                        font.pixelSize: fontSize
                        rotation: -90
                        anchors.centerIn: parent
                    }
                }
            } // rowSliderValues

            // The sliders row
            Row {
                spacing: -10
                id: sliderRow
                height: sliderContainer.slidersHeight

                C2.Slider {
                    id: zoomSlider
                    height: parent.height
                    orientation : Qt.Vertical
                    from : containerRow.mapSource.minimumZoomLevel
                    to : containerRow.mapSource.maximumZoomLevel
                    value : containerRow.mapSource.zoomLevel
                    onValueChanged: {
                            containerRow.mapSource.zoomLevel = value
                    }
                }
            } // Row sliders

            // The labels row
            Row {
                id: rowSliderLabels
                spacing: sliderRow.spacing
                width: sliderRow.width
                property real entryWidth: zoomSlider.width
                property real entryHeight: 64

                Rectangle{
                    color: labelBackground
                    height: parent.entryHeight
                    width: parent.entryWidth
                    border.color: sliderContainer.labelBorderColor
                    Label {
                        id: labelZoom
                        text: "Zoom"
                        font.pixelSize: fontSize
                        rotation: -90
                        anchors.centerIn: parent
                    }
                }
            } // rowSliderLabels
        } // Column
    } // sliderContainer
} // containerRow
