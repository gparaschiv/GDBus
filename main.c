#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>

#include "alarmclock_dbus_generated.h"


// keep local time in ms
guint64 localTime = 0;
// keep local alarm time in ms
guint64 localAlarmTime = 0;
// keep alarm status
gboolean localAlarmStatus = FALSE;
alarmClockAlarmclock *interface;
guint lastTag = -1;
guint tag = 0;


gboolean sendAlarm(gpointer user_data) {
    g_print ("sendAlarm\n");

    alarm_clock_alarmclock_emit_ring_alarm(interface);
    g_print ("sendAlarm1\n");
    //g_source_remove (tag);
    lastTag = -1;
    //function will be called only one time because return FALSE
    return FALSE;
}

void configAlarm() {
    g_print ("configAlarm\n");
    if ((localTime < localAlarmTime) && (localAlarmStatus)) {

        guint diff = (guint)(localAlarmTime - localTime);

        if (tag == lastTag){
            // if another alarm is ongoing stop it
            g_source_remove (tag);
            g_print ("Remove tag\n");
        }

        // call cyclic sendALarm after with a delay in ms represented by diff value
        tag = g_timeout_add(diff, sendAlarm , NULL);
        lastTag = tag;
    }
}

static gboolean
on_set_time (alarmClockAlarmclock *myspaceProp, GDBusMethodInvocation *invocation,
             const guint64 currentTime)
{
    localTime = currentTime;
    configAlarm();
    alarm_clock_alarmclock_complete_set_time(interface,invocation);
    return TRUE;
}

static gboolean
on_set_alarm_time (alarmClockAlarmclock *interface, GDBusMethodInvocation *invocation,
             const guint64 alarmTime)
{
    g_print ("on_set_alarm_time\n");
    localAlarmTime = alarmTime;

    configAlarm();
    alarm_clock_alarmclock_complete_set_alarm_time(interface,invocation);

    return TRUE;
}

static gboolean
on_set_alarm_status (alarmClockAlarmclock *interface, GDBusMethodInvocation *invocation,
             const gboolean alarmStatus)
{
    g_print ("on_set_alarm_status\n");
    localAlarmStatus = alarmStatus;
    configAlarm();
    alarm_clock_alarmclock_complete_set_alarm_status(interface,invocation);
    return TRUE;
}

static gboolean
on_get_alarm_status (alarmClockAlarmclock *interface, GDBusMethodInvocation *invocation)
{
    g_print ("on_get_alarm_status\n");
    alarm_clock_alarmclock_complete_get_alarm_status(interface,invocation,localAlarmStatus);
    return TRUE;
}

static void
on_bus_acquired (GDBusConnection *connection,
                const gchar *name,
                gpointer user_data)
{
    GDBusObjectManagerServer *manager = NULL;
    g_print ("on_bus_acquired\n");

    alarmClockObjectSkeleton *object;
    manager = g_dbus_object_manager_server_new ("/com/time/service/manager");
    object = alarm_clock_object_skeleton_new("/com/time/service/manager/object");

    interface = alarm_clock_alarmclock_skeleton_new();
    alarm_clock_object_skeleton_set_alarmclock (object,interface);
    g_object_unref(interface);
    g_signal_connect (interface, "handle_set_time", G_CALLBACK (on_set_time), NULL);
    g_signal_connect (interface, "handle_set_alarm_time", G_CALLBACK (on_set_alarm_time), NULL);
    g_signal_connect (interface, "handle_get_alarm_status", G_CALLBACK (on_get_alarm_status), NULL);
    g_signal_connect (interface, "handle_set_alarm_status", G_CALLBACK (on_set_alarm_status), NULL);

    g_dbus_object_manager_server_export (manager, G_DBUS_OBJECT_SKELETON (object));

    g_object_unref(object);
    g_dbus_object_manager_server_set_connection(manager,connection);
    g_print ("on_bus_acquired final\n");
}

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  g_print ("Acquired the name %s\n", name);
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  g_print ("Lost the name %s\n", name);
}

int main()
{
    GMainLoop *loop;

    loop = g_main_loop_new (NULL, FALSE);
    g_bus_own_name(G_BUS_TYPE_SYSTEM, // G_BUS_TYPE_SESSION, //
                   "com.time.service.alarmclock",
                   G_BUS_NAME_OWNER_FLAGS_NONE,
                   on_bus_acquired,
                   on_name_acquired,
                   on_name_lost,
                   NULL,
                   NULL);

    g_main_loop_run (loop);
    //clean up
    g_main_loop_unref(loop);
    return 0;
}
