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
    sendMessage("onCreate");
  }
  @Override
  protected void onStart() {
  	super.onStart();
    sendMessage("onStart");
  }
  @Override
  protected void onResume() {
  	super.onResume();
    sendMessage("onResume");
  }
  @Override
  protected void onPause() {
  	super.onPause();
    sendMessage("onPause");
  }
  @Override
  protected void onStop() {
  	super.onStop();
    sendMessage("onStop");
  }
  @Override
  protected void onDestroy() {
  	super.onDestroy();
    sendMessage("onDestroy");
  }
  
  private void sendMessage(String msg) {
    Toast.makeText(getApplicationContext(), msg, Toast.LENGTH_SHORT).show();
  }
}