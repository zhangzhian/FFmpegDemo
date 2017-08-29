package com.yodosmart.ffmpegdemo;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());
        TextView tv1 = (TextView) findViewById(R.id.sample_text1);
        tv1.setText(urlprotocolinfo());
        TextView tv2 = (TextView) findViewById(R.id.sample_text2);
        tv2.setText(avformatinfo());
        TextView tv3 = (TextView) findViewById(R.id.sample_text3);
        tv3.setText(avcodecinfo());
        TextView tv4 = (TextView) findViewById(R.id.sample_text4);
        tv4.setText(avfilterinfo());

    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();


    public native String urlprotocolinfo();

    public native String avformatinfo();

    public native String avcodecinfo();

    public native String avfilterinfo();

}
