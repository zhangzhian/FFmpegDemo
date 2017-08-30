package com.yodosmart.ffmpegdemo;

import android.graphics.Bitmap;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EdgeEffect;
import android.widget.EditText;
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
        tv.setText("1======" + stringFromJNI());
        TextView tv1 = (TextView) findViewById(R.id.sample_text1);
//        tv1.setText("2======" + urlprotocolinfo());
        TextView tv2 = (TextView) findViewById(R.id.sample_text2);
//        tv2.setText("3======" + avformatinfo());
        final TextView tv3 = (TextView) findViewById(R.id.sample_text3);
//        tv3.setText("4======" + avcodecinfo());
        TextView tv4 = (TextView) findViewById(R.id.sample_text4);
//        tv4.setText("5======" + avfilterinfo());
        final EditText et1 = (EditText) findViewById(R.id.et1);
        final EditText et2 = (EditText) findViewById(R.id.et2);
        Button bt = (Button) findViewById(R.id.bt);
        bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String folderurl = Environment.getExternalStorageDirectory().getPath();

                String urltext_input = et1.getText().toString();
                String inputurl = folderurl + "/" + urltext_input;

                String urltext_output = et2.getText().toString();
                String outputurl = folderurl + "/" + urltext_output;

                Log.i("inputurl", inputurl);
                Log.i("outputurl", outputurl);
                tv3.setText("4======" + h264ToBitmap(inputurl, outputurl));

            }
        });

        tv1.setText("2======" + Environment.getExternalStorageDirectory().getPath());
        Log.e("zza", Environment.getExternalStorageDirectory().getPath());
        RGBToBitmap();
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

    public native int h264ToBitmap(String input_jstr, String output_jstr);
    public native int RGBToBitmap();
}
