package nl.naivi.emography;

import android.os.Handler;
import android.util.Log;
import android.widget.Toast;

import java.util.concurrent.atomic.AtomicBoolean;

/*
 * Runs the NAP application in the background until stop is called
 * Messages can be send from thread A (the service) and processed here, on thread B.
 * Calling flush() forces an update of the NAP application:
 * This ensures all collected messages are processed in order.
 */
public class APIThread extends Thread
{
    private ForegroundService mService = null;
    private AtomicBoolean mStop = new AtomicBoolean(false);
    private AtomicBoolean mFlush = new AtomicBoolean(false);
    private static final String TAG = "NAPEmography";
    final private Object mMessageLock = new Object();

    /**
     * API Thread Constructor
     * @param service Foreground service associated with this thread
     */
    APIThread(ForegroundService service) {
        mService = service;
    }

    /**
     * Stops the api thread from running.
     */
    void stopRunning() {
        mStop.set(true);
        flush();
    }

    /**
     * Sends a message to the running NAP application.
     * This can be called from any thread.
     * After receiving a message flush() is called to processes the event.
     * @return if the message is accepted by the running NAP application.
     */
    boolean sendMessage(String message) {
        if(mService.napSendMessage(message))
        {
            flush();
            return true;
        }
        return false;
    }

    /**
     * Flushes the api received api messages.
     * This forces an update on the NAP application side
     */
     private void flush() {

         // Notify NAP thread that a message can be processed
         synchronized (mMessageLock)
         {
             mFlush.set(true);
             mMessageLock.notifyAll();
         }
    }

    /**
     * Initializes and runs the emography NAP application until stopped.
     */
    @Override
    public void run()
    {
        // Initialize NAP application environment
        boolean initialized = mService.napInit();

        // Check init success
        if (!initialized)
        {
            Log.e(TAG, "Abandoning launch due to NAP initialization failure");
            Handler mainHandler = new Handler(mService.getMainLooper());
            Runnable myRunnable = new Runnable()
            {
                @Override
                public void run()
                {
                    Toast.makeText(mService, "NAP initialization failure", Toast.LENGTH_SHORT).show();
                    mService.kill();
                }
            };
            mainHandler.post(myRunnable);
            return;
        }

        // Update NAP when a new message is received
        while (!Thread.interrupted() && !mStop.get())
        {
            synchronized (mMessageLock) {
                try {
                    // Wait until we need to flush
                    while (!mFlush.get()) {
                        mMessageLock.wait();
                    }
                }
                catch(InterruptedException e) {
                    Log.e(TAG, e.toString());
                }
            }

            // Reset
            mFlush.set(false);

            // Flush - forces an update on the C++ side
            mService.napUpdate();
        }

        // Shutdown NAP core
        mService.napShutdown();
        Log.i(TAG, "Service thread exiting cleanly");
    }
}
