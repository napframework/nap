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

public class ForegroundService extends Service
{
    private static final String TAG = "NAPEmography";
    private IntentFilter mIntentFilter;
    private boolean mStopThread = false;
    private String mOutputLog = "";
    private APIMessageBuilder mBuilder = new APIMessageBuilder();


    @Override
    public void onCreate()
    {
        super.onCreate();
        Log.i(TAG, "Service onCreate");

        // Accept shutdown and logging request intents
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(Constants.ACTION.STOP_SERVICE);
        mIntentFilter.addAction(Constants.ACTION.REQUEST_LOG_FROM_SERVICE);
    }


    @Override
    public int onStartCommand(Intent intent, int flags, int startId)
    {
        // Start broadcast receiver
        registerReceiver(mReceiver, mIntentFilter);

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
        Thread t = new Thread(new Runnable()
        {
            @Override
            public void run()
            {
                // Initialisation
                boolean initialized = napInit();

                // Check init success
                if (!initialized)
                {
                    Log.e(TAG, "Abandoning launch due to NAP initialisation failure");
                    Handler mainHandler = new Handler(ForegroundService.this.getMainLooper());
                    Runnable myRunnable = new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            Toast.makeText(ForegroundService.this, "NAP initialisation failure", Toast.LENGTH_SHORT).show();
                            stopAll();
                        }
                    };
                    mainHandler.post(myRunnable);

                    return;
                }

                // Loop through doing some imaginary things with our NAP service
                while (!Thread.interrupted() && !mStopThread)
                    try
                    {
                        // Create NAP Message
                        mBuilder.clear();
                        APIMessage msg = mBuilder.addMessage("updateView");
                        msg.addLong("startTime", System.currentTimeMillis() - (1000*2));
                        msg.addLong("endTime", System.currentTimeMillis());
                        msg.addInt("samples", 200);

                        // Call
                        call(mBuilder.asString());

                        // Flush
                        napFlush();

                        Thread.sleep(5000);
                    }
                    catch (InterruptedException e) { }

                // Shutdown NAP core
                shutdownNap();
                Log.i(TAG, "Service thread exiting cleanly");
            }
        });

        // Start our thread
        t.start();

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
                stopAll();
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


    private void call(String data)
    {
        napSendMessage(data);
    }


    private void napFlush()
    {
        napUpdate();
    }


    private void shutdownNap()
    {
        napShutdown();
    }


    /**
     * Stop the service and request that the activity stop too
     */
    private void stopAll()
    {
        stopForeground(true);
        stopSelf();

        Intent appStopIntent = new Intent();
        appStopIntent.setAction(Constants.ACTION.STOP_ACTIVITY);
        sendBroadcast(appStopIntent);
    }


    @Override
    public void onDestroy()
    {
        Log.i(TAG, "ForegroundService.onDestroy");

        // Stop our worker
        mStopThread = true;

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
     * Currently unused method for pushing the log into Java from the NAP service, instead of
     * pulling as is currently being used.
     */
    public void onAPILog(final String s)
    {
        Handler mainHandler = new Handler(this.getMainLooper());
        Runnable myRunnable = new Runnable()
        {
            @Override
            public void run() {
            Intent logIntent = new Intent();
            logIntent.setAction(Constants.ACTION.API_LOG_ACTIVITY);
            logIntent.putExtra("apilog", s);
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
                apiIntent.setAction(Constants.ACTION.API_MESSAGE_ACTIVITY);
                apiIntent.putExtra("apimessage", apiMessage);
                sendBroadcast(apiIntent);
            }
        };
        mainHandler.post(myRunnable);
    }


    // Load native library
    static {
        System.loadLibrary("nap-service");
    }

    public native boolean   napInit();
    public native void      napUpdate();
    public native boolean   napSendMessage(String data);
    public native void      napShutdown();
}
