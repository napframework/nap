package nl.naivi.emography;

import android.content.Intent;
import android.util.Log;

import java.util.Locale;
import java.util.Random;
import java.util.concurrent.atomic.AtomicBoolean;

import static java.lang.Long.max;

/**
 * Class that generates dummy stress data and sends it to the running nap environment
 */
public class DummyStressGenerator extends Thread
{
    private MainActivity mContext = null;
    private AtomicBoolean mStop = new AtomicBoolean(false);
    private APIMessageBuilder mBuilder = new APIMessageBuilder();
    private long mMessageInterval = 10000;              ///< ms in between sample message
    private long mSampleInterval = 500;                 ///< ms in between samples
    private Random mRandom = new Random();

    /**
     * Constructor
     * @param context Associated context.
     */
    DummyStressGenerator(MainActivity context) {
        mContext = context;
    }

    /**
     * Sends mock stress data to the NAP application until stopped
     */
    @Override
    public void run()
    {
        while(!Thread.interrupted() && !mStop.get()) {
            try {
                // Clear existing messages
                mBuilder.clear();

                // Keep adding messages until message interval reached.
                long sample_time = 0;
                int sample_idx = 0;
                while(sample_time < mMessageInterval)
                {
                    // add sample
                    long start = System.currentTimeMillis();
                    addSample(sample_idx);
                    sample_idx++;

                    // Sleep a while before adding a new sample
                    sleep(Math.max(0, mSampleInterval - (System.currentTimeMillis() - start)));
                    sample_time += mSampleInterval;
                }

                // Create message intent
                Intent messageIntent = new Intent();
                messageIntent.setAction(Constants.ACTION.API_SEND_MESSAGE);

                // Add stress data to message
                messageIntent.putExtra(Constants.API.MESSAGE, mBuilder.asString());

                // Send message to nap application
                mContext.sendBroadcast(messageIntent);

            } catch (Exception e) {
                Log.e("error", e.toString());
            }
        }
    }

    /**
     * Adds a new sample to the current list of sample data
     * The index is used to add a unique value to every parameter
     * This is necessary for serialization:
     * Every parameter must have a unique id in the final json string that is read by NAP.
     * @param index the index of the sample
     */
    private void addSample(int index) {
        APIMessage msg = mBuilder.addMessage("addSample");
        msg.addLong(String.format(Locale.getDefault(), "timestamp_%d", index),
                System.currentTimeMillis());
        msg.addFloat(String.format(Locale.getDefault(),"stressValue_%d", index),
                mRandom.nextFloat());
        msg.addInt(String.format(Locale.getDefault(), "stressState_%d", index),
                mRandom.nextInt((2) + 1));
    }

    void kill() {
        mStop.set(true);
    }
}
