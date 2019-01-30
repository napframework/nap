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

public class MainActivity extends AppCompatActivity
{
    private static final String TAG = "NAPServiceDemo";

    private IntentFilter mIntentFilter;

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
        mIntentFilter.addAction(Constants.ACTION.LOG_TO_ACTIVITY);

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

        // Request the existing log from the service
        // TODO If used outside of demo would need to cater for a non-backlog message coming through
        //      before backlog is received
        Intent logRequestIntent = new Intent();
        logRequestIntent.setAction(Constants.ACTION.REQUEST_LOG_FROM_SERVICE);
        sendBroadcast(logRequestIntent);
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

            if (action.equals(Constants.ACTION.STOP_ACTIVITY))
            {
                // Received stop request from service
                finish();
            } else if (action.equals(Constants.ACTION.LOG_TO_ACTIVITY))
            {
                // Received log from service
                appendToUILog(intent.getStringExtra("log"));
            }
        }
    };


    private void appendToUILog(String s)
    {
        TextView tv = findViewById(R.id.text_log);
        String built = tv.getText().toString();
        if (!built.equals(""))
            built += "\n";

        tv.setText(built + s);
    }


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
     * Called when a new set of data is requested
     * @param view calling view (button)
     */
    public void onRequestData(View view)
    {

    }


    /**
     * Called when the sliders changes
     */

}
