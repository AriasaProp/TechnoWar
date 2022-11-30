package com.ariasaproject.technowar;

import android.app.NativeActivity;
import android.os.Bundle;
import android.content.Context;
import android.widget.Toast;


public class AndroidApplication extends NativeActivity {
  @Override
  protected void onCreate(Bundle savedInstanceState) {
    Toast.makeText(getApplicationContext(), "onCreate", Toast.LENGTH_SHORT).show();
    super.onCreate(savedInstanceState);
  }
}