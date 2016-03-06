package com.p.battery;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.Context;
import android.os.IBinder;
import android.os.BatteryManager;
import android.util.Log;
import android.widget.Toast;
 
public class BatteryService extends Service {
    public BatteryService() {
    }
	private static final String TAG=BatteryActivity.class.getName();
	protected Intent intent;
 
    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        throw new UnsupportedOperationException("Not yet implemented");
    }
 
    @Override
    public void onCreate() {
	Log.v(TAG, " onCreate bservice");
        //Toast.makeText(this, "p_Battery Service was Created", Toast.LENGTH_LONG).show();
	BroadcastReceiver batteryReceiver = new BroadcastReceiver() {
			@Override
            public void onReceive(Context context, Intent intent) {
				BatteryService.this.intent=intent;
				refreshPower();
            }
        };
        IntentFilter filter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        registerReceiver(batteryReceiver, filter);
 
    }
    
    /* A native method that is implemented by the
     * 'battery-jni' native library, which is packaged
     * with this application.
     */
    public native String  pBatterySaverFromJNI();
    public native int batteryless30JNI ();

    /* this is used to load the 'batd-jni' library on application
     * startup. The library has already been unpacked into
     * /data/data/com.p.battery/lib/libbatd-jni.so at
     * installation time by the package manager.
     */
    static {
        System.loadLibrary("batd-jni");
    }
 
    @Override
    public void onStart(Intent intent, int startId) {
    	refreshPower();
    	CharSequence text = pBatterySaverFromJNI();
    	Log.v(TAG, " StartServiced text = " + text);
    	//Log.v(TAG, " StartServiced  = " + pBatterySaverFromJNI());
    	// For time consuming an long tasks you can launch a new thread here...
    	//Toast.makeText(this, pBatterySaverFromJNI(), Toast.LENGTH_LONG).show();    	
    	//batteryless30JNI (); // Temporarily using later needs to remove
    	Log.v(TAG, " batteryless30JNI = " + batteryless30JNI());
    }
 
   private void refreshPower() {
	if (intent!=null)
		refreshPower(intent);
   }

   private void refreshPower(Intent intent) {
	int level = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
	int scale = intent.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
	int temp = intent.getIntExtra(BatteryManager.EXTRA_TEMPERATURE, -1);
	// transform level into percentage
	level=level*100/scale;
	
//	batteryPreference.setTitle(batteryCategoryTitle+" (Actual: "+level+"%)");
//	temperaturePreference.setTitle(temperatureCategoryTitle+" (Actual: "+temp+")");
	Log.v(TAG, "Inside BatteryService level = " + level);
	if (level <= 50) {
		Log.v(TAG, "Inside BatteryService level = " + level);
	} 
	if (level <= 30) {
		Log.v(TAG, "Inside BatteryService + level = " + level);
		//CharSequence text = batteryless30JNI ();
		//Log.v(TAG, " batteryless30JNI = " + text);
//		saveAndClearAll();							
	}
	if (level <= 20) {						
		Log.v(TAG, "Inside BatteryService + level = " + level);
	}
	//	CharSequence text = batteryless30JNI (); // Temporarily using later needs to remove
	Log.v(TAG, " batteryless30JNI = " + batteryless30JNI ());
//	setLowPower(level < Integer.parseInt(sharedPrefs.getString(PreferenceConstant.BATTERY_KEY, PreferenceConstant.BATTERY_DEFAULT)));
  }
    @Override
    public void onDestroy() {
        Toast.makeText(this, "Battery Service Destroyed", Toast.LENGTH_LONG).show();
 
    }
}

