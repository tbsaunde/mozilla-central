/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Oracle Corporation code.
 *
 * The Initial Developer of the Original Code is Oracle Corporation
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <stuart.parmenter@oracle.com>
 *   Philipp Kewisch <mozilla@kewis.ch>
 *   Daniel Boelzle <daniel.boelzle@sun.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

function getAlarmService() {
    if (!window.mAlarmService) {
        window.mAlarmService = Components.classes["@mozilla.org/calendar/alarm-service;1"]
                                         .getService(Components.interfaces.calIAlarmService);
    }
    return window.mAlarmService;
}

function onSnoozeAlarm(event) {
    // reschedule alarm:
    var duration = Components.classes["@mozilla.org/calendar/duration;1"]
                             .createInstance(Components.interfaces.calIDuration);
    duration.minutes = event.detail;
    duration.normalize();
    getAlarmService().snoozeAlarm(event.target.item, duration);
}

function onDismissAlarm(event) {
    getAlarmService().dismissAlarm(event.target.item);
}

function onDismissAllAlarms(event) {
    // removes widgets on the fly:
    var alarmlist = document.getElementById("alarmlist");
    for (var i = alarmlist.childNodes.length-1; i >= 0; i--) {
        getAlarmService().dismissAlarm(alarmlist.childNodes[i].item);
    }
    return true; // close dialog
}

function addWidgetFor(item) {
    var widget = document.createElement("calendar-alarm-widget");
    widget.setAttribute("title", item.title);
    var time = item.startDate || item.entryDate || item.dueDate;
    var dateFormatter = Components.classes["@mozilla.org/calendar/datetime-formatter;1"]
                                  .getService(Components.interfaces.calIDateTimeFormatter);
    widget.setAttribute("time", dateFormatter.formatDateTime(time));
    widget.setAttribute("location", item.getProperty("LOCATION"));
    widget.addEventListener("snooze", onSnoozeAlarm, false);
    widget.addEventListener("dismiss", onDismissAlarm, false);
    widget.item = item;
    document.getElementById("alarmlist").appendChild(widget);

    var snoozePref = getPrefSafe("calendar.alarms.defaultsnoozelength", 0);
    if (snoozePref > 0) {
        if ((snoozePref % 60) == 0) {
            snoozePref = snoozePref / 60;
            if ((snoozePref % 24) == 0) {
                snoozePref = snoozePref / 24;
                document.getAnonymousElementByAttribute(widget, "anonid", "alarm-widget-snooze-unit").selectedIndex = 2;
            } else {
                document.getAnonymousElementByAttribute(widget, "anonid", "alarm-widget-snooze-unit").selectedIndex = 1;
            }
        }
    } else {
        snoozePref = 0;
    }
    document.getAnonymousElementByAttribute(widget, "anonid", "alarm-widget-snooze-value").value = snoozePref;

    window.getAttention();
}

function removeWidgetFor(item) {
    var hashId = item.hashId;
    var alarmlist = document.getElementById("alarmlist");
    var nodes = alarmlist.childNodes;
    for (var i = nodes.length - 1; i >= 0; --i) {
        var widget = nodes[i];
        if (widget.item.hashId == hashId) {
            widget.removeEventListener("snooze", onSnoozeAlarm, false);
            widget.removeEventListener("dismiss", onDismissAlarm, false);
            alarmlist.removeChild(widget);
            if (!alarmlist.hasChildNodes()) {
                // check again next round since this removeWidgetFor call may be
                // followed by an addWidgetFor call (e.g. when refreshing), and
                // we don't want to close and open the window in that case.
                function closer() {
                    if (!alarmlist.hasChildNodes()) {
                        document.getElementById("calendar-alarmwindow").cancelDialog();
                    }
                }
                setTimeout(closer, 0);
            }
            break;
        }
    }
}
