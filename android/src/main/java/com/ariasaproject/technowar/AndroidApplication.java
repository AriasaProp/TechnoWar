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
    getWindow().getDecorView().setBackgroundColor(0xff00ff00);
  }
}