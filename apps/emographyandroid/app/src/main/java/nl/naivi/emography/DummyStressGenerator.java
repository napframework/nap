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
                // Log start time
                long sstart = System.currentTimeMillis();

                // Clear existing messages
                mBuilder.clear();

                // Add sample
                addSample();

                // Create message intent
                Intent messageIntent = new Intent();
                messageIntent.setAction(Constants.ACTION.API_SEND_MESSAGE);

                // Add stress data to message
                messageIntent.putExtra(Constants.API.MESSAGE, mBuilder.asString());

                // Send message to nap application
                mContext.sendBroadcast(messageIntent);

                sleep(Math.max(0, mMessageInterval - (System.currentTimeMillis() - sstart)));

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
