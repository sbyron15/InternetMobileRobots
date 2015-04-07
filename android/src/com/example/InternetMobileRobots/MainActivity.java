package com.example.InternetMobileRobots;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URI;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;

import android.support.v7.app.ActionBarActivity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Spinner;
import android.widget.TextView;
import com.example.InternetMobileRobots.R;

public class MainActivity extends ActionBarActivity {

	public enum Command {
		moveForward, moveBackward, leftTurn, rightTurn, stop, setSlowSpeed, setMediumSpeed, setFastSpeed;
	}
	
	private static final String BOARD1 = "locationBoard1";
	private static final String BOARD2 = "locationBoard2";
	private Spinner modeSpinner;
	private Button buttonUp, buttonDown, buttonLeft, buttonRight;
	private SeekBar seekbar1;
	private TextView t1;
	private static final String RCMODE = "remoteControl";
	private String SID;
	
	HttpClient httpClient;
	String uri;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		SID = "default";
		
		
		setContentView(R.layout.activity_main);
		addListenerOnButtons();
		addSliderListener();
		addListenerOnSpinnerItemSelection();
		httpClient = new DefaultHttpClient();
		uri = "";
		new MyAsyncTask().execute(BOARD1,"getSID");
		PreferenceManager.setDefaultValues(this, R.xml.preferences, false);
		
		
	}

	
	public void addListenerOnSpinnerItemSelection(){
		modeSpinner = (Spinner) findViewById(R.id.spinner1);
		modeSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {

			@Override
			public void onItemSelected(AdapterView<?> parent, View view,
					int position, long id) {
				// TODO Auto-generated method stub
				String nextMode = parent.getItemAtPosition(position).toString();
				Log.d("Behaviour", nextMode);
				if(nextMode.equals(RCMODE)) {
					buttonUp.setEnabled(true);
					buttonDown.setEnabled(true);
					buttonLeft.setEnabled(true); 
					buttonRight.setEnabled(true);
				} else{
					buttonUp.setEnabled(false);
					buttonDown.setEnabled(false);
					buttonLeft.setEnabled(false); 
					buttonRight.setEnabled(false);
				}
				new MyAsyncTask().execute(BOARD2,nextMode);
				
				
				
			}

			@Override
			public void onNothingSelected(AdapterView<?> parent) {
				// TODO Auto-generated method stub
				
			}
		});
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		
		
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		

		
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			
			Intent intent = new Intent(this, SettingsActivity.class);
			startActivity(intent);
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
	
	public void addSliderListener() {
		seekbar1 = (SeekBar) findViewById(R.id.seekBar1);
		
		seekbar1.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {


			@Override
			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
				switch(progress) {
				
					case 0: new MyAsyncTask().execute(BOARD1,Command.setSlowSpeed.toString());
							Log.d("speed", "Setting speed to Slow");
							break;
					case 1: new MyAsyncTask().execute(BOARD1,Command.setMediumSpeed.toString());
							Log.d("speed", "Setting speed to Medium");
							break;
					case 2: new MyAsyncTask().execute(BOARD1,Command.setFastSpeed.toString());
							Log.d("speed", "Setting speed to Fast");
							break;
					default: break;
				}
			}

			@Override
			public void onStartTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}

			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}
		});
	}
	
	public void addListenerOnButtons() {
		
		buttonUp = (Button) findViewById(R.id.buttonUP);
		buttonDown = (Button) findViewById(R.id.buttonDOWN);
		buttonLeft = (Button) findViewById(R.id.buttonLEFT);
		buttonRight = (Button) findViewById(R.id.buttonRIGHT);
		t1 = (TextView) findViewById(R.id.textView1);
		
		buttonUp.setOnTouchListener(new View.OnTouchListener() {
			
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				if (event.getAction() == MotionEvent.ACTION_DOWN){
					Log.d("Up Pressed", "Telling robot to start moving forward");
					new MyAsyncTask().execute(BOARD1,Command.moveForward.toString());
				} else if(event.getAction() == MotionEvent.ACTION_UP){
					Log.d("Up Released", "Telling robot to stop moving forward");
					new MyAsyncTask().execute(BOARD1,Command.stop.toString());
				}
				return false;
			}
		});
		
		buttonDown.setOnTouchListener(new View.OnTouchListener() {
			
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				if (event.getAction() == MotionEvent.ACTION_DOWN){
					Log.d("Down Pressed", "Telling robot to start moving backwards");
					new MyAsyncTask().execute(BOARD1,Command.moveBackward.toString());
				} else if(event.getAction() == MotionEvent.ACTION_UP){
					Log.d("Down Released", "Telling robot to stop moving backwards");
					new MyAsyncTask().execute(BOARD1,Command.stop.toString());
				}
				return false;
			}
		});
		
		buttonLeft.setOnTouchListener(new View.OnTouchListener() {
			
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				if (event.getAction() == MotionEvent.ACTION_DOWN){
					Log.d("Left Pressed", "Telling robot to start turning left");
					new MyAsyncTask().execute(BOARD1,Command.leftTurn.toString());
				} else if(event.getAction() == MotionEvent.ACTION_UP){
					Log.d("Left Released", "Telling robot to stop turning left");
					new MyAsyncTask().execute(BOARD1,Command.stop.toString());
				}
				return false;
			}
		});
		
		buttonRight.setOnTouchListener(new View.OnTouchListener() {
			
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				if (event.getAction() == MotionEvent.ACTION_DOWN){
					Log.d("Right Pressed", "Telling robot to start turning right");
					new MyAsyncTask().execute(BOARD1,Command.rightTurn.toString());
				} else if(event.getAction() == MotionEvent.ACTION_UP){
					Log.d("Right Released", "Telling robot to stop turning right");
					new MyAsyncTask().execute(BOARD1,Command.stop.toString());
				}
				return false;
			}
		});
		
			
		
	}
	
	private void sendRequest(String board, String action){
		SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(this);
		String location = sharedPref.getString(board, "undefined");
		if(SID.equals("default")){
			uri = "http://" + location + "/arduino/" + action;
			Log.d("http request", uri);
		}else {
			uri = "http://" + location + "/arduino/" + action + "/" + SID;
		}
		try {
			Log.d("http request", uri);
			HttpResponse response = httpClient.execute(new HttpGet(uri));
			Log.d("http response:", response.toString());
			
			if(response.getStatusLine().getStatusCode() == 200){
				HttpEntity entity = response.getEntity();
				StringBuilder sb = new StringBuilder();
				try{
					BufferedReader reader = new BufferedReader(new InputStreamReader(entity.getContent()), 65728);
					String line = null;
					while ((line = reader.readLine()) != null) {
						sb.append(line);
					}
				}
				catch (IOException e) { e.printStackTrace();}
				catch (Exception e) { e.printStackTrace(); }
				final String a = sb.toString();
				if(action.equals("getSID")){
					SID = a.toString();
				}
				runOnUiThread(new Runnable() {
				     @Override
				     public void run() {

				    	 t1.setText(a.toString());
				    	 
				    }
				});
			}
		} catch (ClientProtocolException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	private class MyAsyncTask extends AsyncTask<String,Integer,Double> {

		@Override
		protected Double doInBackground(String... params) {
			sendRequest(params[0], params[1]);
			return null;
		}

	}
}

