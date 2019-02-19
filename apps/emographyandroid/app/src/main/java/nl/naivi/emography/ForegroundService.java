package nl.naivi.emography;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.support.annotation.Keep;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

/**
 * Emography foreground service, which is always available.
 * Manages the emography NAP application environment.
 * This service can send and receive messages from the running NAP application.
 * The NAP application runs in the background on a separate thread.
 * Sending messages to the NAP application is a-synchronous, ie: won't block the calling thread.
 * Messages are processed in order, ie: first in - first out.
 */
public class ForegroundService extends Service
{
    private static final String TAG = "NAPEmography";
    private IntentFilter mIntentFilter;
    private APIThread mAPIThread = null;

    @Override
    public void onCreate()
    {
        super.onCreate();
        Log.i(TAG, "ForegroundService.onCreate");

        // Accept shutdown and logging request intents
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(Constants.ACTION.STOP_SERVICE);
        mIntentFilter.addAction(Constants.ACTION.API_SEND_MESSAGE);

        // Build worker thread
        mAPIThread = new APIThread(this);
    }


    @Override
    public int onStartCommand(Intent intent, int flags, int startId)
    {
        Log.i(TAG, "ForegroundService.onStart");

        // Start broadcast receiver
        registerReceiver(mReceiver, mIntentFilter);

        // Create notification channels
        createNotificationChannel();

        // Intent for touch on notification
        Intent notificationTouchIntent = new Intent(this, MainActivity.class);
        PendingIntent notificationTouchPendingIntent =
                PendingIntent.getActivity(this, 0, notificationTouchIntent, 0);

        // Intent for shutdown button
        Intent buttonShutdownIntent = new Intent();
        buttonShutdownIntent.setAction(Constants.ACTION.STOP_SERVICE);
        PendingIntent stopPendingIntent = PendingIntent.getBroadcast(this, 0, buttonShutdownIntent, 0);

        // Build notification
        Notification notification =
                new NotificationCompat.Builder(this, Constants.NOTIFICATIONS.CHANNEL_ID)
                        .setContentTitle(getText(R.string.notification_title))
                        .setContentText(getText(R.string.notification_message))
                        .setSmallIcon(R.drawable.ic_nap_logo)
                        .setContentIntent(notificationTouchPendingIntent)
                        .setTicker(getText(R.string.ticker_text))
                        .addAction(android.R.drawable.ic_menu_close_clear_cancel, getString(R.string.shutdown), stopPendingIntent)
                        .build();

        // Run in foreground, show notification
        startForeground(Constants.NOTIFICATIONS.FOREGROUND_SERVICE_ID, notification);

        // Start our thread
        mAPIThread.start();

        // Keep this service around
        return START_STICKY;
    }


    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent)
        {
            if (intent.getAction().equals(Constants.ACTION.STOP_SERVICE))
            {
                // Shutdown request received
                Log.i(TAG, "Received stop service intent");
                kill();
            }
            else if (intent.getAction().equals(Constants.ACTION.API_SEND_MESSAGE))
            {
                String api_request = intent.getStringExtra("apimessage");

                // Call into nap with the request
                call(api_request);
            }
        }
    };


    // Create notification channel as required on Android O+
    private void createNotificationChannel()
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
        {
            CharSequence name = getString(R.string.channel_name);
            String description = getString(R.string.channel_description);
            int importance = NotificationManager.IMPORTANCE_LOW;
            NotificationChannel channel = new NotificationChannel(Constants.NOTIFICATIONS.CHANNEL_ID, name, importance);
            channel.setDescription(description);

            NotificationManager notificationManager = getSystemService(NotificationManager.class);
            notificationManager.createNotificationChannel(channel);
        }
    }


     /**
      * Sends a message to the running NAP application as a json formatted string.
      * After receiving a message flush() is called to processes the event.
      * This is an a-synchronous call that won't block the calling thread.
      *
      * @param data the NAP message as json formatted string.
      */
    private void call(String data)
    {
         if(!mAPIThread.sendMessage(data)) {
             Log.e(TAG, "Unable to send API message: " + data);
         }
    }


    /**
     * Stop this service and request that the activity stop too
     * This is called when an intent to stop the service is received.
     */
    public void kill()
    {
        stopForeground(true);
        stopSelf();

        Intent appStopIntent = new Intent();
        appStopIntent.setAction(Constants.ACTION.STOP_ACTIVITY);
        sendBroadcast(appStopIntent);
    }


    /**
     * Called on destruction, stops the NAP thread from running.
     * On destruction the NAP system is flushed before exiting.
     */
    @Override
    public void onDestroy()
    {
        // Stop API thread from running
        Log.i(TAG, "ForegroundService.onDestroy");
        mAPIThread.stopRunning();

        // Unregister broadcast receiver
        unregisterReceiver(mReceiver);
        super.onDestroy();
    }


    @Override
    public IBinder onBind(Intent intent)
    {
        // Used only in case of bound services.
        return null;
    }


    @Keep
    /**
     * Called by the NAP API Service when a new log message is received
     * Log messages are often errors or warnings.
     * When received this log message is forwarded to potential listeners
     * @param logMessage the log message received from the NAP API service
     */
    public void onAPILog(final String logMessage)
    {
        Handler mainHandler = new Handler(this.getMainLooper());
        Runnable myRunnable = new Runnable()
        {
            @Override
            public void run() {
            Intent logIntent = new Intent();
            logIntent.setAction(Constants.ACTION.API_LOG_ACTIVITY);
            logIntent.putExtra(Constants.API.LOG, logMessage);
            sendBroadcast(logIntent);
            }
        };
        mainHandler.post(myRunnable);
    }

    @Keep
    /**
     * Called by the NAP API Service when a new event in the form of a message is received
     * Forwards it to the main activity
     * @param apiMessage the api message as a json deserializable structure
     */
    public void onAPIMessage(final String apiMessage)
    {
        Handler mainHandler = new Handler(this.getMainLooper());
        Runnable myRunnable = new Runnable()
        {
            @Override
            public void run() {
                Intent apiIntent = new Intent();
                apiIntent.setAction(Constants.ACTION.API_RESPONSE_ACTIVITY);
                apiIntent.putExtra(Constants.API.MESSAGE, apiMessage);
                sendBroadcast(apiIntent);
            }
        };
        mainHandler.post(myRunnable);
    }


    // Load native library
    static {
        System.loadLibrary("nap-service");
    }


    ////////////////////////////////////////////////////////////
    // Native Java Calls
    ////////////////////////////////////////////////////////////

    /**
     * Java JNI Wrapper function to initialize the NAP application
     * @return if the nap application initialized correctly.
     */
    public native boolean   napInit();

    /**
     * Java JNI Wrapper function to update the initialized NAP application
     */
    public native void      napUpdate();

    /**
     * Java JNI Wrapper function to send a message to the initialized NAP application
     * @param jsonMessage the json formatted api message as string.
     * @return if the message was accepted by the system.
     */
    public native boolean   napSendMessage(String jsonMessage);

    /**
     * Java JNI Wrapper function to shut down the running NAP application
     */
    public native void      napShutdown();
}
