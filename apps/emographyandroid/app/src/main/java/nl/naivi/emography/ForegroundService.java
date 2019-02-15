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
import android.widget.Toast;

import java.util.concurrent.atomic.AtomicBoolean;

public class ForegroundService extends Service
{
    private static final String TAG = "NAPEmography";
    private IntentFilter mIntentFilter;
    private volatile boolean mStopThread = false;
    private AtomicBoolean mFlush = new AtomicBoolean(false);
    final private Object mLock = new Object();

    @Override
    public void onCreate()
    {
        super.onCreate();
        Log.i(TAG, "Service onCreate");

        // Accept shutdown and logging request intents
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(Constants.ACTION.STOP_SERVICE);
        mIntentFilter.addAction(Constants.ACTION.API_SEND_MESSAGE);
    }


    @Override
    public int onStartCommand(Intent intent, int flags, int startId)
    {
        // Start broadcast receiver
        registerReceiver(mReceiver, mIntentFilter);

        // Create notification channel
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

        // Build worker thread
        Thread nap_thread = new Thread(new Runnable()
        {
            @Override
            public void run()
            {
                // Initialisation
                boolean initialized = napInit();

                // Check init success
                if (!initialized)
                {
                    Log.e(TAG, "Abandoning launch due to NAP initialization failure");
                    Handler mainHandler = new Handler(ForegroundService.this.getMainLooper());
                    Runnable myRunnable = new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            Toast.makeText(ForegroundService.this, "NAP initialization failure", Toast.LENGTH_SHORT).show();
                            stopService();
                        }
                    };
                    mainHandler.post(myRunnable);
                    return;
                }

                // Update NAP when a new message is received
                while (!Thread.interrupted() && !mStopThread)
                {
                    synchronized (mLock) {
                        try {
                            // Wait until we need to flush
                            while (!mFlush.get()) {
                                mLock.wait();
                            }
                        }
                        catch(InterruptedException e) {
                            Log.e("error", e.toString());
                        } // Perform action appropriate to condition
                    }

                    // Reset
                    mFlush.set(false);

                    // Flush - forces an update on the C++ side
                    napUpdate();
                }

                // Shutdown NAP core
                napShutdown();
                Log.i(TAG, "Service thread exiting cleanly");
            }
        });

        // Start our thread
        nap_thread.start();
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
                stopService();
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
     * Notifies the NAP thread to flush all current events (messages)
     * All events are processed in order, ie: first in / first out.
     */
    private void flush ()
    {
        // Notify NAP thread that a message can be processed
        synchronized (mLock)
        {
            mFlush.set(true);
            mLock.notifyAll();
        }
    }


    /**
     * Forwards a message to the NAP APP.
     * Also ensures the NAP thread is notified about a received message.
     * @param data the NAP message as json formatted string.
     */
    private void call(String data)
    {
        // Send message and Flush:
        // triggers an update of the NAP application running in the background
        if(napSendMessage(data))
            flush();
    }


    /**
     * Stop this service and request that the activity stop too
     * This is called when an intent to stop the service is received.
     */
    private void stopService()
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
        Log.i(TAG, "ForegroundService.onDestroy");

        // Stop our worker
        mStopThread = true;

        // Flush NAP system
        flush();

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
