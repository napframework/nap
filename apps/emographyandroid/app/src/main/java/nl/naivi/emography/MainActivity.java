package nl.naivi.emography;

import android.app.ActivityManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import java.util.Locale;
import java.util.UUID;

public class MainActivity extends AppCompatActivity
{
    private static final String TAG = "NAPServiceDemo";

    private IntentFilter mIntentFilter;
    private APIMessageBuilder mBuilder = new APIMessageBuilder();

    /**
     * Returns to current number of samples based on slider value
     */
    private int getNumberOfSamples()
    {
        SeekBar slider = findViewById(R.id.sampleSlider);
        return Math.max(slider.getProgress(), 1);
    }


    /**
     * Updates sample count text view
     */
    private void updateSampleView()
    {
        TextView view  = findViewById(R.id.sampleDisplay);
        int sample_count = getNumberOfSamples();
        view.setText(String.format(Locale.getDefault(), "%04d", sample_count));
    }


    /**
     * Initializes the GUI
     */
    void initGUI()
    {
        // Set toolbar title
        Toolbar toolbar = findViewById(R.id.toolbar);
        toolbar.setTitle(getString(R.string.toolbar_title));
        setSupportActionBar(toolbar);

        // Get slider
        SeekBar sample_slider = findViewById(R.id.sampleSlider);
        sample_slider.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                updateSampleView();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
        sample_slider.setProgress(100);
    }


    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initGUI();

        // Add our intent actions for shutdown and logging
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(Constants.ACTION.STOP_ACTIVITY);
        mIntentFilter.addAction(Constants.ACTION.API_LOG_ACTIVITY);
        mIntentFilter.addAction(Constants.ACTION.API_RESPONSE_ACTIVITY);

        // Check if service is running
        if (!isServiceRunning(ForegroundService.class))
        {
            // Not running, start service
            Log.i(TAG, "Starting service");
            Intent startIntent = new Intent(MainActivity.this, ForegroundService.class);
            startService(startIntent);
        }
        else {
            Log.i(TAG, "Service already running");
        }
    }


    @Override
    protected void onResume()
    {
        super.onResume();
        Log.i(TAG, "MainActivity.onResume");

        // Register broadcast receiver
        registerReceiver(mReceiver, mIntentFilter);

        // Flush existing log view
        TextView tv = findViewById(R.id.text_log);
        tv.setText("");

        // Flush existing nap view
        TextView nv = findViewById(R.id.api_log);
        nv.setText("");
    }


    @Override
    protected void onPause()
    {
        super.onPause();
        Log.i(TAG, "MainActivity.onPause");

        // Unregister broadcast receiver
        unregisterReceiver(mReceiver);
    }


    @Override
    public void onDestroy()
    {
        Log.i(TAG, "MainActivity.onDestroy");
        super.onDestroy();
    }


    private final BroadcastReceiver mReceiver = new BroadcastReceiver()
    {
        @Override
        public void onReceive(Context context, Intent intent)
        {
            String action = intent.getAction();

            if (action.equals(Constants.ACTION.STOP_ACTIVITY)) {
                // Received stop request from service
                finish();
            }
            else if(action.equals(Constants.ACTION.API_RESPONSE_ACTIVITY)) {
                // Received api response message from service
                TextView tv = findViewById(R.id.api_log);
                tv.setText(intent.getStringExtra(Constants.API.MESSAGE));
            }
            else if (action.equals(Constants.ACTION.API_LOG_ACTIVITY)) {
                // Received api log message from service
                TextView tv = findViewById(R.id.text_log);
                String msg = intent.getStringExtra(Constants.API.LOG);
                tv.setText(String.format("%s\n%s", tv.getText(), msg));
            }
        }
    };


    /**
     * Check whether service is already running
     *
     * @param serviceClass Class of service to check for
     * @return Whether running
     */
    private boolean isServiceRunning(Class<?> serviceClass)
    {
        ActivityManager manager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        for (ActivityManager.RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE))
        {
            if (serviceClass.getName().equals(service.service.getClassName()))
            {
                return true;
            }
        }
        return false;
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
        MenuInflater menuInflater = getMenuInflater();
        menuInflater.inflate(R.menu.menu, menu);
        return true;
    }


    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {

        switch (item.getItemId())
        {
            case R.id.shutdown:
                // Received shutdown request from menu
                Intent shutdownIntent = new Intent();
                shutdownIntent.setAction(Constants.ACTION.STOP_SERVICE);
                sendBroadcast(shutdownIntent);
                break;
        }
        return true;
    }


    /**
     * Called when a new set of data is requested.
     * Constructs an api message based on the requested number of samples and start-end time
     * @param view calling view (button)
     */
    public void onRequestData(View view)
    {
        Intent queryIntent = new Intent();
        queryIntent.setAction(Constants.ACTION.API_SEND_MESSAGE);

        // Construct message
        mBuilder.clear();
        APIMessage msg = mBuilder.addMessage("updateView");
        msg.addLong("startTime", System.currentTimeMillis() - (1000*60*60*20));
        msg.addLong("endTime", System.currentTimeMillis()-(1000*60*60*10));
        msg.addInt("samples", getNumberOfSamples());
        msg.addString("UUID", UUID.randomUUID().toString());

        // Add message
        queryIntent.putExtra(Constants.API.MESSAGE, mBuilder.asString());

        // Send
        sendBroadcast(queryIntent);
    }


    /**
     * Called when the data model needs to be cleared
     * @param view calling view (button)
     */
    public void onClearCache(View view)
    {
        Intent queryIntent = new Intent();
        queryIntent.setAction(Constants.ACTION.API_SEND_MESSAGE);

        // Construct message
        mBuilder.clear();
        APIMessage msg = mBuilder.addMessage("clearCache");

        // Add message
        queryIntent.putExtra(Constants.API.MESSAGE, mBuilder.asString());

        // Send
        sendBroadcast(queryIntent);
    }


    /**
     * Called when the data model needs to be populated
     * @param view calling view (button)
     */
    public void onPopulateCache(View view)
    {
        Intent queryIntent = new Intent();
        queryIntent.setAction(Constants.ACTION.API_SEND_MESSAGE);

        // Construct message
        mBuilder.clear();
        APIMessage msg = mBuilder.addMessage("populateCache");

        // Add message
        queryIntent.putExtra(Constants.API.MESSAGE, mBuilder.asString());

        // Send
        sendBroadcast(queryIntent);
    }
}
