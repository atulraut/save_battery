package com.p.battery;

import java.io.IOException;

import android.annotation.SuppressLint;
import android.app.WallpaperManager;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
//import android.location.LocationManager;
import android.net.wifi.WifiManager;
import android.os.BatteryManager;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.preference.Preference.OnPreferenceClickListener;
import android.provider.Settings;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Gravity;
import android.view.ViewGroup;
import android.webkit.MimeTypeMap;
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.ImageView;
import android.widget.TextView;

@SuppressLint("InlinedApi") public class BatteryActivity extends PreferenceActivity implements OnSharedPreferenceChangeListener
{
	private static final String TAG=BatteryActivity.class.getName();
	private Bitmap bitmap;
	private int brightness=-1;
	private int brightnessMode;
	private Boolean isAutoSync;
	private WallpaperManager wallpaperManager;
	private Preference batteryPreference;
	private CharSequence batteryCategoryTitle;
	private Preference temperaturePreference;
	Preference pref_toast;
	private CharSequence temperatureCategoryTitle;
//	private LocationManager locationmanager;
	private Boolean isLocationEnabled;
	private Boolean isNetworkEnabled;
	BluetoothAdapter mBluetoothAdapter;
	WifiManager mWifiManager;
	private Boolean isBluetoothEnabled;
	private Boolean isWifiEnabled;
	private SharedPreferences sharedPrefs;
	private Boolean lastLowPower;
	protected Intent intent;

