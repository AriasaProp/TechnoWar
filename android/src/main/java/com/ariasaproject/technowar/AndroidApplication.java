package com.ariasaproject.technowar;

import android.app.NativeActivity;
import android.os.Bundle;
import android.content.Context;
import android.widget.Toast;
import android.view.View;
import android.view.Window;


public class AndroidApplication extends NativeActivity {
  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    Toast.makeText(getApplicationContext(), "onCreate", Toast.LENGTH_SHORT).show();
  }
  @Override
  protected void onStart() {
  	super.onStart();
    Toast.makeText(getApplicationContext(), "onStart", Toast.LENGTH_SHORT).show();
  }
  @Override
  protected void onResume() {
  	super.onResume();
    Toast.makeText(getApplicationContext(), "onResume", Toast.LENGTH_SHORT).show();
  }
  @Override
  protected void onPause() {
  	super.onPause();
    Toast.makeText(getApplicationContext(), "onPause", Toast.LENGTH_SHORT).show();
  }
  @Override
  protected void onStop() {
  	super.onStop();
    Toast.makeText(getApplicationContext(), "onStop", Toast.LENGTH_SHORT).show();
  }
  @Override
  protected void onDestroy() {
  	super.onDestroy();
    Toast.makeText(getApplicationContext(), "onDestroy", Toast.LENGTH_SHORT).show();
  }
  
  private void sendMessage(String msg) {
    Toast.makeText(getApplicationContext(), msg, Toast.LENGTH_SHORT).show();
  }
}