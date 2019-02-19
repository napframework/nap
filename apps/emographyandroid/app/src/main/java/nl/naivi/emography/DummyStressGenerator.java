package nl.naivi.emography;

import android.content.Intent;
import android.util.Log;

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
    private long mMessageInterval = 2500;           ///< ms in between sample message
    private long mSampleInterval = 100;             ///< ms in between samples
    Random mRandom = new Random();

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
                long start = System.currentTimeMillis();

                // Clear existing messages
                mBuilder.clear();

                // Add new sample
                addSample();

                /*
                // Keep adding messages until message interval reached.
                long sample_time = 0;
                while(sample_time < mMessageInterval)
                {
                    // add sample
                    long start = System.currentTimeMillis();
                    addSample();

                    // Sleep a while before adding a new sample
                    sleep(Math.max(0, mSampleInterval - (System.currentTimeMillis() - start)));
                    sample_time += mSampleInterval;
                }
                */

                // Create message intent
                Intent messageIntent = new Intent();
                messageIntent.setAction(Constants.ACTION.API_SEND_MESSAGE);

                // Add stress data to message
                messageIntent.putExtra(Constants.API.MESSAGE, mBuilder.asString());

                // Send message to nap application
                mContext.sendBroadcast(messageIntent);
                sleep(Math.max(0, mMessageInterval - (System.currentTimeMillis() - start)));

            } catch (Exception e) {
                Log.e("error", e.toString());
            }
        }
    }

    /**
     * Adds a new sample to the current list of sample data
     */
    private void addSample() {
        APIMessage msg = mBuilder.addMessage("addSample");
        msg.addLong("timeStamp", System.currentTimeMillis());
        msg.addFloat("stressValue", mRandom.nextFloat());
        msg.addInt("stressState", mRandom.nextInt((2) + 1));
    }

    public void kill() {
        mStop.set(true);
    }
}