	/** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.preferences);

        initComponent();

        saveAll();

        BroadcastReceiver batteryReceiver = new BroadcastReceiver() {
			@Override
            public void onReceive(Context context, Intent intent) {
				BatteryActivity.this.intent=intent;
				refreshPower();
            }
        };
        IntentFilter filter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        registerReceiver(batteryReceiver, filter);
    }

	protected void initComponent()
	{
		wallpaperManager=WallpaperManager.getInstance(getApplicationContext());
//        locationmanager=((LocationManager)getApplicationContext().getSystemService(Context.LOCATION_SERVICE));  
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter(); 
        mWifiManager = (WifiManager) this.getSystemService(Context.WIFI_SERVICE); 
        		
        sharedPrefs=PreferenceManager.getDefaultSharedPreferences(this);
        sharedPrefs.registerOnSharedPreferenceChangeListener(this);

        batteryPreference=findPreference("battery_category");
        batteryCategoryTitle=batteryPreference.getTitle();
	Preference pref_toast = findPreference("toast_key");
   	pref_toast.setOnPreferenceClickListener(new OnPreferenceClickListener() {      	
		@Override
      		public boolean onPreferenceClick(Preference preference) {
        	 // TODO Auto-generated method stub
		  showAbout ();
        	 return false;
      		}
	});	

        //temperaturePreference=findPreference("temperature_category");
        //temperatureCategoryTitle=temperaturePreference.getTitle();
	}

    private void saveAll()
	{
		isAutoSync=ContentResolver.getMasterSyncAutomatically();
		try {
		/*    isLocationEnabled=locationmanager.isProviderEnabled(LocationManager.GPS_PROVIDER);
		    // remove it from the location manager		    
		    locationmanager.removeTestProvider(LocationManager.GPS_PROVIDER);
		    locationmanager.clearTestProviderEnabled(LocationManager.GPS_PROVIDER);
	            locationmanager.clearTestProviderLocation(LocationManager.GPS_PROVIDER);
		    locationmanager.clearTestProviderStatus(LocationManager.GPS_PROVIDER);
		 */   //locationmanager.removeProximityAlert(LocationManager.GPS_PROVIDER);
		}
		catch (Exception e) {}
	//	isNetworkEnabled =locationmanager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
		isBluetoothEnabled = mBluetoothAdapter.isEnabled();
		//isWifiEnabled = mWifiManager.isWifiEnabled();
		
		brightnessMode=Settings.System.getInt(getContentResolver(), android.provider.Settings.System.SCREEN_BRIGHTNESS_MODE, android.provider.Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL);
		brightness=Settings.System.getInt(getContentResolver(), android.provider.Settings.System.SCREEN_BRIGHTNESS, 255);
		Drawable drawable=wallpaperManager.getDrawable();
		if(drawable instanceof BitmapDrawable)
			bitmap=((BitmapDrawable)drawable).getBitmap();
	}

	@Override
    public void onBackPressed()
    {
    	// Override back button to act like home button
    	moveTaskToBack(true);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
    	//getMenuInflater().inflate(R.menu.menu, menu);
    	return true;
    }

    @Override
    protected void onDestroy()
    {
    	super.onDestroy();
    	stopNewService ();
    	sharedPrefs.unregisterOnSharedPreferenceChangeListener(this);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
    	if(item.getItemId()==R.id.quitMenuItem)
    	{
    		restoreAll();
    		finish();
    	}
    	else if(item.getItemId()==R.id.aboutMenuItem)
    	{
    		try
			{
				AboutDialogBuilder.create(this, getString(R.string.about)).show();
			}
			catch (NameNotFoundException e)
			{
				Log.e(TAG, "Failed to display about dialog", e);
			}
    	}
    	return true;
    }

    private void saveAndClearAll()
    {
    	//saveAndClearScreenBrightness();
    	saveAndClearAutoSync();
    //	saveAndClearLocation();
    	saveAndClearWallpaper();
    	saveAndClearBluetooth();
    	saveAndClearWifi();
    }

	private void saveAndClearAutoSync()
	{
		setAutoSync(false);
	}

	private void saveAndClearLocation()
	{
		setLocation(false);
	}

	private void saveAndClearScreenBrightness_20()
	{ // SCREEN_BRIGHTNESS range 0-255
		setScreenBrightness(android.provider.Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL, 25);
	}

	private void saveAndClearScreenBrightness_30()
	{ // SCREEN_BRIGHTNESS range 0-255
		setScreenBrightness(android.provider.Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL, 70);
	}
	
	private void saveAndClearScreenBrightnes_50()
	{ // SCREEN_BRIGHTNESS range 0-255
		setScreenBrightness(android.provider.Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL, 150);
	}
	
	private void saveAndClearWallpaper()
	{
		try
		{
			wallpaperManager.setBitmap(BitmapFactory.decodeResource(getResources(), R.drawable.black));
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
	}
	
	private void saveAndClearBluetooth () {
		if (mBluetoothAdapter.isEnabled()) {
		    mBluetoothAdapter.disable(); 
		}		
	}
	
	private void saveAndClearWifi () {
		//if(mWifiManager.isWifiEnabled())
		//	mWifiManager.setWifiEnabled(false);		
	}
	
	private void restoreAll()
	{
		//restoreScreenBrightness();
		restoreAutoSync();
	//	restoreLocation();
		restoreWallpaper();
		restoreBluetooth();
		restoreWifi();
	}
    
	private void restoreLocation()
	{
		if(isLocationEnabled!=null)
			setLocation(isLocationEnabled);
	}

	private void setLocation(boolean locationEnabled)
	{
		// TODO
		Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
                startActivityForResult(intent, locationEnabled ? 1 : 0);
	}

	private void restoreAutoSync()
	{
		if(isAutoSync!=null)
			setAutoSync(isAutoSync);
	}

	private void restoreScreenBrightness()
	{
		if(brightness!=-1)
			setScreenBrightness(brightnessMode, brightness);
	}

	private void restoreWallpaper()
	{
		if(bitmap!=null)
		{
			try
			{
				wallpaperManager.setBitmap(bitmap);
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}
		}
	}

	private void setAutoSync(boolean value)
	{
		ContentResolver.setMasterSyncAutomatically(value);
	}

	private void setScreenBrightness(int mode, int brightness)
	{
		Settings.System.putInt(getContentResolver(), android.provider.Settings.System.SCREEN_BRIGHTNESS_MODE, mode);
		Settings.System.putInt(getContentResolver(), android.provider.Settings.System.SCREEN_BRIGHTNESS, brightness);
	}
	
	private void restoreBluetooth () {
		isBluetoothEnabled = mBluetoothAdapter.isEnabled();
	    if (!isBluetoothEnabled) {
	    	mBluetoothAdapter.enable(); 
	    }		
	}

	private void restoreWifi () {
//		isWifiEnabled = mWifiManager.isWifiEnabled();
//		if (!isWifiEnabled)
//			mWifiManager.setWifiEnabled(false);
	}
	
	@Override
	public void onSharedPreferenceChanged(SharedPreferences sharedpreferences, String s)
	{
		if(s.equals(PreferenceConstant.BATTERY_KEY))
			refreshPower();
		if(s.equals(PreferenceConstant.SHOW_HELP_KEY))
			showAbout();
	}

	private void refreshPower()
	{
		if (intent!=null)
			refreshPower(intent);
	}
	private void showAbout()
	{
		LayoutInflater inflater = getLayoutInflater();
                View layout = inflater.inflate(R.layout.toast,
			                       (ViewGroup) findViewById(R.id.toast_layout_root));
		TextView text = (TextView) layout.findViewById(R.id.text);
		text.setText(" Contact - pratikpatil1615@gmail.com !");
		Toast toast = new Toast(getApplicationContext());
		toast.setGravity(Gravity.CENTER_VERTICAL, 0, 0);
		toast.setDuration(Toast.LENGTH_LONG);
		toast.setView(layout);
		toast.show();
	}

	private void refreshPower(Intent intent)
	{
		int level = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
		int scale = intent.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
		int temp = intent.getIntExtra(BatteryManager.EXTRA_TEMPERATURE, -1);
		// transform level into percentage
		level=level*100/scale;
		batteryPreference.setTitle(batteryCategoryTitle+" "+level+"%");
//		temperaturePreference.setTitle(temperatureCategoryTitle+" (Actual: "+temp+")");
		if (level <= 50) {
			saveAndClearScreenBrightnes_50();
		} 
		if (level <= 30) {					
			saveAndClearScreenBrightness_30 ();
		}
		if (level <= 20) {
			saveAndClearScreenBrightness_20();
			saveAndClearAll();
			
		}
		saveAndClearAll();
		setLowPower(level < Integer.parseInt(sharedPrefs.getString(PreferenceConstant.BATTERY_KEY, PreferenceConstant.BATTERY_DEFAULT)));
		startNewService (); // Starting BatteryService
	}

	private void setLowPower(boolean lowpower)
	{
		if(lastLowPower==null || lastLowPower!=lowpower)
		{
			Toast toast = Toast.makeText(getApplicationContext(), lowpower ? "Set low power mode" : "Set full power mode", Toast.LENGTH_LONG);
			toast.show();
			if(lowpower)
				saveAndClearAll();
			else {
			//	restoreAll();
			}
			lastLowPower=lowpower;
		}
	}
	
	// Start the  service
    public void startNewService() {
 	Log.v(TAG, "started BatteryServcie");
        startService(new Intent(this, BatteryService.class));
    }
 
    // Stop the  service
    public void stopNewService() {
 
        stopService(new Intent(this, BatteryService.class));
    }
}
